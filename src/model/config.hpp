#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "model/types.hpp"

namespace tsv {

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

} // namespace tsv