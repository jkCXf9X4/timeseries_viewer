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

/// Kind of data source.
enum class SourceKind { Csv, Sqlite };

/// Inferred kind of a column's data.
enum class ColumnKind { Numeric, DateTime, Text, Empty };

/// Metadata for a single column in a source table.
struct ColumnInfo {
  std::string name;            ///< Column name as it appears in the source.
  ColumnKind kind{ColumnKind::Text};  ///< Inferred data kind.
  bool time_candidate{false};  ///< Whether this column is a candidate for the time axis.
};

/// Metadata for a single table in a source.
struct TableCatalog {
  std::string name;                     ///< Table name.
  std::vector<ColumnInfo> columns;      ///< Column descriptors.
};

/// Full catalog of a data source (CSV file or SQLite database).
struct SourceCatalog {
  SourceKind kind{SourceKind::Csv};       ///< Source type.
  std::string source_name;                ///< Human-readable source name (typically the stem).
  std::filesystem::path path;             ///< Path to the source file.
  std::vector<TableCatalog> tables;       ///< Tables discovered in the source.
};

/// Loaded time-series data for one variable.
struct SeriesData {
  std::string name;           ///< Fully qualified series name.
  std::vector<double> time;   ///< Time values (numeric epoch representation).
  std::vector<double> value;  ///< Corresponding data values.
  std::string source_name;    ///< Name of the source this series was loaded from.
  std::string table_name;     ///< Name of the table this series was loaded from.
  std::string time_column;    ///< Name of the column used for time values.
  std::string value_column;   ///< Name of the column used for data values.
};

/// Parameters for requesting a series load from a source.
struct SeriesRequest {
  std::optional<std::string> table_name;    ///< Table to load from (optional for single-table sources).
  std::string time_column;                  ///< Column to use for time values.
  std::string value_column;                 ///< Column to use for data values.
  std::optional<std::size_t> max_points;    ///< Maximum number of points (downsampled if set).
};

/// Outcome of a series load operation.
struct LoadOutcome {
  bool ok{false};         ///< Whether the load succeeded.
  SeriesData series;      ///< Loaded series data (valid only if ok is true).
  std::string error;      ///< Error message (valid only if ok is false).
};

/// A node in a hierarchical variable browser tree.
struct TreeNode {
  std::string label;                ///< Display label for this node.
  std::string full_name;            ///< Fully qualified name for leaf nodes.
  bool leaf{false};                 ///< Whether this is a leaf (selectable variable).
  std::vector<TreeNode> children;   ///< Child nodes.
};

/// Describes a source as stored in a project file.
struct ProjectSource {
  SourceKind kind{SourceKind::Csv};               ///< Source type.
  std::string path;                               ///< Path to the source file.
  std::string alias;                              ///< User-assigned alias.
  std::optional<std::string> table_name;          ///< Selected table (optional for CSV).
  std::optional<std::string> time_column;         ///< Override for the time column.
  std::vector<std::string> selected_variables;    ///< Variables selected from this source.
};

} // namespace tsv