#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <lua.hpp>
#include <nlohmann/json.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>

namespace tsv {

enum class SourceKind {
  Csv,
  Sqlite
};

enum class ColumnKind {
  Numeric,
  DateTime,
  Text,
  Empty
};

struct ColumnInfo {
  std::string name;
  ColumnKind kind{ColumnKind::Text};
  bool time_candidate{false};
};

struct TableCatalog {
  std::string name;
  std::vector<ColumnInfo> columns;
};

struct SourceCatalog {
  SourceKind kind{SourceKind::Csv};
  std::string source_name;
  std::filesystem::path path;
  std::vector<TableCatalog> tables;
};

struct SeriesData {
  std::string name;
  std::vector<double> time;
  std::vector<double> value;
  std::string source_name;
  std::string table_name;
  std::string time_column;
  std::string value_column;
};

struct SeriesRequest {
  std::optional<std::string> table_name;
  std::string time_column;
  std::string value_column;
  std::optional<std::size_t> max_points;
};

struct LoadOutcome {
  bool ok{false};
  SeriesData series;
  std::string error;
};

struct TreeNode {
  std::string label;
  std::string full_name;
  bool leaf{false};
  std::vector<TreeNode> children;
};

struct ProjectSource {
  SourceKind kind{SourceKind::Csv};
  std::string path;
  std::string alias;
  std::optional<std::string> table_name;
  std::optional<std::string> time_column;
  std::vector<std::string> selected_variables;
};

struct PlotSeriesConfig {
  std::string name;
  std::string expression;
  std::optional<std::string> source_alias;
  std::optional<std::string> source_path;
  std::optional<std::string> table_name;
  std::optional<std::string> time_column;
  std::optional<std::string> value_column;
  bool visible{true};
  bool derived{false};
  std::array<double, 4> color{1.0, 0.0, 0.0, 1.0};
};

struct PlotTabConfig {
  std::string title;
  std::vector<PlotSeriesConfig> series;
  bool autoscale_x{true};
  bool autoscale_y{true};
  std::optional<std::array<double, 2>> x_range;
  std::optional<std::array<double, 2>> y_range;
  std::size_t active_series_index{0};
  std::string expression_draft;
};

using PlotViewConfig = PlotTabConfig;

struct AnalysisWindowConfig {
  std::string title;
  std::vector<PlotTabConfig> tabs;
  std::size_t active_tab{0};
};

struct WorkspaceConfig {
  std::vector<AnalysisWindowConfig> windows;
  std::size_t point_budget{50000};
};

struct ProjectState {
  std::vector<ProjectSource> sources;
  WorkspaceConfig workspace;
};

struct SeriesRegistry {
  void set(SeriesData series) {
    const auto key = series.name;
    entries_[key] = std::move(series);
  }

  [[nodiscard]] bool contains(const std::string& name) const {
    return entries_.find(name) != entries_.end();
  }

  [[nodiscard]] const SeriesData& at(const std::string& name) const {
    return entries_.at(name);
  }

  [[nodiscard]] std::vector<std::string> names() const {
    std::vector<std::string> result;
    result.reserve(entries_.size());
    for (const auto& [key, _] : entries_) {
      result.push_back(key);
    }
    std::sort(result.begin(), result.end());
    return result;
  }

 private:
  std::unordered_map<std::string, SeriesData> entries_;
};

inline std::string trim(std::string_view value) {
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string_view::npos) {
    return {};
  }
  const auto last = value.find_last_not_of(" \t\r\n");
  return std::string(value.substr(first, last - first + 1));
}

inline std::vector<std::string> split_csv_line(std::string_view line) {
  std::vector<std::string> fields;
  std::string current;
  bool in_quotes = false;

  for (std::size_t i = 0; i < line.size(); ++i) {
    const char ch = line[i];
    if (ch == '"') {
      if (in_quotes && i + 1 < line.size() && line[i + 1] == '"') {
        current.push_back('"');
        ++i;
      } else {
        in_quotes = !in_quotes;
      }
      continue;
    }

    if (ch == ',' && !in_quotes) {
      fields.push_back(trim(current));
      current.clear();
      continue;
    }

    current.push_back(ch);
  }

  fields.push_back(trim(current));
  return fields;
}

