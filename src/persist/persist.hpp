#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <nlohmann/json.hpp>

#include "model/config.hpp"
#include "model/equality.hpp"
#include "model/types.hpp"

namespace tsv {

inline std::string to_string(SourceKind kind) {
  switch (kind) {
    case SourceKind::Csv: return "csv";
    case SourceKind::Sqlite: return "sqlite";
  }
  return "csv";
}

inline SourceKind source_kind_from_string(const std::string& value) {
  if (value == "csv") return SourceKind::Csv;
  if (value == "sqlite") return SourceKind::Sqlite;
  throw std::runtime_error("Unknown source kind: " + value);
}

inline void to_json(nlohmann::json& j, const ProjectSource& source) {
  j = nlohmann::json{
    {"kind", to_string(source.kind)}, {"path", source.path}, {"alias", source.alias},
    {"table_name", source.table_name}, {"time_column", source.time_column},
    {"selected_variables", source.selected_variables}
  };
}

inline void from_json(const nlohmann::json& j, ProjectSource& source) {
  source.kind = source_kind_from_string(j.at("kind").get<std::string>());
  source.path = j.at("path").get<std::string>();
  source.alias = j.value("alias", std::string{});
  if (j.contains("table_name") && !j.at("table_name").is_null()) source.table_name = j.at("table_name").get<std::string>();
  if (j.contains("time_column") && !j.at("time_column").is_null()) source.time_column = j.at("time_column").get<std::string>();
  if (j.contains("selected_variables")) source.selected_variables = j.at("selected_variables").get<std::vector<std::string>>();
}

inline void to_json(nlohmann::json& j, const PlotSeriesConfig& series) {
  j = nlohmann::json{
    {"name", series.name}, {"expression", series.expression},
    {"source_alias", series.source_alias}, {"source_path", series.source_path},
    {"table_name", series.table_name}, {"time_column", series.time_column},
    {"value_column", series.value_column}, {"visible", series.visible},
    {"derived", series.derived}, {"color", series.color}
  };
}

inline void from_json(const nlohmann::json& j, PlotSeriesConfig& series) {
  series.name = j.at("name").get<std::string>();
  series.expression = j.value("expression", std::string{});
  if (j.contains("source_alias") && !j.at("source_alias").is_null()) series.source_alias = j.at("source_alias").get<std::string>();
  if (j.contains("source_path") && !j.at("source_path").is_null()) series.source_path = j.at("source_path").get<std::string>();
  if (j.contains("table_name") && !j.at("table_name").is_null()) series.table_name = j.at("table_name").get<std::string>();
  if (j.contains("time_column") && !j.at("time_column").is_null()) series.time_column = j.at("time_column").get<std::string>();
  if (j.contains("value_column") && !j.at("value_column").is_null()) series.value_column = j.at("value_column").get<std::string>();
  series.visible = j.value("visible", true);
  series.derived = j.value("derived", false);
  if (j.contains("color")) series.color = j.at("color").get<std::array<double, 4>>();
}

inline void to_json(nlohmann::json& j, const PlotTabConfig& view) {
  j = nlohmann::json{
    {"title", view.title}, {"series", view.series},
    {"autoscale_x", view.autoscale_x}, {"autoscale_y", view.autoscale_y},
    {"x_range", view.x_range}, {"y_range", view.y_range},
    {"active_series_index", view.active_series_index},
    {"expression_draft", view.expression_draft}
  };
}

inline void from_json(const nlohmann::json& j, PlotViewConfig& view) {
  view.title = j.at("title").get<std::string>();
  if (j.contains("series")) view.series = j.at("series").get<std::vector<PlotSeriesConfig>>();
  view.autoscale_x = j.value("autoscale_x", true);
  view.autoscale_y = j.value("autoscale_y", true);
  if (j.contains("x_range") && !j.at("x_range").is_null()) view.x_range = j.at("x_range").get<std::array<double, 2>>();
  if (j.contains("y_range") && !j.at("y_range").is_null()) view.y_range = j.at("y_range").get<std::array<double, 2>>();
  view.active_series_index = j.value("active_series_index", std::size_t{0});
  view.expression_draft = j.value("expression_draft", std::string{});
}

inline void to_json(nlohmann::json& j, const AnalysisWindowConfig& window) {
  j = nlohmann::json{{"title", window.title}, {"tabs", window.tabs}, {"active_tab", window.active_tab}};
}

inline void from_json(const nlohmann::json& j, AnalysisWindowConfig& window) {
  window.title = j.at("title").get<std::string>();
  if (j.contains("tabs")) window.tabs = j.at("tabs").get<std::vector<PlotTabConfig>>();
  window.active_tab = j.value("active_tab", std::size_t{0});
}

inline void to_json(nlohmann::json& j, const WorkspaceConfig& workspace) {
  j = nlohmann::json{{"windows", workspace.windows}, {"point_budget", workspace.point_budget}};
}

inline void from_json(const nlohmann::json& j, WorkspaceConfig& workspace) {
  if (j.contains("windows")) workspace.windows = j.at("windows").get<std::vector<AnalysisWindowConfig>>();
  workspace.point_budget = j.value("point_budget", std::size_t{50000});
}

inline void to_json(nlohmann::json& j, const ProjectState& project) {
  j = nlohmann::json{{"sources", project.sources}, {"workspace", project.workspace}};
}

inline void from_json(const nlohmann::json& j, ProjectState& project) {
  if (j.contains("sources")) project.sources = j.at("sources").get<std::vector<ProjectSource>>();
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
  if (!ec && !relative.empty() && relative.native().find("..") != 0) return relative;
  return target;
}

inline std::filesystem::path resolve_project_path(const std::filesystem::path& project_file, const std::string& stored_path) {
  const std::filesystem::path stored(stored_path);
  if (stored.is_absolute()) return stored;
  return project_file.parent_path() / stored;
}

inline void save_project(const std::filesystem::path& file, ProjectState project) {
  for (auto& source : project.sources) {
    source.path = project_relative_path(file, source.path).string();
  }
  const nlohmann::json json = project;
  std::ofstream output(file);
  if (!output.is_open()) throw std::runtime_error("Failed to open project file for writing: " + file.string());
  output << json.dump(2) << '\n';
}

inline ProjectState load_project(const std::filesystem::path& file) {
  std::ifstream input(file);
  if (!input.is_open()) throw std::runtime_error("Failed to open project file for reading: " + file.string());
  const auto json = nlohmann::json::parse(input);
  ProjectState project = json.get<ProjectState>();
  for (auto& source : project.sources) {
    source.path = resolve_project_path(file, source.path).string();
  }
  return project;
}

} // namespace tsv