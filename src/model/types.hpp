#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tsv {

enum class SourceKind { Csv, Sqlite };
enum class ColumnKind { Numeric, DateTime, Text, Empty };

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

} // namespace tsv