inline bool parse_double(std::string_view text, double& value) {
  const auto trimmed = trim(text);
  if (trimmed.empty()) {
    return false;
  }
  const auto* begin = trimmed.data();
  const auto* end = begin + trimmed.size();
  auto result = std::from_chars(begin, end, value);
  return result.ec == std::errc{} && result.ptr == end;
}

inline std::optional<double> parse_iso_like(std::string_view text) {
  const auto trimmed = trim(text);
  if (trimmed.size() < 10) {
    return std::nullopt;
  }

  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
  int second = 0;

  const auto parse_ok = [&](const std::string& s) -> bool {
    if (std::sscanf(s.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
      return true;
    }
    hour = minute = second = 0;
    if (std::sscanf(s.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
      return true;
    }
    hour = minute = second = 0;
    if (std::sscanf(s.c_str(), "%d-%d-%d", &year, &month, &day) == 3) {
      return true;
    }
    return false;
  };

  std::string buffer(trimmed);
  if (!parse_ok(buffer)) {
    return std::nullopt;
  }

  std::tm tm{};
  tm.tm_year = year - 1900;
  tm.tm_mon = month - 1;
  tm.tm_mday = day;
  tm.tm_hour = hour;
  tm.tm_min = minute;
  tm.tm_sec = second;
  tm.tm_isdst = 0;

  const auto epoch = static_cast<double>(timegm(&tm));
  return epoch;
}

inline std::optional<double> parse_scalar(std::string_view text) {
  double numeric = 0.0;
  if (parse_double(text, numeric)) {
    return numeric;
  }
  return parse_iso_like(text);
}

inline ColumnKind infer_column_kind(const std::vector<std::string>& samples) {
  std::size_t numeric_count = 0;
  std::size_t datetime_count = 0;
  std::size_t non_empty = 0;

  for (const auto& sample : samples) {
    const auto trimmed = trim(sample);
    if (trimmed.empty()) {
      continue;
    }
    ++non_empty;
    double value = 0.0;
    if (parse_double(trimmed, value)) {
      ++numeric_count;
      continue;
    }
    if (parse_iso_like(trimmed).has_value()) {
      ++datetime_count;
    }
  }

  if (non_empty == 0) {
    return ColumnKind::Empty;
  }
  if (numeric_count == non_empty) {
    return ColumnKind::Numeric;
  }
  if (datetime_count == non_empty) {
    return ColumnKind::DateTime;
  }
  if (numeric_count > 0) {
    return ColumnKind::Numeric;
  }
  if (datetime_count > 0) {
    return ColumnKind::DateTime;
  }
  return ColumnKind::Text;
}

inline bool is_time_candidate(ColumnKind kind) {
  return kind == ColumnKind::Numeric || kind == ColumnKind::DateTime;
}

inline SourceCatalog load_csv_catalog(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input.is_open()) {
    throw std::runtime_error("Failed to open CSV file: " + path.string());
  }

  std::string header_line;
  if (!std::getline(input, header_line)) {
    throw std::runtime_error("CSV file has no header row: " + path.string());
  }

  const auto headers = split_csv_line(header_line);
  if (headers.empty()) {
    throw std::runtime_error("CSV header row is empty: " + path.string());
  }

  std::vector<std::vector<std::string>> rows;
  std::string line;
  while (std::getline(input, line)) {
    if (trim(line).empty()) {
      continue;
    }
    rows.push_back(split_csv_line(line));
  }

  std::vector<ColumnInfo> columns;
  columns.reserve(headers.size());
  for (std::size_t index = 0; index < headers.size(); ++index) {
    std::vector<std::string> samples;
    samples.reserve(rows.size());
    for (const auto& row : rows) {
      if (index < row.size()) {
        samples.push_back(row[index]);
      }
    }
    const auto kind = infer_column_kind(samples);
    columns.push_back(ColumnInfo{headers[index], kind, is_time_candidate(kind)});
  }

  SourceCatalog catalog;
  catalog.kind = SourceKind::Csv;
  catalog.path = path;
  catalog.source_name = path.stem().string();
  catalog.tables.push_back(TableCatalog{catalog.source_name, std::move(columns)});
  return catalog;
}

inline std::string sqlite_quote_identifier(std::string_view value) {
  std::string quoted = "\"";
  for (const char ch : value) {
    if (ch == '"') {
      quoted += "\"\"";
    } else {
      quoted.push_back(ch);
    }
  }
  quoted.push_back('"');
  return quoted;
}

inline std::vector<std::string> fetch_sqlite_tables(sqlite3* db) {
  std::vector<std::string> tables;
  const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name;";
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to list SQLite tables: " + std::string(sqlite3_errmsg(db)));
  }
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* text = sqlite3_column_text(stmt, 0);
    if (text != nullptr) {
      tables.emplace_back(reinterpret_cast<const char*>(text));
    }
  }
  sqlite3_finalize(stmt);
  return tables;
}

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
    const std::string lower = [&]() {
      std::string copy = decl_type;
      std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
      return copy;
    }();
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

