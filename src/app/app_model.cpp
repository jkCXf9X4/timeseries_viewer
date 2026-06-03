#include "timeseries_viewer/app_model.hpp"

#include <algorithm>
#include <cstddef>
#include <cctype>
#include <map>
#include <set>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace tsv::app {
namespace {

std::optional<tsv::TableCatalog> find_table(const tsv::SourceCatalog& catalog, const std::optional<std::string>& table_name) {
  if (catalog.tables.empty()) {
    return std::nullopt;
  }
  if (!table_name.has_value()) {
    return catalog.tables.front();
  }
  for (const auto& table : catalog.tables) {
    if (table.name == *table_name) {
      return table;
    }
  }
  return std::nullopt;
}

std::string default_time_column(const tsv::TableCatalog& table) {
  if (const auto inferred = tsv::infer_time_column(table.columns); inferred.has_value()) {
    return *inferred;
  }
  return {};
}

std::string make_series_name(const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column) {
  if (table_name.has_value()) {
    return source.alias + "." + *table_name + "." + value_column;
  }
  return source.alias + "." + value_column;
}

const OpenSource* find_source_by_path(const AppState& app, const fs::path& path) {
  const auto it = std::find_if(app.sources.begin(), app.sources.end(), [&](const OpenSource& source) {
    return source.catalog.path == path;
  });
  return it == app.sources.end() ? nullptr : &*it;
}

const OpenSource* find_source_by_alias(const AppState& app, const std::string& alias) {
  const auto it = std::find_if(app.sources.begin(), app.sources.end(), [&](const OpenSource& source) {
    return source.alias == alias;
  });
  return it == app.sources.end() ? nullptr : &*it;
}

std::optional<tsv::SeriesData> load_raw_series(
  const OpenSource& source,
  const std::optional<std::string>& table_name,
  const std::optional<std::string>& time_column_override,
  const std::string& value_column,
  std::string& error,
  std::string& time_column_out,
  const std::optional<std::size_t> max_points
) {
  const auto table = find_table(source.catalog, table_name);
  if (!table.has_value()) {
    error = "Table not found";
    return std::nullopt;
  }

  tsv::SeriesRequest request;
  request.table_name = table_name;
  request.time_column = time_column_override.value_or(default_time_column(*table));
  request.value_column = value_column;
  request.max_points = max_points;
  time_column_out = request.time_column;

  tsv::LoadOutcome outcome;
  if (source.catalog.kind == tsv::SourceKind::Csv) {
    outcome = tsv::load_csv_series(source.catalog.path, request);
  } else {
    outcome = tsv::load_sqlite_series(source.catalog.path, table->name, request);
  }

  if (!outcome.ok) {
    error = outcome.error;
    return std::nullopt;
  }
  outcome.series.name = make_series_name(source, table_name, value_column);
  return outcome.series;
}

void populate_legacy_bindings(AppState& app, const tsv::ProjectState& project) {
  for (auto& window : app.workspace.windows) {
    for (auto& tab : window.tabs) {
      for (auto& series_cfg : tab.series) {
        if (series_cfg.derived) {
          continue;
        }
        if (series_cfg.source_path.has_value() && series_cfg.value_column.has_value()) {
          continue;
        }

        for (const auto& source : project.sources) {
          if (std::find(source.selected_variables.begin(), source.selected_variables.end(), series_cfg.name) == source.selected_variables.end()) {
            continue;
          }

          series_cfg.source_path = source.path;
          series_cfg.source_alias = source.alias.empty() ? sanitize_identifier(fs::path(source.path).stem().string()) : source.alias;
          series_cfg.table_name = source.table_name;
          series_cfg.time_column = source.time_column;
          const auto last_dot = series_cfg.name.find_last_of('.');
          series_cfg.value_column = last_dot == std::string::npos ? series_cfg.name : series_cfg.name.substr(last_dot + 1);
          break;
        }
      }
    }
  }
}

} // namespace

bool SeriesBindingKey::operator==(const SeriesBindingKey& other) const {
  return path == other.path
      && table_name == other.table_name
      && time_column == other.time_column
      && value_column == other.value_column;
}

