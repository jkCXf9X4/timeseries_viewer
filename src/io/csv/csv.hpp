#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "model/types.hpp"
#include "model/utility.hpp"

namespace tsv {

/// Loads the catalog (column metadata) from a CSV file.
/// @param path  Path to the CSV file.
/// @return A SourceCatalog with one table entry describing the columns.
/// @throws std::runtime_error if the file cannot be opened or has no header row.
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

  std::vector<std::vector<std::string>> samples;
  std::string line;
  constexpr std::size_t kSampleLimit = 1000;
  while (std::getline(input, line)) {
    if (trim(line).empty()) continue;
    samples.push_back(split_csv_line(line));
    if (samples.size() >= kSampleLimit) break;
  }

  std::vector<ColumnInfo> columns;
  columns.reserve(headers.size());
  for (std::size_t index = 0; index < headers.size(); ++index) {
    std::vector<std::string> col_samples;
    col_samples.reserve(samples.size());
    for (const auto& row : samples) {
      if (index < row.size()) col_samples.push_back(row[index]);
    }
    const auto kind = infer_column_kind(col_samples);
    columns.push_back(ColumnInfo{headers[index], kind, is_time_candidate(kind)});
  }

  SourceCatalog catalog;
  catalog.kind = SourceKind::Csv;
  catalog.path = path;
  catalog.source_name = path.stem().string();
  catalog.tables.push_back(TableCatalog{catalog.source_name, std::move(columns)});
  return catalog;
}

/// Loads a series from a CSV file using pre-discovered column metadata.
/// @param path     Path to the CSV file.
/// @param columns  Column metadata (from load_csv_catalog).
/// @param request  Series request specifying time/value columns and optional max points.
/// @return LoadOutcome with the loaded series on success, or an error description on failure.
inline LoadOutcome load_csv_series_streaming(const std::filesystem::path& path, const std::vector<ColumnInfo>& columns, const SeriesRequest& request) {
  std::ifstream input(path);
  if (!input.is_open()) {
    return LoadOutcome{false, {}, "Failed to open CSV file: " + path.string()};
  }

  std::string header_line;
  if (!std::getline(input, header_line)) {
    return LoadOutcome{false, {}, "CSV file has no header row"};
  }

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
  if (!columns.empty()) {
    series.source_name = path.stem().string();
    series.table_name = path.stem().string();
    series.time_column = columns[*time_index].name;
    series.value_column = columns[*value_index].name;
    series.name = series.source_name + "." + series.value_column;
  }

  std::string line;
  while (std::getline(input, line)) {
    if (trim(line).empty()) continue;
    const auto row = split_csv_line(line);
    if (*time_index >= row.size() || *value_index >= row.size()) continue;
    const auto time_value = parse_scalar(row[*time_index]);
    double numeric_value = 0.0;
    if (!time_value.has_value() || !parse_double(row[*value_index], numeric_value)) continue;
    series.time.push_back(*time_value);
    series.value.push_back(numeric_value);
  }

  if (request.max_points.has_value()) {
    downsample_series(series, *request.max_points);
  }

  return LoadOutcome{true, std::move(series), {}};
}

/// Convenience function: loads a series from a CSV file in one call.
/// Internally calls load_csv_catalog then load_csv_series_streaming.
/// @param path     Path to the CSV file.
/// @param request  Series request specifying time/value columns and optional max points.
/// @return LoadOutcome with the loaded series on success, or an error description on failure.
inline LoadOutcome load_csv_series(const std::filesystem::path& path, const SeriesRequest& request) {
  const auto catalog = load_csv_catalog(path);
  const auto& table = catalog.tables.at(0);
  return load_csv_series_streaming(path, table.columns, request);
}

} // namespace tsv