inline std::vector<std::vector<std::string>> read_sqlite_rows(sqlite3* db, const std::string& table_name, const std::vector<ColumnInfo>& columns);

inline SourceCatalog load_sqlite_catalog(const std::filesystem::path& path) {
  sqlite3* db = nullptr;
  if (sqlite3_open_v2(path.string().c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    const std::string message = db != nullptr ? sqlite3_errmsg(db) : "unknown SQLite error";
    if (db != nullptr) {
      sqlite3_close(db);
    }
    throw std::runtime_error("Failed to open SQLite file: " + message);
  }

  SourceCatalog catalog;
  catalog.kind = SourceKind::Sqlite;
  catalog.path = path;
  catalog.source_name = path.stem().string();

  const auto tables = fetch_sqlite_tables(db);
  for (const auto& table_name : tables) {
    TableCatalog table;
    table.name = table_name;
    table.columns = fetch_sqlite_columns(db, table_name);
    if (table.columns.empty()) {
      continue;
    }
    const auto rows = read_sqlite_rows(db, table_name, table.columns);
    for (std::size_t col = 0; col < table.columns.size(); ++col) {
      std::vector<std::string> samples;
      samples.reserve(rows.size());
      for (const auto& row : rows) {
        if (col < row.size()) {
          samples.push_back(row[col]);
        }
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

inline std::optional<std::string> infer_time_column(const std::vector<ColumnInfo>& columns) {
  for (const auto& column : columns) {
    if (column.time_candidate) {
      return column.name;
    }
  }
  return std::nullopt;
}

inline std::optional<std::size_t> column_index_by_name(const std::vector<ColumnInfo>& columns, const std::string& name) {
  for (std::size_t i = 0; i < columns.size(); ++i) {
    if (columns[i].name == name) {
      return i;
    }
  }
  return std::nullopt;
}

inline std::vector<std::vector<std::string>> read_csv_rows(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input.is_open()) {
    throw std::runtime_error("Failed to open CSV file: " + path.string());
  }

  std::string header_line;
  if (!std::getline(input, header_line)) {
    return {};
  }

  std::vector<std::vector<std::string>> rows;
  std::string line;
  while (std::getline(input, line)) {
    if (trim(line).empty()) {
      continue;
    }
    rows.push_back(split_csv_line(line));
  }
  return rows;
}

inline void downsample_series(SeriesData& series, std::size_t max_points) {
  if (max_points == 0 || series.time.size() <= max_points) {
    return;
  }
  if (max_points == 1) {
    const auto first_time = series.time.front();
    const auto first_value = series.value.front();
    series.time = {first_time};
    series.value = {first_value};
    return;
  }

  std::vector<double> time;
  std::vector<double> value;
  time.reserve(max_points);
  value.reserve(max_points);

  const double last_index = static_cast<double>(series.time.size() - 1);
  for (std::size_t i = 0; i < max_points; ++i) {
    const auto sample_index = static_cast<std::size_t>(std::lround((last_index * static_cast<double>(i)) / static_cast<double>(max_points - 1)));
    const auto clamped = std::min(sample_index, series.time.size() - 1);
    time.push_back(series.time[clamped]);
    value.push_back(series.value[clamped]);
  }

  series.time = std::move(time);
  series.value = std::move(value);
}

inline LoadOutcome load_csv_series(const std::filesystem::path& path, const SeriesRequest& request) {
  const auto catalog = load_csv_catalog(path);
  const auto& table = catalog.tables.at(0);
  const auto rows = read_csv_rows(path);

  std::optional<std::size_t> time_index;
  if (!request.time_column.empty()) {
    time_index = column_index_by_name(table.columns, request.time_column);
  } else if (const auto inferred = infer_time_column(table.columns); inferred.has_value()) {
    time_index = column_index_by_name(table.columns, inferred.value());
  }
  const auto value_index = column_index_by_name(table.columns, request.value_column);

  if (!time_index.has_value()) {
    return LoadOutcome{false, {}, "Time column not found: " + request.time_column};
  }
  if (!value_index.has_value()) {
    return LoadOutcome{false, {}, "Value column not found: " + request.value_column};
  }

  SeriesData series;
  series.source_name = catalog.source_name;
  series.table_name = table.name;
  series.time_column = table.columns[*time_index].name;
  series.value_column = table.columns[*value_index].name;
  series.name = catalog.source_name + "." + series.value_column;

  for (const auto& row : rows) {
    if (*time_index >= row.size() || *value_index >= row.size()) {
      continue;
    }
    const auto time_value = parse_scalar(row[*time_index]);
    double numeric_value = 0.0;
    if (!time_value.has_value() || !parse_double(row[*value_index], numeric_value)) {
      continue;
    }
    series.time.push_back(*time_value);
    series.value.push_back(numeric_value);
  }

  if (request.max_points.has_value()) {
    downsample_series(series, *request.max_points);
  }

  return LoadOutcome{true, std::move(series), {}};
}

inline std::vector<std::vector<std::string>> read_sqlite_rows(sqlite3* db, const std::string& table_name, const std::vector<ColumnInfo>& columns) {
  std::vector<std::vector<std::string>> rows;
  if (columns.empty()) {
    return rows;
  }

  std::string sql = "SELECT ";
  for (std::size_t i = 0; i < columns.size(); ++i) {
    if (i > 0) {
      sql += ", ";
    }
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
  }

  sqlite3_finalize(stmt);
  return rows;
}

inline LoadOutcome load_sqlite_series(const std::filesystem::path& path, const std::string& table_name, const SeriesRequest& request) {
  sqlite3* db = nullptr;
  if (sqlite3_open_v2(path.string().c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    const std::string message = db != nullptr ? sqlite3_errmsg(db) : "unknown SQLite error";
    if (db != nullptr) {
      sqlite3_close(db);
    }
    return LoadOutcome{false, {}, "Failed to open SQLite file: " + message};
  }

  const auto columns = fetch_sqlite_columns(db, table_name);
  const auto rows = read_sqlite_rows(db, table_name, columns);
  sqlite3_close(db);

  std::optional<std::size_t> time_index;
  if (!request.time_column.empty()) {
    time_index = column_index_by_name(columns, request.time_column);
  } else if (const auto inferred = infer_time_column(columns); inferred.has_value()) {
    time_index = column_index_by_name(columns, inferred.value());
  }
  const auto value_index = column_index_by_name(columns, request.value_column);

  if (!time_index.has_value()) {
    return LoadOutcome{false, {}, "Time column not found: " + request.time_column};
  }
  if (!value_index.has_value()) {
    return LoadOutcome{false, {}, "Value column not found: " + request.value_column};
  }

  SeriesData series;
  series.source_name = path.stem().string();
  series.table_name = table_name;
  series.time_column = columns[*time_index].name;
  series.value_column = columns[*value_index].name;
  series.name = series.source_name + "." + table_name + "." + series.value_column;

  for (const auto& row : rows) {
    if (*time_index >= row.size() || *value_index >= row.size()) {
      continue;
    }
    const auto time_value = parse_scalar(row[*time_index]);
    double numeric_value = 0.0;
    if (!time_value.has_value() || !parse_double(row[*value_index], numeric_value)) {
      continue;
    }
    series.time.push_back(*time_value);
    series.value.push_back(numeric_value);
  }

  if (request.max_points.has_value()) {
    downsample_series(series, *request.max_points);
  }

  return LoadOutcome{true, std::move(series), {}};
}

inline std::vector<std::string> dotted_parts(std::string_view name) {
  std::vector<std::string> parts;
  std::string current;
  for (const char ch : name) {
    if (ch == '.') {
      if (!current.empty()) {
        parts.push_back(current);
        current.clear();
      }
      continue;
    }
    current.push_back(ch);
  }
  if (!current.empty()) {
    parts.push_back(current);
  }
  return parts;
}

inline void insert_tree_path(TreeNode& node, const std::vector<std::string>& parts, std::size_t index, const std::string& prefix) {
  if (index >= parts.size()) {
    return;
  }

  const auto& part = parts[index];
  const auto next_full = prefix.empty() ? part : prefix + "." + part;
  auto it = std::find_if(node.children.begin(), node.children.end(), [&](const TreeNode& child) {
    return child.label == part;
  });

  if (it == node.children.end()) {
    node.children.push_back(TreeNode{part, next_full, index + 1 == parts.size(), {}});
    it = std::prev(node.children.end());
  } else if (index + 1 == parts.size()) {
    it->leaf = true;
    it->full_name = next_full;
  }

  insert_tree_path(*it, parts, index + 1, next_full);
}

inline TreeNode build_variable_tree(const std::vector<std::string>& names) {
  TreeNode root{"", "", false, {}};
  for (const auto& name : names) {
    insert_tree_path(root, dotted_parts(name), 0, "");
  }

  std::function<void(TreeNode&)> sort_tree = [&](TreeNode& node) {
    std::sort(node.children.begin(), node.children.end(), [](const TreeNode& a, const TreeNode& b) {
      return a.label < b.label;
    });
    for (auto& child : node.children) {
      sort_tree(child);
    }
  };
  sort_tree(root);
  return root;
}

inline double interpolate_value(const SeriesData& series, double x) {
  if (series.time.empty()) {
    return 0.0;
  }
  if (series.time.size() == 1) {
    return series.value.front();
  }
  if (x <= series.time.front()) {
    return series.value.front();
  }
  if (x >= series.time.back()) {
    return series.value.back();
  }

  const auto upper = std::upper_bound(series.time.begin(), series.time.end(), x);
  const std::size_t idx = static_cast<std::size_t>(std::distance(series.time.begin(), upper));
  const std::size_t left = idx - 1;
  const std::size_t right = idx;
  const double x0 = series.time[left];
  const double x1 = series.time[right];
  const double y0 = series.value[left];
  const double y1 = series.value[right];
  if (x1 == x0) {
    return y0;
  }
  const double ratio = (x - x0) / (x1 - x0);
  return y0 + ratio * (y1 - y0);
}

inline SeriesData remap_to_grid(const SeriesData& series, const std::vector<double>& target_time) {
  SeriesData remapped;
  remapped.name = series.name;
  remapped.source_name = series.source_name;
  remapped.table_name = series.table_name;
  remapped.time_column = series.time_column;
  remapped.value_column = series.value_column;
  remapped.time = target_time;
  remapped.value.reserve(target_time.size());
  for (const auto x : target_time) {
    remapped.value.push_back(interpolate_value(series, x));
  }
  return remapped;
}

inline SeriesData apply_series_series(const SeriesData& lhs, const SeriesData& rhs, const std::function<double(double, double)>& op, const std::string& name) {
  SeriesData result;
  result.name = name;
  result.source_name = lhs.source_name.empty() ? rhs.source_name : lhs.source_name;
  result.table_name = lhs.table_name;
  result.time = lhs.time;
  const auto remapped = remap_to_grid(rhs, lhs.time);
  result.value.reserve(lhs.value.size());
  for (std::size_t i = 0; i < lhs.value.size(); ++i) {
    result.value.push_back(op(lhs.value[i], remapped.value[i]));
  }
  return result;
}

inline SeriesData apply_series_scalar(const SeriesData& lhs, double rhs, const std::function<double(double, double)>& op, const std::string& name) {
  SeriesData result = lhs;
  result.name = name;
  for (auto& value : result.value) {
    value = op(value, rhs);
  }
  return result;
}

inline SeriesData apply_scalar_series(double lhs, const SeriesData& rhs, const std::function<double(double, double)>& op, const std::string& name) {
  SeriesData result = rhs;
  result.name = name;
  for (auto& value : result.value) {
    value = op(lhs, value);
  }
  return result;
}

inline SeriesData series_add(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return a + b; }, lhs.name + "_plus_" + rhs.name);
}

inline SeriesData series_sub(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return a - b; }, lhs.name + "_minus_" + rhs.name);
}

inline SeriesData series_mul(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return a * b; }, lhs.name + "_times_" + rhs.name);
}