std::size_t SeriesBindingKeyHash::operator()(const SeriesBindingKey& key) const noexcept {
  std::size_t seed = std::hash<std::string>{}(key.path.string());
  auto mix = [&](const auto& value) {
    using T = std::decay_t<decltype(value)>;
    const std::size_t h = std::hash<T>{}(value);
    seed ^= h + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
  };
  mix(key.table_name.has_value() ? *key.table_name : std::string{});
  mix(key.time_column.has_value() ? *key.time_column : std::string{});
  mix(key.value_column);
  return seed;
}

std::string sanitize_identifier(std::string value) {
  for (char& ch : value) {
    const auto u = static_cast<unsigned char>(ch);
    if (!std::isalnum(u) && ch != '_') {
      ch = '_';
    }
  }
  while (value.find("__") != std::string::npos) {
    value.erase(value.find("__"), 1);
  }
  if (value.empty()) {
    value = "source";
  }
  return value;
}

std::string unique_alias(const std::vector<OpenSource>& sources, const std::string& preferred) {
  const auto base = sanitize_identifier(preferred);
  std::string alias = base;
  int suffix = 2;
  const auto exists = [&](const std::string& candidate) {
    return std::any_of(sources.begin(), sources.end(), [&](const OpenSource& src) {
      return src.alias == candidate;
    });
  };
  while (exists(alias)) {
    alias = base + "_" + std::to_string(suffix++);
  }
  return alias;
}

void ensure_workspace_defaults(AppState& app) {
  if (app.workspace.windows.empty()) {
    AnalysisWindowConfig window;
    window.title = "Window 1";
    window.tabs.push_back(PlotTabConfig{});
    app.workspace.windows.push_back(std::move(window));
  }
  for (std::size_t index = 0; index < app.workspace.windows.size(); ++index) {
    auto& window = app.workspace.windows[index];
    if (window.title.empty()) {
      window.title = "Window " + std::to_string(index + 1);
    }
    if (window.tabs.empty()) {
      window.tabs.push_back(PlotTabConfig{});
    }
    if (window.active_tab >= window.tabs.size()) {
      window.active_tab = 0;
    }
    for (std::size_t tab_index = 0; tab_index < window.tabs.size(); ++tab_index) {
      auto& tab = window.tabs[tab_index];
      if (tab.title.empty()) {
        tab.title = "Plot " + std::to_string(tab_index + 1);
      }
      if (tab.active_series_index >= tab.series.size() && !tab.series.empty()) {
        tab.active_series_index = 0;
      }
    }
  }
  if (app.active_window < 0 || app.active_window >= static_cast<int>(app.workspace.windows.size())) {
    app.active_window = 0;
  }
}

AnalysisWindowConfig& active_window(AppState& app) {
  ensure_workspace_defaults(app);
  return app.workspace.windows.at(static_cast<std::size_t>(app.active_window));
}

PlotTabConfig& active_tab(AppState& app) {
  auto& window = active_window(app);
  return window.tabs.at(window.active_tab);
}

const PlotTabConfig& active_tab(const AppState& app) {
  return app.workspace.windows.at(static_cast<std::size_t>(app.active_window)).tabs.at(app.workspace.windows.at(static_cast<std::size_t>(app.active_window)).active_tab);
}

void add_window(AppState& app, std::string title) {
  AnalysisWindowConfig window;
  window.title = title.empty() ? "Window " + std::to_string(app.workspace.windows.size() + 1) : std::move(title);
  window.tabs.push_back(PlotTabConfig{});
  app.workspace.windows.push_back(std::move(window));
  app.active_window = static_cast<int>(app.workspace.windows.size()) - 1;
}

void add_tab(AppState& app, std::size_t window_index, std::string title) {
  ensure_workspace_defaults(app);
  auto& window = app.workspace.windows.at(window_index);
  PlotTabConfig tab;
  tab.title = title.empty() ? "Plot " + std::to_string(window.tabs.size() + 1) : std::move(title);
  window.tabs.push_back(std::move(tab));
  window.active_tab = window.tabs.size() - 1;
  app.active_window = static_cast<int>(window_index);
}

