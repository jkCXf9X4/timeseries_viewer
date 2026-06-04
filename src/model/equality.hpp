#pragma once

#include "model/config.hpp"
#include "model/types.hpp"

namespace tsv {

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

} // namespace tsv