inline SeriesData series_div(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return b == 0.0 ? 0.0 : a / b; }, lhs.name + "_div_" + rhs.name);
}

inline SeriesData series_add(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return a + b; }, lhs.name + "_plus_scalar");
}

inline SeriesData series_sub(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return a - b; }, lhs.name + "_minus_scalar");
}

inline SeriesData series_mul(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return a * b; }, lhs.name + "_times_scalar");
}

inline SeriesData series_div(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return b == 0.0 ? 0.0 : a / b; }, lhs.name + "_div_scalar");
}

inline SeriesData series_add(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return a + b; }, rhs.name + "_scalar_plus");
}

inline SeriesData series_sub(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return a - b; }, rhs.name + "_scalar_minus");
}

inline SeriesData series_mul(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return a * b; }, rhs.name + "_scalar_times");
}

inline SeriesData series_div(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return b == 0.0 ? 0.0 : a / b; }, rhs.name + "_scalar_div");
}

inline SeriesData normalize_series_name(SeriesData series, const std::string& name) {
  series.name = name;
  return series;
}

inline SeriesData evaluate_expression(const std::string& expression, const SeriesRegistry& registry) {
  sol::state lua;
  lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);

  lua.new_usertype<SeriesData>(
    "SeriesData",
    sol::constructors<SeriesData()>(),
    "time", &SeriesData::time,
    "value", &SeriesData::value,
    "name", &SeriesData::name,
    sol::meta_function::addition,
    sol::overload(
      [](const SeriesData& lhs, const SeriesData& rhs) { return series_add(lhs, rhs); },
      [](const SeriesData& lhs, double rhs) { return series_add(lhs, rhs); },
      [](double lhs, const SeriesData& rhs) { return series_add(lhs, rhs); }
    ),
    sol::meta_function::subtraction,
    sol::overload(
      [](const SeriesData& lhs, const SeriesData& rhs) { return series_sub(lhs, rhs); },
      [](const SeriesData& lhs, double rhs) { return series_sub(lhs, rhs); },
      [](double lhs, const SeriesData& rhs) { return series_sub(lhs, rhs); }
    ),
    sol::meta_function::multiplication,
    sol::overload(
      [](const SeriesData& lhs, const SeriesData& rhs) { return series_mul(lhs, rhs); },
      [](const SeriesData& lhs, double rhs) { return series_mul(lhs, rhs); },
      [](double lhs, const SeriesData& rhs) { return series_mul(lhs, rhs); }
    ),
    sol::meta_function::division,
    sol::overload(
      [](const SeriesData& lhs, const SeriesData& rhs) { return series_div(lhs, rhs); },
      [](const SeriesData& lhs, double rhs) { return series_div(lhs, rhs); },
      [](double lhs, const SeriesData& rhs) { return series_div(lhs, rhs); }
    )
  );

  lua.set_function("series", [&](const std::string& name) -> SeriesData {
    if (!registry.contains(name)) {
      throw std::runtime_error("Unknown series: " + name);
    }
    return registry.at(name);
  });

  const auto result = lua.safe_script("return " + expression, sol::script_pass_on_error);
  if (!result.valid()) {
    const sol::error err = result;
    throw std::runtime_error("Expression error: " + std::string(err.what()));
  }

  if (result.get_type() == sol::type::userdata) {
    SeriesData series = result.get<SeriesData>();
    return series;
  }

  if (result.get_type() == sol::type::number) {
    const auto value = result.get<double>();
    if (registry.names().empty()) {
      throw std::runtime_error("Scalar expressions require at least one reference series");
    }
    const auto& reference = registry.at(registry.names().front());
    SeriesData series;
    series.name = expression;
    series.time = reference.time;
    series.value.assign(reference.time.size(), value);
    return series;
  }

  throw std::runtime_error("Expression did not evaluate to a series: " + expression);
}