void select_series(AppState& app, std::size_t window_index, std::size_t tab_index, std::size_t series_index) {
  ensure_workspace_defaults(app);
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  if (series_index < tab.series.size()) {
    tab.active_series_index = series_index;
    app.active_window = static_cast<int>(window_index);
    app.workspace.windows.at(window_index).active_tab = tab_index;
  }
}

void remove_series(AppState& app, std::size_t window_index, std::size_t tab_index, std::size_t series_index) {
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  if (series_index >= tab.series.size()) {
    return;
  }
  tab.series.erase(tab.series.begin() + static_cast<std::ptrdiff_t>(series_index));
  if (tab.active_series_index >= tab.series.size() && !tab.series.empty()) {
    tab.active_series_index = tab.series.size() - 1;
  }
}

void toggle_series_visibility(AppState& app, std::size_t window_index, std::size_t tab_index, std::size_t series_index, bool visible) {
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  if (series_index < tab.series.size()) {
    tab.series[series_index].visible = visible;
  }
}

void rename_series(AppState& app, std::size_t window_index, std::size_t tab_index, std::size_t series_index, const std::string& name) {
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  if (series_index < tab.series.size()) {
    tab.series[series_index].name = name;
  }
}

void open_source(AppState& app, const fs::path& path, const std::optional<std::string>& alias_override, const std::optional<tsv::SourceKind>& kind_override) {
  try {
    OpenSource source;
    const auto kind = kind_override.has_value()
      ? *kind_override
      : (path.extension() == ".csv" ? tsv::SourceKind::Csv : tsv::SourceKind::Sqlite);
    source.catalog = kind == tsv::SourceKind::Csv ? tsv::load_csv_catalog(path) : tsv::load_sqlite_catalog(path);
    source.alias = alias_override.has_value() ? *alias_override : unique_alias(app.sources, path.stem().string());
    source.catalog.source_name = source.alias;
    std::error_code ec;
    source.last_write_time = fs::last_write_time(path, ec);
    app.sources.push_back(std::move(source));
    app.status = "Opened " + path.filename().string();
  } catch (const std::exception& ex) {
    app.status = ex.what();
  }
}

void rebuild_cache(AppState& app) {
  ensure_workspace_defaults(app);
  app.series_cache.clear();
  app.series_errors.clear();

  tsv::SeriesRegistry registry;

  for (auto& window : app.workspace.windows) {
    for (auto& tab : window.tabs) {
      for (auto& series_cfg : tab.series) {
        if (series_cfg.derived) {
          continue;
        }

        const auto source_path = series_cfg.source_path.has_value() ? fs::path(*series_cfg.source_path) : fs::path{};
        const OpenSource* source = nullptr;
        if (!source_path.empty()) {
          source = find_source_by_path(app, source_path);
        }
        if (source == nullptr && series_cfg.source_alias.has_value()) {
          source = find_source_by_alias(app, *series_cfg.source_alias);
        }
        if (source == nullptr) {
          app.series_errors[series_cfg.name] = "Source missing";
          continue;
        }
        if (!series_cfg.value_column.has_value()) {
          app.series_errors[series_cfg.name] = "Value column missing";
          continue;
        }

        const auto table = find_table(source->catalog, series_cfg.table_name);
        if (!table.has_value()) {
          app.series_errors[series_cfg.name] = "Table not found";
          continue;
        }

        const auto resolved_time_column = series_cfg.time_column.has_value()
          ? *series_cfg.time_column
          : default_time_column(*table);

        SeriesBindingKey key{source->catalog.path, series_cfg.table_name, resolved_time_column, *series_cfg.value_column};
        auto& entry = app.raw_series_cache[key];
        const auto current_stamp = source->last_write_time;
        const auto budget = app.workspace.point_budget == 0 ? std::optional<std::size_t>{} : std::optional<std::size_t>{app.workspace.point_budget};
        const bool cache_valid = entry.source_stamp.has_value()
          && current_stamp.has_value()
          && *entry.source_stamp == *current_stamp
          && entry.max_points == budget;

        if (!cache_valid) {
          std::string error;
          std::string time_column;
          const auto series = load_raw_series(*source, series_cfg.table_name, resolved_time_column, *series_cfg.value_column, error, time_column, budget);
          if (!series.has_value()) {
            app.series_errors[series_cfg.name] = error;
            continue;
          }
          entry.source_stamp = current_stamp;
          entry.max_points = budget;
          entry.series = *series;
        }

        if (series_cfg.name.empty()) {
          series_cfg.name = entry.series.name;
        }
        if (!series_cfg.source_alias.has_value()) {
          series_cfg.source_alias = source->alias;
        }
        if (!series_cfg.source_path.has_value()) {
          series_cfg.source_path = source->catalog.path.string();
        }
        series_cfg.time_column = entry.series.time_column;

        auto loaded = entry.series;
        loaded.name = series_cfg.name;
        loaded.source_name = source->alias;
        app.series_cache[loaded.name] = loaded;
        registry.set(loaded);
      }
    }
  }

  for (auto& window : app.workspace.windows) {
    for (auto& tab : window.tabs) {
      for (auto& series_cfg : tab.series) {
        if (!series_cfg.derived) {
          continue;
        }
        if (series_cfg.name.empty()) {
          series_cfg.name = series_cfg.expression.empty() ? "derived" : series_cfg.expression;
        }
        try {
          auto series = tsv::evaluate_expression(series_cfg.expression, registry);
          series.name = series_cfg.name;
          app.series_cache[series.name] = std::move(series);
          registry.set(app.series_cache.at(series_cfg.name));
        } catch (const std::exception& ex) {
          app.series_errors[series_cfg.name] = ex.what();
        }
      }
    }
  }
}

