#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "model/types.hpp"

namespace tsv {

/// Configuration for a single series within a plot tab.
struct PlotSeriesConfig {
  std::string name;                                    ///< Display name for the series.
  std::string expression;                              ///< Lua expression for derived series.
  std::optional<std::string> source_alias;             ///< Source alias for binding.
  std::optional<std::string> source_path;              ///< Source file path for binding.
  std::optional<std::string> table_name;               ///< Table name for binding.
  std::optional<std::string> time_column;              ///< Time column for binding.
  std::optional<std::string> value_column;             ///< Value column for binding.
  bool visible{true};                                  ///< Whether the series is visible on the plot.
  bool derived{false};                                 ///< Whether this is a derived (expression-based) series.
  std::array<double, 4> color{1.0, 0.0, 0.0, 1.0};    ///< RGBA color for the plot line.
};

/// Configuration for a single plot tab within an analysis window.
struct PlotTabConfig {
  std::string title;                                    ///< Tab title.
  std::vector<PlotSeriesConfig> series;                 ///< Series displayed in this tab.
  bool autoscale_x{true};                               ///< Whether the X axis auto-scales.
  bool autoscale_y{true};                               ///< Whether the Y axis auto-scales.
  std::optional<std::array<double, 2>> x_range;         ///< Fixed X axis range (when autoscale is off).
  std::optional<std::array<double, 2>> y_range;         ///< Fixed Y axis range (when autoscale is off).
  std::size_t active_series_index{0};                   ///< Index of the currently selected series.
  std::string expression_draft;                         ///< In-progress expression text in the editor.
};

/// Alias for backward compatibility; represents a single plot view.
using PlotViewConfig = PlotTabConfig;

/// Configuration for an analysis window containing multiple plot tabs.
struct AnalysisWindowConfig {
  std::string title;                        ///< Window title.
  std::vector<PlotTabConfig> tabs;          ///< Tabs in this window.
  std::size_t active_tab{0};               ///< Index of the active tab.
};

/// Configuration for the entire workspace (all windows).
struct WorkspaceConfig {
  std::vector<AnalysisWindowConfig> windows;   ///< All analysis windows.
  std::size_t point_budget{50000};            ///< Maximum points per series before downsampling.
};

/// Complete project state for serialization.
struct ProjectState {
  std::vector<ProjectSource> sources;   ///< Open sources.
  WorkspaceConfig workspace;            ///< Workspace layout and series configuration.
};

} // namespace tsv