inline std::string to_string(SourceKind kind) {
  switch (kind) {
    case SourceKind::Csv: return "csv";
    case SourceKind::Sqlite: return "sqlite";
  }
  return "csv";
}

inline SourceKind source_kind_from_string(const std::string& value) {
  if (value == "csv") {
    return SourceKind::Csv;
  }
  if (value == "sqlite") {
    return SourceKind::Sqlite;
  }
  throw std::runtime_error("Unknown source kind: " + value);
}

inline void to_json(nlohmann::json& j, const ProjectSource& source) {
  j = nlohmann::json{
    {"kind", to_string(source.kind)},
    {"path", source.path},
    {"alias", source.alias},
    {"table_name", source.table_name},
    {"time_column", source.time_column},
    {"selected_variables", source.selected_variables}
  };
}

inline void from_json(const nlohmann::json& j, ProjectSource& source) {
  source.kind = source_kind_from_string(j.at("kind").get<std::string>());
  source.path = j.at("path").get<std::string>();
  source.alias = j.value("alias", std::string{});
  if (j.contains("table_name") && !j.at("table_name").is_null()) {
    source.table_name = j.at("table_name").get<std::string>();
  }
  if (j.contains("time_column") && !j.at("time_column").is_null()) {
    source.time_column = j.at("time_column").get<std::string>();
  }
  if (j.contains("selected_variables")) {
    source.selected_variables = j.at("selected_variables").get<std::vector<std::string>>();
  }
}