void add_raw_series(AppState& app, const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column) {
  tsv::PlotSeriesConfig series_cfg;
  series_cfg.source_alias = source.alias;
  series_cfg.source_path = source.catalog.path.string();
  series_cfg.table_name = table_name;
  series_cfg.value_column = value_column;

  std::string error;
  std::string time_column;
  const auto budget = app.workspace.point_budget == 0 ? std::optional<std::size_t>{} : std::optional<std::size_t>{app.workspace.point_budget};
  const auto preview = load_raw_series(source, table_name, std::nullopt, value_column, error, time_column, budget);
  if (!preview.has_value()) {
    app.status = error;
    return;
  }

  series_cfg.name = preview->name;
  series_cfg.time_column = time_column;

  auto& tab = active_tab(app);
  tab.series.push_back(std::move(series_cfg));
  tab.active_series_index = tab.series.size() - 1;
  app.status = "Added " + preview->name;
  rebuild_cache(app);
}

void bind_series_to_source(
  AppState& app,
  std::size_t window_index,
  std::size_t tab_index,
  std::size_t series_index,
  const OpenSource& source,
  const std::optional<std::string>& table_name,
  const std::string& value_column,
  const std::optional<std::string>& time_column_override
) {
  auto& series = app.workspace.windows.at(window_index).tabs.at(tab_index).series.at(series_index);
  series.derived = false;
  series.expression.clear();
  series.source_alias = source.alias;
  series.source_path = source.catalog.path.string();
  series.table_name = table_name;
  series.value_column = value_column;
  series.time_column = time_column_override;
  series.name = make_series_name(source, table_name, value_column);
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  tab.active_series_index = series_index;
  rebuild_cache(app);
}

void add_derived_series(AppState& app) {
  const std::string expression = active_tab(app).expression_draft;
  if (expression.empty()) {
    app.status = "Expression is empty";
    return;
  }

  tsv::SeriesRegistry registry;
  for (const auto& [name, series] : app.series_cache) {
    registry.set(series);
  }

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
  } catch (const std::exception& ex) {
    app.status = ex.what();
  }
}

