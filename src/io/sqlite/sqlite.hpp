#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <sqlite3.h>

#include "model/types.hpp"
#include "model/utility.hpp"

namespace tsv {

/// Quotes a SQLite identifier (table or column name) for safe use in SQL statements.
/// @param value  The identifier to quote.
/// @return The quoted identifier (e.g., `"my column"`).
inline std::string sqlite_quote_identifier(std::string_view value) {
  std::string quoted = "\"";
  for (const char ch : value) {
    if (ch == '"') { quoted += "\"\""; }
    else { quoted.push_back(ch); }
  }
  quoted.push_back('"');
  return quoted;
}

/// Fetches the list of user table names from a SQLite database.
/// @param db  Open SQLite database handle.
/// @return Vector of table names (excluding sqlite_% internal tables).
/// @throws std::runtime_error if the query fails.
inline std::vector<std::string> fetch_sqlite_tables(sqlite3* db) {
  std::vector<std::string> tables;
  const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name;";
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to list SQLite tables: " + std::string(sqlite3_errmsg(db)));
  }
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* text = sqlite3_column_text(stmt, 0);
    if (text != nullptr) tables.emplace_back(reinterpret_cast<const char*>(text));
  }
  sqlite3_finalize(stmt);
  return tables;
}

/// Fetches column metadata for a given table from a SQLite database.
/// @param db         Open SQLite database handle.
/// @param table_name  Name of the table to inspect.
/// @return Vector of ColumnInfo describing each column.
/// @throws std::runtime_error if the PRAGMA query fails.
inline std::vector<ColumnInfo> fetch_sqlite_columns(sqlite3* db, const std::string& table_name) {
  std::vector<ColumnInfo> columns;
  const auto pragma = "PRAGMA table_info(" + sqlite_quote_identifier(table_name) + ");";
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, pragma.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to inspect SQLite table: " + std::string(sqlite3_errmsg(db)));
  }
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* text = sqlite3_column_text(stmt, 1);
    const unsigned char* decl = sqlite3_column_text(stmt, 2);
    const std::string name = text == nullptr ? std::string{} : reinterpret_cast<const char*>(text);
    const std::string decl_type = decl == nullptr ? std::string{} : reinterpret_cast<const char*>(decl);
    ColumnKind kind = ColumnKind::Text;
    std::string lower = decl_type;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (lower.find("int") != std::string::npos || lower.find("real") != std::string::npos || lower.find("num") != std::string::npos || lower.find("double") != std::string::npos) {
      kind = ColumnKind::Numeric;
    } else if (lower.find("date") != std::string::npos || lower.find("time") != std::string::npos) {
      kind = ColumnKind::DateTime;
    }
    columns.push_back(ColumnInfo{name, kind, is_time_candidate(kind)});
  }
  sqlite3_finalize(stmt);
  return columns;
}

// Forward declaration needed by load_sqlite_catalog
inline std::vector<std::vector<std::string>> read_sqlite_rows(sqlite3* db, const std::string& table_name, const std::vector<ColumnInfo>& columns, std::optional<std::size_t> limit = std::nullopt);