inline void to_json(nlohmann::json& j, const PlotSeriesConfig& series) {
  j = nlohmann::json{
    {"name", series.name},
    {"expression", series.expression},
    {"source_alias", series.source_alias},
    {"source_path", series.source_path},
    {"table_name", series.table_name},
    {"time_column", series.time_column},
    {"value_column", series.value_column},
    {"visible", series.visible},
    {"derived", series.derived},
    {"color", series.color}
  };
}

inline void from_json(const nlohmann::json& j, PlotSeriesConfig& series) {
  series.name = j.at("name").get<std::string>();
  series.expression = j.value("expression", std::string{});
  if (j.contains("source_alias") && !j.at("source_alias").is_null()) {
    series.source_alias = j.at("source_alias").get<std::string>();
  }
  if (j.contains("source_path") && !j.at("source_path").is_null()) {
    series.source_path = j.at("source_path").get<std::string>();
  }
  if (j.contains("table_name") && !j.at("table_name").is_null()) {
    series.table_name = j.at("table_name").get<std::string>();
  }
  if (j.contains("time_column") && !j.at("time_column").is_null()) {
    series.time_column = j.at("time_column").get<std::string>();
  }
  if (j.contains("value_column") && !j.at("value_column").is_null()) {
    series.value_column = j.at("value_column").get<std::string>();
  }
  series.visible = j.value("visible", true);
  series.derived = j.value("derived", false);
  if (j.contains("color")) {
    series.color = j.at("color").get<std::array<double, 4>>();
  }
}