void add_derived_series_to_tab(AppState& app, std::size_t window_index, std::size_t tab_index) {
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  const std::string expression = tab.expression_draft;
  if (expression.empty()) {
    app.status = "Expression is empty";
    return;
  }

  tsv::SeriesRegistry registry;
  for (const auto& [name, series] : app.series_cache) {
    registry.set(series);
  }

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
  } catch (const std::exception& ex) {
    app.status = ex.what();
  }
}

std::vector<BindableParameter> list_bindable_parameters(const OpenSource& source) {
  std::vector<BindableParameter> parameters;
  for (const auto& table : source.catalog.tables) {
    const auto time_column = tsv::infer_time_column(table.columns);
    for (const auto& column : table.columns) {
      if (time_column.has_value() && column.name == *time_column) {
        continue;
      }

      BindableParameter parameter;
      parameter.source_alias = source.alias;
      parameter.source_path = source.catalog.path.string();
      parameter.value_column = column.name;
      if (source.catalog.kind == tsv::SourceKind::Sqlite) {
        parameter.table_name = table.name;
        parameter.display_name = source.alias + "." + table.name + "." + column.name;
      } else {
        parameter.display_name = source.alias + "." + column.name;
      }
      parameters.push_back(std::move(parameter));
    }
  }

  std::sort(parameters.begin(), parameters.end(), [](const BindableParameter& lhs, const BindableParameter& rhs) {
    return lhs.display_name < rhs.display_name;
  });
  return parameters;
}

tsv::TreeNode build_bindable_parameter_tree(const OpenSource& source) {
  std::vector<std::string> names;
  names.reserve(source.catalog.tables.size() * 4);
  for (const auto& parameter : list_bindable_parameters(source)) {
    names.push_back(parameter.display_name);
  }
  return tsv::build_variable_tree(names);
}

std::optional<BindableParameter> find_bindable_parameter(const OpenSource& source, const std::string& display_name) {
  const auto parameters = list_bindable_parameters(source);
  const auto it = std::find_if(parameters.begin(), parameters.end(), [&](const BindableParameter& parameter) {
    return parameter.display_name == display_name;
  });
  if (it == parameters.end()) {
    return std::nullopt;
  }
  return *it;
}

std::string plot_legend_label(const tsv::PlotSeriesConfig& series) {
  if (series.derived) {
    if (!series.expression.empty()) {
      return series.name + " := " + series.expression;
    }
    return series.name + " (derived)";
  }

  std::string label = series.name;
  if (series.source_alias.has_value() && !series.source_alias->empty()) {
    label += " [" + *series.source_alias + "]";
  }
  if (series.source_path.has_value() && !series.source_path->empty()) {
    label += " (" + fs::path(*series.source_path).filename().string() + ")";
  }
  return label;
}

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
      for (const auto& series : tab.series) {
        if (series.derived || !series.source_path.has_value()) {
          continue;
        }
        auto& entry = entries[*series.source_path];
        entry.selected_variables.push_back(series.name);
        if (!entry.table_name.has_value() && series.table_name.has_value()) {
          entry.table_name = series.table_name;
        }
        if (!entry.time_column.has_value() && series.time_column.has_value()) {
          entry.time_column = series.time_column;
        }
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
    populate_legacy_bindings(app, project);

    for (const auto& source : project.sources) {
      const auto source_path = fs::path(source.path);
      open_source(app, source_path, source.alias.empty() ? std::optional<std::string>{} : std::optional<std::string>{source.alias}, source.kind);
    }
    ensure_workspace_defaults(app);
    app.project_path = path;
    app.status = "Loaded project";
    rebuild_cache(app);
  } catch (const std::exception& ex) {
    app.status = ex.what();
  }
}

void poll_live_reload(AppState& app) {
  if (!app.live_mode) {
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  if (now - app.last_poll < std::chrono::seconds(1)) {
    return;
  }
  app.last_poll = now;

  bool changed = false;
  for (auto& source : app.sources) {
    std::error_code ec;
    const auto current = fs::last_write_time(source.catalog.path, ec);
    if (!ec && current != source.last_write_time) {
      source.last_write_time = current;
      changed = true;
    }
  }
  if (changed) {
    rebuild_cache(app);
    app.status = "Live data refreshed";
  }
}

} // namespace tsv::app