/// Loads the full catalog (tables and columns) from a SQLite database.
/// @param path  Path to the SQLite database file.
/// @return SourceCatalog with all discovered tables and columns.
/// @throws std::runtime_error if the database cannot be opened.
inline SourceCatalog load_sqlite_catalog(const std::filesystem::path& path) {
  sqlite3* db = nullptr;
  if (sqlite3_open_v2(path.string().c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    const std::string message = db != nullptr ? sqlite3_errmsg(db) : "unknown SQLite error";
    if (db != nullptr) sqlite3_close(db);
    throw std::runtime_error("Failed to open SQLite file: " + message);
  }

  SourceCatalog catalog;
  catalog.kind = SourceKind::Sqlite;
  catalog.path = path;
  catalog.source_name = path.stem().string();

  const auto tables = fetch_sqlite_tables(db);
  constexpr std::size_t kSampleLimit = 1000;
  for (const auto& table_name : tables) {
    TableCatalog table;
    table.name = table_name;
    table.columns = fetch_sqlite_columns(db, table_name);
    if (table.columns.empty()) continue;
    const auto rows = read_sqlite_rows(db, table_name, table.columns, kSampleLimit);
    for (std::size_t col = 0; col < table.columns.size(); ++col) {
      std::vector<std::string> samples;
      samples.reserve(rows.size());
      for (const auto& row : rows) {
        if (col < row.size()) samples.push_back(row[col]);
      }
      const auto inferred = infer_column_kind(samples);
      if (inferred == ColumnKind::Numeric || inferred == ColumnKind::DateTime) {
        table.columns[col].kind = inferred;
        table.columns[col].time_candidate = true;
      }
    }
    catalog.tables.push_back(std::move(table));
  }

  sqlite3_close(db);
  return catalog;
}

/// Loads a series from a SQLite table using pre-discovered column metadata.
/// @param path       Path to the SQLite database file.
/// @param table_name Name of the table to query.
/// @param columns    Column metadata (from fetch_sqlite_columns).
/// @param request    Series request specifying time/value columns and optional max points.
/// @return LoadOutcome with the loaded series on success, or an error description on failure.
inline LoadOutcome load_sqlite_series_targeted(const std::filesystem::path& path, const std::string& table_name, const std::vector<ColumnInfo>& columns, const SeriesRequest& request) {
  sqlite3* db = nullptr;
  if (sqlite3_open_v2(path.string().c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    const std::string message = db != nullptr ? sqlite3_errmsg(db) : "unknown SQLite error";
    if (db != nullptr) sqlite3_close(db);
    return LoadOutcome{false, {}, "Failed to open SQLite file: " + message};
  }

  struct DbCloser { sqlite3* db{nullptr}; ~DbCloser() { if (db != nullptr) sqlite3_close(db); } } closer{db};

  std::optional<std::size_t> time_index;
  if (!request.time_column.empty()) {
    time_index = column_index_by_name(columns, request.time_column);
  } else if (const auto inferred = infer_time_column(columns); inferred.has_value()) {
    time_index = column_index_by_name(columns, inferred.value());
  }
  const auto value_index = column_index_by_name(columns, request.value_column);

  if (!time_index.has_value()) return LoadOutcome{false, {}, "Time column not found: " + request.time_column};
  if (!value_index.has_value()) return LoadOutcome{false, {}, "Value column not found: " + request.value_column};

  std::string sql = "SELECT ";
  sql += sqlite_quote_identifier(columns[*time_index].name);
  sql += ", ";
  sql += sqlite_quote_identifier(columns[*value_index].name);
  sql += " FROM ";
  sql += sqlite_quote_identifier(table_name);
  sql += ";";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return LoadOutcome{false, {}, "Failed to query SQLite: " + std::string(sqlite3_errmsg(db))};
  }

  SeriesData series;
  series.source_name = path.stem().string();
  series.table_name = table_name;
  series.time_column = columns[*time_index].name;
  series.value_column = columns[*value_index].name;
  series.name = series.source_name + "." + table_name + "." + series.value_column;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* time_text = sqlite3_column_text(stmt, 0);
    const unsigned char* value_text = sqlite3_column_text(stmt, 1);
    if (time_text == nullptr || value_text == nullptr) continue;
    const auto time_value = parse_scalar(reinterpret_cast<const char*>(time_text));
    double numeric_value = 0.0;
    if (!time_value.has_value() || !parse_double(reinterpret_cast<const char*>(value_text), numeric_value)) continue;
    series.time.push_back(*time_value);
    series.value.push_back(numeric_value);
  }

  sqlite3_finalize(stmt);

  if (request.max_points.has_value()) downsample_series(series, *request.max_points);

  return LoadOutcome{true, std::move(series), {}};
}

/// Reads raw rows from a SQLite table, optionally limited.
/// @param db         Open SQLite database handle.
/// @param table_name Name of the table to read.
/// @param columns    Column metadata (determines which columns are selected).
/// @param limit      Optional maximum number of rows to read.
/// @return Vector of rows, each row being a vector of string values.
/// @throws std::runtime_error if the query fails.
inline std::vector<std::vector<std::string>> read_sqlite_rows(sqlite3* db, const std::string& table_name, const std::vector<ColumnInfo>& columns, std::optional<std::size_t> limit) {
  std::vector<std::vector<std::string>> rows;
  if (columns.empty()) return rows;

  std::string sql = "SELECT ";
  for (std::size_t i = 0; i < columns.size(); ++i) {
    if (i > 0) sql += ", ";
    sql += sqlite_quote_identifier(columns[i].name);
  }
  sql += " FROM ";
  sql += sqlite_quote_identifier(table_name);
  sql += ";";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to read SQLite rows: " + std::string(sqlite3_errmsg(db)));
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    std::vector<std::string> row;
    row.reserve(columns.size());
    for (std::size_t col = 0; col < columns.size(); ++col) {
      const unsigned char* text = sqlite3_column_text(stmt, static_cast<int>(col));
      row.emplace_back(text == nullptr ? std::string{} : reinterpret_cast<const char*>(text));
    }
    rows.push_back(std::move(row));
    if (limit.has_value() && rows.size() >= *limit) break;
  }

  sqlite3_finalize(stmt);
  return rows;
}

/// Convenience function: loads a series from a SQLite table in one call.
/// Internally opens the database, fetches columns, then calls load_sqlite_series_targeted.
/// @param path       Path to the SQLite database file.
/// @param table_name Name of the table to query.
/// @param request    Series request specifying time/value columns and optional max points.
/// @return LoadOutcome with the loaded series on success, or an error description on failure.
inline LoadOutcome load_sqlite_series(const std::filesystem::path& path, const std::string& table_name, const SeriesRequest& request) {
  sqlite3* db = nullptr;
  if (sqlite3_open_v2(path.string().c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    const std::string message = db != nullptr ? sqlite3_errmsg(db) : "unknown SQLite error";
    if (db != nullptr) sqlite3_close(db);
    return LoadOutcome{false, {}, "Failed to open SQLite file: " + message};
  }

  const auto columns = fetch_sqlite_columns(db, table_name);
  sqlite3_close(db);

  return load_sqlite_series_targeted(path, table_name, columns, request);
}

} // namespace tsv