#include "app_internal.hpp"

namespace tsv::app {

void save_project_file(AppState& app, const fs::path& path) {
  tsv::ProjectState project;
  std::map<std::string, tsv::ProjectSource> entries;
  for (const auto& source : app.sources) {
    tsv::ProjectSource entry;
    entry.kind = source.catalog.kind;
    entry.path = source.catalog.path.string();
    entry.alias = source.alias;
    entries.emplace(entry.path, std::move(entry));
  }
  for (const auto& window : app.workspace.windows) {
    for (const auto& tab : window.tabs) {
      for (const auto& series_cfg : tab.series) {
        if (series_cfg.derived || !series_cfg.source_path.has_value()) continue;
        auto& entry = entries[*series_cfg.source_path];
        entry.selected_variables.push_back(series_cfg.name);
        if (!entry.table_name.has_value() && series_cfg.table_name.has_value()) entry.table_name = series_cfg.table_name;
        if (!entry.time_column.has_value() && series_cfg.time_column.has_value()) entry.time_column = series_cfg.time_column;
      }
    }
  }
  for (auto& [_, entry] : entries) {
    std::sort(entry.selected_variables.begin(), entry.selected_variables.end());
    entry.selected_variables.erase(std::unique(entry.selected_variables.begin(), entry.selected_variables.end()), entry.selected_variables.end());
    project.sources.push_back(std::move(entry));
  }
  project.workspace = app.workspace;
  tsv::save_project(path, project);
  app.project_path = path;
  app.status = "Saved project";
}

void load_project_file(AppState& app, const fs::path& path) {
  try {
    const auto project = tsv::load_project(path);
    app.sources.clear();
    app.series_cache.clear();
    app.series_errors.clear();
    app.raw_series_cache.clear();
    app.workspace = project.workspace;
    detail::populate_legacy_bindings(app, project);
    for (const auto& source : project.sources) {
      const auto source_path = fs::path(source.path);
      open_source(app, source_path, source.alias.empty() ? std::optional<std::string>{} : std::optional<std::string>{source.alias}, source.kind);
    }
    ensure_workspace_defaults(app);
    app.project_path = path;
    app.status = "Loaded project";
    rebuild_cache(app);
  } catch (const std::exception& ex) { app.status = ex.what(); }
}

} // namespace tsv::app