inline void to_json(nlohmann::json& j, const PlotTabConfig& view) {
  j = nlohmann::json{
    {"title", view.title},
    {"series", view.series},
    {"autoscale_x", view.autoscale_x},
    {"autoscale_y", view.autoscale_y},
    {"x_range", view.x_range},
    {"y_range", view.y_range},
    {"active_series_index", view.active_series_index},
    {"expression_draft", view.expression_draft}
  };
}

inline void from_json(const nlohmann::json& j, PlotViewConfig& view) {
  view.title = j.at("title").get<std::string>();
  if (j.contains("series")) {
    view.series = j.at("series").get<std::vector<PlotSeriesConfig>>();
  }
  view.autoscale_x = j.value("autoscale_x", true);
  view.autoscale_y = j.value("autoscale_y", true);
  if (j.contains("x_range") && !j.at("x_range").is_null()) {
    view.x_range = j.at("x_range").get<std::array<double, 2>>();
  }
  if (j.contains("y_range") && !j.at("y_range").is_null()) {
    view.y_range = j.at("y_range").get<std::array<double, 2>>();
  }
  view.active_series_index = j.value("active_series_index", std::size_t{0});
  view.expression_draft = j.value("expression_draft", std::string{});
}

inline void to_json(nlohmann::json& j, const AnalysisWindowConfig& window) {
  j = nlohmann::json{
    {"title", window.title},
    {"tabs", window.tabs},
    {"active_tab", window.active_tab}
  };
}

inline void from_json(const nlohmann::json& j, AnalysisWindowConfig& window) {
  window.title = j.at("title").get<std::string>();
  if (j.contains("tabs")) {
    window.tabs = j.at("tabs").get<std::vector<PlotTabConfig>>();
  }
  window.active_tab = j.value("active_tab", std::size_t{0});
}

inline void to_json(nlohmann::json& j, const WorkspaceConfig& workspace) {
  j = nlohmann::json{
    {"windows", workspace.windows},
    {"point_budget", workspace.point_budget}
  };
}

inline void from_json(const nlohmann::json& j, WorkspaceConfig& workspace) {
  if (j.contains("windows")) {
    workspace.windows = j.at("windows").get<std::vector<AnalysisWindowConfig>>();
  }
  workspace.point_budget = j.value("point_budget", std::size_t{50000});
}

inline void to_json(nlohmann::json& j, const ProjectState& project) {
  j = nlohmann::json{
    {"sources", project.sources},
    {"workspace", project.workspace}
  };
}

