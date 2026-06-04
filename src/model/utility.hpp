#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <charconv>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "model/types.hpp"

namespace tsv {

inline std::string trim(std::string_view value) {
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string_view::npos) return {};
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
        current.push_back('"'); ++i;
      } else { in_quotes = !in_quotes; }
      continue;
    }
    if (ch == ',' && !in_quotes) {
      fields.push_back(trim(current)); current.clear(); continue;
    }
    current.push_back(ch);
  }
  fields.push_back(trim(current));
  return fields;
}

inline bool parse_double(std::string_view text, double& value) {
  const auto trimmed = trim(text);
  if (trimmed.empty()) return false;
  const auto* begin = trimmed.data();
  const auto* end = begin + trimmed.size();
  auto result = std::from_chars(begin, end, value);
  return result.ec == std::errc{} && result.ptr == end;
}

inline std::optional<double> parse_iso_like(std::string_view text) {
  const auto trimmed = trim(text);
  if (trimmed.size() < 10) return std::nullopt;
  int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
  const auto parse_ok = [&](const std::string& s) -> bool {
    if (std::sscanf(s.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) return true;
    hour = minute = second = 0;
    if (std::sscanf(s.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) return true;
    hour = minute = second = 0;
    if (std::sscanf(s.c_str(), "%d-%d-%d", &year, &month, &day) == 3) return true;
    return false;
  };
  std::string buffer(trimmed);
  if (!parse_ok(buffer)) return std::nullopt;
  std::tm tm{};
  tm.tm_year = year - 1900; tm.tm_mon = month - 1; tm.tm_mday = day;
  tm.tm_hour = hour; tm.tm_min = minute; tm.tm_sec = second; tm.tm_isdst = 0;
  return static_cast<double>(timegm(&tm));
}

inline std::optional<double> parse_scalar(std::string_view text) {
  double numeric = 0.0;
  if (parse_double(text, numeric)) return numeric;
  return parse_iso_like(text);
}

inline ColumnKind infer_column_kind(const std::vector<std::string>& samples) {
  std::size_t numeric_count = 0, datetime_count = 0, non_empty = 0;
  for (const auto& sample : samples) {
    const auto trimmed = trim(sample);
    if (trimmed.empty()) continue;
    ++non_empty;
    double value = 0.0;
    if (parse_double(trimmed, value)) { ++numeric_count; continue; }
    if (parse_iso_like(trimmed).has_value()) ++datetime_count;
  }
  if (non_empty == 0) return ColumnKind::Empty;
  if (numeric_count == non_empty) return ColumnKind::Numeric;
  if (datetime_count == non_empty) return ColumnKind::DateTime;
  if (numeric_count > 0) return ColumnKind::Numeric;
  if (datetime_count > 0) return ColumnKind::DateTime;
  return ColumnKind::Text;
}

inline bool is_time_candidate(ColumnKind kind) {
  return kind == ColumnKind::Numeric || kind == ColumnKind::DateTime;
}

inline std::optional<std::string> infer_time_column(const std::vector<ColumnInfo>& columns) {
  for (const auto& column : columns) {
    if (column.time_candidate) return column.name;
  }
  return std::nullopt;
}

inline std::optional<std::size_t> column_index_by_name(const std::vector<ColumnInfo>& columns, const std::string& name) {
  for (std::size_t i = 0; i < columns.size(); ++i) {
    if (columns[i].name == name) return i;
  }
  return std::nullopt;
}

inline void downsample_series(SeriesData& series, std::size_t max_points) {
  if (max_points == 0 || series.time.size() <= max_points) return;
  if (max_points == 1) {
    series.time = {series.time.front()};
    series.value = {series.value.front()};
    return;
  }
  std::vector<double> time, value;
  time.reserve(max_points); value.reserve(max_points);
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

inline std::vector<std::vector<std::string>> read_csv_rows(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input.is_open()) throw std::runtime_error("Failed to open CSV file: " + path.string());
  std::string header_line;
  if (!std::getline(input, header_line)) return {};
  std::vector<std::vector<std::string>> rows;
  std::string line;
  while (std::getline(input, line)) {
    if (trim(line).empty()) continue;
    rows.push_back(split_csv_line(line));
  }
  return rows;
}

inline std::vector<std::string> dotted_parts(std::string_view name) {
  std::vector<std::string> parts;
  std::string current;
  for (const char ch : name) {
    if (ch == '.') {
      if (!current.empty()) { parts.push_back(current); current.clear(); }
      continue;
    }
    current.push_back(ch);
  }
  if (!current.empty()) parts.push_back(current);
  return parts;
}

inline void insert_tree_path(TreeNode& node, const std::vector<std::string>& parts, std::size_t index, const std::string& prefix) {
  if (index >= parts.size()) return;
  const auto& part = parts[index];
  const auto next_full = prefix.empty() ? part : prefix + "." + part;
  auto it = std::find_if(node.children.begin(), node.children.end(), [&](const TreeNode& child) { return child.label == part; });
  if (it == node.children.end()) {
    node.children.push_back(TreeNode{part, next_full, index + 1 == parts.size(), {}});
    it = std::prev(node.children.end());
  } else if (index + 1 == parts.size()) {
    it->leaf = true; it->full_name = next_full;
  }
  insert_tree_path(*it, parts, index + 1, next_full);
}

inline TreeNode build_variable_tree(const std::vector<std::string>& names) {
  TreeNode root{"", "", false, {}};
  for (const auto& name : names) insert_tree_path(root, dotted_parts(name), 0, "");
  std::function<void(TreeNode&)> sort_tree = [&](TreeNode& node) {
    std::sort(node.children.begin(), node.children.end(), [](const TreeNode& a, const TreeNode& b) { return a.label < b.label; });
    for (auto& child : node.children) sort_tree(child);
  };
  sort_tree(root);
  return root;
}

inline std::vector<std::string> missing_variables(const std::vector<std::string>& selected, const std::vector<std::string>& available) {
  std::set<std::string> available_set(available.begin(), available.end());
  std::vector<std::string> missing;
  for (const auto& name : selected) {
    if (!available_set.contains(name)) missing.push_back(name);
  }
  return missing;
}

} // namespace tsv