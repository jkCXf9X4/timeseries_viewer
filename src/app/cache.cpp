#include "app_internal.hpp"

namespace tsv::app {

void rebuild_cache(AppState& app) {
  ensure_workspace_defaults(app);
  app.series_cache.clear();
  app.series_errors.clear();
  tsv::SeriesRegistry registry;
  // Load non-derived series
  for (auto& window : app.workspace.windows) {
    for (auto& tab : window.tabs) {
      for (auto& series_cfg : tab.series) {
        if (series_cfg.derived) continue;
        const auto source_path = series_cfg.source_path.has_value() ? fs::path(*series_cfg.source_path) : fs::path{};
        const OpenSource* source = nullptr;
        if (!source_path.empty()) source = detail::find_source_by_path(app, source_path);
        if (source == nullptr && series_cfg.source_alias.has_value()) source = detail::find_source_by_alias(app, *series_cfg.source_alias);
        if (source == nullptr) { app.series_errors[series_cfg.name] = "Source missing"; continue; }
        if (!series_cfg.value_column.has_value()) { app.series_errors[series_cfg.name] = "Value column missing"; continue; }
        const auto table = detail::find_table(source->catalog, series_cfg.table_name);
        if (!table.has_value()) { app.series_errors[series_cfg.name] = "Table not found"; continue; }
        const auto resolved_time_column = series_cfg.time_column.has_value() ? *series_cfg.time_column : detail::default_time_column(*table);
        SeriesBindingKey key{source->catalog.path, series_cfg.table_name, resolved_time_column, *series_cfg.value_column};
        auto& entry = app.raw_series_cache[key];
        const auto current_stamp = source->last_write_time;
        const auto budget = app.workspace.point_budget == 0 ? std::optional<std::size_t>{} : std::optional<std::size_t>{app.workspace.point_budget};
        const bool cache_valid = entry.source_stamp.has_value() && current_stamp.has_value() && *entry.source_stamp == *current_stamp && entry.max_points == budget;
        if (!cache_valid) {
          std::string error, time_column;
          const auto series = detail::load_raw_series(*source, series_cfg.table_name, resolved_time_column, *series_cfg.value_column, error, time_column, budget);
          if (!series.has_value()) { app.series_errors[series_cfg.name] = error; continue; }
          entry.source_stamp = current_stamp;
          entry.max_points = budget;
          entry.series = *series;
        }
        if (series_cfg.name.empty()) series_cfg.name = entry.series.name;
        if (!series_cfg.source_alias.has_value()) series_cfg.source_alias = source->alias;
        if (!series_cfg.source_path.has_value()) series_cfg.source_path = source->catalog.path.string();
        series_cfg.time_column = entry.series.time_column;
        auto loaded = entry.series;
        loaded.name = series_cfg.name;
        loaded.source_name = source->alias;
        app.series_cache[loaded.name] = loaded;
        registry.set(loaded);
      }
    }
  }
  // Evaluate derived series
  for (auto& window : app.workspace.windows) {
    for (auto& tab : window.tabs) {
      for (auto& series_cfg : tab.series) {
        if (!series_cfg.derived) continue;
        if (series_cfg.name.empty()) series_cfg.name = series_cfg.expression.empty() ? "derived" : series_cfg.expression;
        try {
          auto series = tsv::evaluate_expression(series_cfg.expression, registry);
          series.name = series_cfg.name;
          app.series_cache[series.name] = std::move(series);
          registry.set(app.series_cache.at(series_cfg.name));
        } catch (const std::exception& ex) { app.series_errors[series_cfg.name] = ex.what(); }
      }
    }
  }
}

void rebuild_cache_metadata(AppState& app) {
  ensure_workspace_defaults(app);
  app.series_cache.clear();
  app.series_errors.clear();
  app.cache_stale = false;
}

void ensure_tab_data(AppState& app, std::size_t window_index, std::size_t tab_index) {
  if (window_index >= app.workspace.windows.size()) return;
  auto& window = app.workspace.windows[window_index];
  if (tab_index >= window.tabs.size()) return;
  auto& tab = window.tabs[tab_index];
  for (auto& series_cfg : tab.series) {
    if (series_cfg.derived) continue;
    if (app.series_cache.contains(series_cfg.name) && !app.series_errors.contains(series_cfg.name)) continue;
    const auto source_path = series_cfg.source_path.has_value() ? fs::path(*series_cfg.source_path) : fs::path{};
    const OpenSource* source = nullptr;
    if (!source_path.empty()) source = detail::find_source_by_path(app, source_path);
    if (source == nullptr && series_cfg.source_alias.has_value()) source = detail::find_source_by_alias(app, *series_cfg.source_alias);
    if (source == nullptr) { app.series_errors[series_cfg.name] = "Source missing"; continue; }
    if (!series_cfg.value_column.has_value()) { app.series_errors[series_cfg.name] = "Value column missing"; continue; }
    const auto table = detail::find_table(source->catalog, series_cfg.table_name);
    if (!table.has_value()) { app.series_errors[series_cfg.name] = "Table not found"; continue; }
    const auto resolved_time_column = series_cfg.time_column.has_value() ? *series_cfg.time_column : detail::default_time_column(*table);
    SeriesBindingKey key{source->catalog.path, series_cfg.table_name, resolved_time_column, *series_cfg.value_column};
    auto& entry = app.raw_series_cache[key];
    const auto current_stamp = source->last_write_time;
    const auto budget = app.workspace.point_budget == 0 ? std::optional<std::size_t>{} : std::optional<std::size_t>{app.workspace.point_budget};
    const bool cache_valid = entry.source_stamp.has_value() && current_stamp.has_value() && *entry.source_stamp == *current_stamp && entry.max_points == budget;
    if (!cache_valid) {
      std::string error, time_column;
      const auto series = detail::load_raw_series(*source, series_cfg.table_name, resolved_time_column, *series_cfg.value_column, error, time_column, budget);
      if (!series.has_value()) { app.series_errors[series_cfg.name] = error; continue; }
      entry.source_stamp = current_stamp;
      entry.max_points = budget;
      entry.series = *series;
    }
    if (series_cfg.name.empty()) series_cfg.name = entry.series.name;
    if (!series_cfg.source_alias.has_value()) series_cfg.source_alias = source->alias;
    if (!series_cfg.source_path.has_value()) series_cfg.source_path = source->catalog.path.string();
    series_cfg.time_column = entry.series.time_column;
    auto loaded = entry.series;
    loaded.name = series_cfg.name;
    loaded.source_name = source->alias;
    app.series_cache[loaded.name] = loaded;
  }
  // Re-evaluate derived series in this tab
  if (std::any_of(tab.series.begin(), tab.series.end(), [](const auto& s) { return s.derived; })) {
    tsv::SeriesRegistry registry;
    for (const auto& [name, series] : app.series_cache) registry.set(series);
    for (auto& series_cfg : tab.series) {
      if (!series_cfg.derived || (!app.series_errors.contains(series_cfg.name) && app.series_cache.contains(series_cfg.name))) continue;
      if (series_cfg.name.empty()) series_cfg.name = series_cfg.expression.empty() ? "derived" : series_cfg.expression;
      try {
        auto series = tsv::evaluate_expression(series_cfg.expression, registry);
        series.name = series_cfg.name;
        app.series_cache[series.name] = std::move(series);
        registry.set(app.series_cache.at(series_cfg.name));
      } catch (const std::exception& ex) { app.series_errors[series_cfg.name] = ex.what(); }
    }
  }
}

void add_raw_series(AppState& app, const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column) {
  const auto table = detail::find_table(source.catalog, table_name);
  if (!table.has_value()) { app.status = "Table not found"; return; }
  tsv::PlotSeriesConfig series_cfg;
  series_cfg.source_alias = source.alias;
  series_cfg.source_path = source.catalog.path.string();
  series_cfg.table_name = table_name;
  series_cfg.value_column = value_column;
  series_cfg.name = detail::make_series_name(source, table_name, value_column);
  if (const auto inferred_time = detail::default_time_column(*table); !inferred_time.empty()) series_cfg.time_column = inferred_time;
  auto& tab = active_tab(app);
  tab.series.push_back(std::move(series_cfg));
  tab.active_series_index = tab.series.size() - 1;
  app.status = "Added " + tab.series.back().name;
  rebuild_cache(app);
}

void bind_series_to_source(AppState& app, std::size_t window_index, std::size_t tab_index, std::size_t series_index, const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column, const std::optional<std::string>& time_column_override) {
  auto& series = app.workspace.windows.at(window_index).tabs.at(tab_index).series.at(series_index);
  series.derived = false;
  series.expression.clear();
  series.source_alias = source.alias;
  series.source_path = source.catalog.path.string();
  series.table_name = table_name;
  series.value_column = value_column;
  series.time_column = time_column_override;
  series.name = detail::make_series_name(source, table_name, value_column);
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  tab.active_series_index = series_index;
  rebuild_cache(app);
}

void add_derived_series(AppState& app) {
  const std::string expression = active_tab(app).expression_draft;
  if (expression.empty()) { app.status = "Expression is empty"; return; }
  tsv::SeriesRegistry registry;
  for (const auto& [name, series] : app.series_cache) registry.set(series);
  try {
    auto series = tsv::evaluate_expression(expression, registry);
    auto& tab = active_tab(app);
    tsv::PlotSeriesConfig series_cfg;
    series_cfg.name = series.name;
    series_cfg.expression = expression;
    series_cfg.derived = true;
    tab.series.push_back(std::move(series_cfg));
    tab.active_series_index = tab.series.size() - 1;
    app.series_cache[series.name] = std::move(series);
    app.status = "Added derived series";
    rebuild_cache(app);
  } catch (const std::exception& ex) { app.status = ex.what(); }
}

void add_derived_series_to_tab(AppState& app, std::size_t window_index, std::size_t tab_index) {
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  const std::string expression = tab.expression_draft;
  if (expression.empty()) { app.status = "Expression is empty"; return; }
  tsv::SeriesRegistry registry;
  for (const auto& [name, series] : app.series_cache) registry.set(series);
  try {
    auto series = tsv::evaluate_expression(expression, registry);
    tsv::PlotSeriesConfig series_cfg;
    series_cfg.name = series.name;
    series_cfg.expression = expression;
    series_cfg.derived = true;
    tab.series.push_back(std::move(series_cfg));
    tab.active_series_index = tab.series.size() - 1;
    app.series_cache[series.name] = std::move(series);
    app.status = "Added derived series";
    rebuild_cache(app);
  } catch (const std::exception& ex) { app.status = ex.what(); }
}

} // namespace tsv::app