inline void from_json(const nlohmann::json& j, ProjectState& project) {
  if (j.contains("sources")) {
    project.sources = j.at("sources").get<std::vector<ProjectSource>>();
  }
  if (j.contains("workspace")) {
    project.workspace = j.at("workspace").get<WorkspaceConfig>();
  } else if (j.contains("views")) {
    AnalysisWindowConfig window;
    window.title = "Workspace";
    window.tabs = j.at("views").get<std::vector<PlotTabConfig>>();
    project.workspace.windows.push_back(std::move(window));
  }
}

inline std::filesystem::path project_relative_path(const std::filesystem::path& project_file, const std::filesystem::path& target) {
  std::error_code ec;
  const auto relative = std::filesystem::relative(target, project_file.parent_path(), ec);
  if (!ec && !relative.empty() && relative.native().find("..") != 0) {
    return relative;
  }
  return target;
}

inline std::filesystem::path resolve_project_path(const std::filesystem::path& project_file, const std::string& stored_path) {
  const std::filesystem::path stored(stored_path);
  if (stored.is_absolute()) {
    return stored;
  }
  return project_file.parent_path() / stored;
}

inline void save_project(const std::filesystem::path& file, ProjectState project) {
  for (auto& source : project.sources) {
    source.path = project_relative_path(file, source.path).string();
  }
  const nlohmann::json json = project;
  std::ofstream output(file);
  if (!output.is_open()) {
    throw std::runtime_error("Failed to open project file for writing: " + file.string());
  }
  output << json.dump(2) << '\n';
}

inline ProjectState load_project(const std::filesystem::path& file) {
  std::ifstream input(file);
  if (!input.is_open()) {
    throw std::runtime_error("Failed to open project file for reading: " + file.string());
  }
  const auto json = nlohmann::json::parse(input);
  ProjectState project = json.get<ProjectState>();
  for (auto& source : project.sources) {
    source.path = resolve_project_path(file, source.path).string();
  }
  return project;
}

inline bool operator==(const ProjectSource& lhs, const ProjectSource& rhs) {
  return lhs.kind == rhs.kind
      && lhs.path == rhs.path
      && lhs.alias == rhs.alias
      && lhs.table_name == rhs.table_name
      && lhs.time_column == rhs.time_column
      && lhs.selected_variables == rhs.selected_variables;
}

inline bool operator==(const PlotSeriesConfig& lhs, const PlotSeriesConfig& rhs) {
  return lhs.name == rhs.name
      && lhs.expression == rhs.expression
      && lhs.source_alias == rhs.source_alias
      && lhs.source_path == rhs.source_path
      && lhs.table_name == rhs.table_name
      && lhs.time_column == rhs.time_column
      && lhs.value_column == rhs.value_column
      && lhs.visible == rhs.visible
      && lhs.derived == rhs.derived
      && lhs.color == rhs.color;
}

inline bool operator==(const PlotTabConfig& lhs, const PlotTabConfig& rhs) {
  return lhs.title == rhs.title
      && lhs.series == rhs.series
      && lhs.autoscale_x == rhs.autoscale_x
      && lhs.autoscale_y == rhs.autoscale_y
      && lhs.x_range == rhs.x_range
      && lhs.y_range == rhs.y_range
      && lhs.active_series_index == rhs.active_series_index
      && lhs.expression_draft == rhs.expression_draft;
}

inline bool operator==(const AnalysisWindowConfig& lhs, const AnalysisWindowConfig& rhs) {
  return lhs.title == rhs.title
      && lhs.tabs == rhs.tabs
      && lhs.active_tab == rhs.active_tab;
}

inline bool operator==(const WorkspaceConfig& lhs, const WorkspaceConfig& rhs) {
  return lhs.windows == rhs.windows
      && lhs.point_budget == rhs.point_budget;
}

inline bool operator==(const ProjectState& lhs, const ProjectState& rhs) {
  return lhs.sources == rhs.sources && lhs.workspace == rhs.workspace;
}

inline std::vector<std::string> missing_variables(const std::vector<std::string>& selected, const std::vector<std::string>& available) {
  std::set<std::string> available_set(available.begin(), available.end());
  std::vector<std::string> missing;
  for (const auto& name : selected) {
    if (!available_set.contains(name)) {
      missing.push_back(name);
    }
  }
  return missing;
}

} // namespace tsv
