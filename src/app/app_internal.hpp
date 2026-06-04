#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <system_error>
#include <utility>

#include "timeseries_viewer/app_model.hpp"
#include "io/csv/csv.hpp"
#include "io/sqlite/sqlite.hpp"

namespace tsv::app::detail {

inline std::optional<tsv::TableCatalog> find_table(const tsv::SourceCatalog& catalog, const std::optional<std::string>& table_name) {
  if (catalog.tables.empty()) return std::nullopt;
  if (!table_name.has_value()) return catalog.tables.front();
  for (const auto& table : catalog.tables) {
    if (table.name == *table_name) return table;
  }
  return std::nullopt;
}

inline std::string default_time_column(const tsv::TableCatalog& table) {
  if (const auto inferred = tsv::infer_time_column(table.columns); inferred.has_value()) {
    return *inferred;
  }
  return {};
}

inline std::string make_series_name(const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column) {
  if (table_name.has_value()) {
    return source.alias + "." + *table_name + "." + value_column;
  }
  return source.alias + "." + value_column;
}

inline std::optional<fs::file_time_type> source_stamp(const fs::path& path, tsv::SourceKind kind) {
  std::optional<fs::file_time_type> stamp;
  const auto consider = [&](const fs::path& candidate) {
    std::error_code ec;
    const auto current = fs::last_write_time(candidate, ec);
    if (ec) return;
    if (!stamp.has_value() || current > *stamp) stamp = current;
  };
  consider(path);
  if (kind == tsv::SourceKind::Sqlite) {
    consider(path.string() + "-wal");
    consider(path.string() + "-shm");
    consider(path.string() + "-journal");
  }
  return stamp;
}

template <typename Rows>
inline std::optional<tsv::SeriesData> load_series_from_rows(
  const OpenSource& source,
  const tsv::TableCatalog& table,
  const Rows& rows,
  const std::optional<std::string>& table_name,
  const std::optional<std::string>& time_column_override,
  const std::string& value_column,
  std::string& error,
  std::string& time_column_out,
  const std::optional<std::size_t> max_points
) {
  const auto resolved_time_column = time_column_override.value_or(default_time_column(table));
  const auto time_index = tsv::column_index_by_name(table.columns, resolved_time_column);
  const auto value_index = tsv::column_index_by_name(table.columns, value_column);
  if (!time_index.has_value()) {
    error = "Time column not found: " + resolved_time_column;
    return std::nullopt;
  }
  if (!value_index.has_value()) {
    error = "Value column not found: " + value_column;
    return std::nullopt;
  }
  tsv::SeriesData series;
  series.source_name = source.alias;
  series.table_name = table.name;
  series.time_column = table.columns[*time_index].name;
  series.value_column = table.columns[*value_index].name;
  series.name = make_series_name(source, table_name, value_column);
  time_column_out = series.time_column;
  for (const auto& row : rows) {
    if (*time_index >= row.size() || *value_index >= row.size()) continue;
    const auto time_value = tsv::parse_scalar(row[*time_index]);
    double numeric_value = 0.0;
    if (!time_value.has_value() || !tsv::parse_double(row[*value_index], numeric_value)) continue;
    series.time.push_back(*time_value);
    series.value.push_back(numeric_value);
  }
  if (max_points.has_value()) {
    tsv::downsample_series(series, *max_points);
  }
  return series;
}

inline const OpenSource* find_source_by_path(const AppState& app, const fs::path& path) {
  const auto it = std::find_if(app.sources.begin(), app.sources.end(), [&](const OpenSource& s) { return s.catalog.path == path; });
  return it == app.sources.end() ? nullptr : &*it;
}

inline const OpenSource* find_source_by_alias(const AppState& app, const std::string& alias) {
  const auto it = std::find_if(app.sources.begin(), app.sources.end(), [&](const OpenSource& s) { return s.alias == alias; });
  return it == app.sources.end() ? nullptr : &*it;
}

inline std::optional<tsv::SeriesData> load_raw_series(
  const OpenSource& source,
  const std::optional<std::string>& table_name,
  const std::optional<std::string>& time_column_override,
  const std::string& value_column,
  std::string& error,
  std::string& time_column_out,
  const std::optional<std::size_t> max_points
) {
  const auto table = find_table(source.catalog, table_name);
  if (!table.has_value()) { error = "Table not found"; return std::nullopt; }
  try {
    if (source.catalog.kind == tsv::SourceKind::Csv) {
      tsv::SeriesRequest request;
      request.time_column = time_column_override.value_or(default_time_column(*table));
      request.value_column = value_column;
      request.max_points = max_points;
      auto outcome = tsv::load_csv_series_streaming(source.catalog.path, table->columns, request);
      if (!outcome.ok) { error = outcome.error; return std::nullopt; }
      time_column_out = outcome.series.time_column;
      return outcome.series;
    }
    tsv::SeriesRequest request;
    request.time_column = time_column_override.value_or(default_time_column(*table));
    request.value_column = value_column;
    request.max_points = max_points;
    auto outcome = tsv::load_sqlite_series_targeted(source.catalog.path, table->name, table->columns, request);
    if (!outcome.ok) { error = outcome.error; return std::nullopt; }
    time_column_out = outcome.series.time_column;
    return outcome.series;
  } catch (const std::exception& ex) { error = ex.what(); return std::nullopt; }
}

inline void rebuild_bindable_parameter_cache(const OpenSource& source) {
  source.bindable_parameter_cache.clear();
  source.bindable_parameter_lookup.clear();
  source.bindable_parameter_tree_cache = tsv::TreeNode{"", "", false, {}};
  for (const auto& table : source.catalog.tables) {
    const auto time_column = tsv::infer_time_column(table.columns);
    for (const auto& column : table.columns) {
      if (time_column.has_value() && column.name == *time_column) continue;
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
      source.bindable_parameter_cache.push_back(std::move(parameter));
    }
  }
  std::sort(source.bindable_parameter_cache.begin(), source.bindable_parameter_cache.end(), [](const BindableParameter& a, const BindableParameter& b) { return a.display_name < b.display_name; });
  source.bindable_parameter_lookup.reserve(source.bindable_parameter_cache.size());
  std::vector<std::string> sorted_names;
  sorted_names.reserve(source.bindable_parameter_cache.size());
  for (std::size_t index = 0; index < source.bindable_parameter_cache.size(); ++index) {
    const auto& parameter = source.bindable_parameter_cache[index];
    source.bindable_parameter_lookup.emplace(parameter.display_name, index);
    sorted_names.push_back(parameter.display_name);
  }
  source.bindable_parameter_tree_cache = tsv::build_variable_tree(sorted_names);
  source.bindable_parameter_cache_ready = true;
}

inline void populate_legacy_bindings(AppState& app, const tsv::ProjectState& project) {
  for (auto& window : app.workspace.windows) {
    for (auto& tab : window.tabs) {
      for (auto& series_cfg : tab.series) {
        if (series_cfg.derived) continue;
        if (series_cfg.source_path.has_value() && series_cfg.value_column.has_value()) continue;
        for (const auto& source : project.sources) {
          if (std::find(source.selected_variables.begin(), source.selected_variables.end(), series_cfg.name) == source.selected_variables.end()) continue;
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

inline bool series_matches_binding(const tsv::PlotSeriesConfig& series, const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column) {
  if (series.derived) return false;
  if (series.source_path.has_value()) {
    if (*series.source_path != source.catalog.path.string()) return false;
  } else if (series.source_alias.has_value()) {
    if (*series.source_alias != source.alias) return false;
  } else return false;
  return series.table_name == table_name && series.value_column.has_value() && *series.value_column == value_column;
}

inline bool series_matches_source(const tsv::PlotSeriesConfig& series, const OpenSource& source) {
  if (series.derived) return false;
  if (series.source_path.has_value()) return *series.source_path == source.catalog.path.string();
  if (series.source_alias.has_value()) return *series.source_alias == source.alias;
  return false;
}

inline std::set<std::string> collect_derived_series_names(const AppState& app) {
  std::set<std::string> names;
  for (const auto& window : app.workspace.windows) {
    for (const auto& tab : window.tabs) {
      for (const auto& series_cfg : tab.series) {
        if (series_cfg.derived && !series_cfg.name.empty()) names.insert(series_cfg.name);
      }
    }
  }
  return names;
}

inline void erase_series_cache_entries(AppState& app, const std::set<std::string>& names) {
  for (const auto& name : names) {
    app.series_cache.erase(name);
    app.series_errors.erase(name);
  }
}

inline void populate_registry_from_raw_series(AppState& app, tsv::SeriesRegistry& registry) {
  for (const auto& window : app.workspace.windows) {
    for (const auto& tab : window.tabs) {
      for (const auto& series_cfg : tab.series) {
        if (series_cfg.derived || series_cfg.name.empty()) continue;
        const auto it = app.series_cache.find(series_cfg.name);
        if (it != app.series_cache.end()) registry.set(it->second);
      }
    }
  }
}

inline tsv::SeriesRegistry make_registry_from_raw_cache(const AppState& app) {
  tsv::SeriesRegistry registry;
  for (const auto& [name, series] : app.series_cache) registry.set(series);
  return registry;
}

inline void refresh_live_source_cache(AppState& app, std::size_t source_index, tsv::SeriesRegistry& registry) {
  auto& source = app.sources.at(source_index);
  const auto budget = app.workspace.point_budget == 0 ? std::optional<std::size_t>{} : std::optional<std::size_t>{app.workspace.point_budget};
  for (auto& window : app.workspace.windows) {
    for (auto& tab : window.tabs) {
      for (auto& series_cfg : tab.series) {
        if (!series_matches_source(series_cfg, source)) continue;
        const auto table = find_table(source.catalog, series_cfg.table_name);
        const std::string series_name = series_cfg.name.empty() ? make_series_name(source, series_cfg.table_name, series_cfg.value_column.value_or("")) : series_cfg.name;
        if (!table.has_value()) {
          app.series_errors[series_name] = "Table not found";
          app.series_cache.erase(series_name);
          continue;
        }
        if (!series_cfg.value_column.has_value()) {
          app.series_errors[series_name] = "Value column missing";
          app.series_cache.erase(series_name);
          continue;
        }
        if (series_cfg.name.empty()) series_cfg.name = series_name;
        const auto resolved_time_column = series_cfg.time_column.has_value() ? *series_cfg.time_column : default_time_column(*table);
        SeriesBindingKey key{source.catalog.path, series_cfg.table_name, resolved_time_column, *series_cfg.value_column};
        auto& entry = app.raw_series_cache[key];
        const auto current_stamp = source.last_write_time;
        const bool cache_valid = entry.source_stamp.has_value() && current_stamp.has_value() && *entry.source_stamp == *current_stamp && entry.max_points == budget;
        if (!cache_valid) {
          std::string error, time_column;
          const auto series = load_raw_series(source, series_cfg.table_name, resolved_time_column, *series_cfg.value_column, error, time_column, budget);
          if (!series.has_value()) {
            app.series_errors[series_name] = error;
            app.series_cache.erase(series_name);
            continue;
          }
          entry.source_stamp = current_stamp;
          entry.max_points = budget;
          entry.series = *series;
        }
        entry.series.name = series_name;
        entry.series.source_name = source.alias;
        series_cfg.time_column = entry.series.time_column;
        if (!series_cfg.source_alias.has_value()) series_cfg.source_alias = source.alias;
        if (!series_cfg.source_path.has_value()) series_cfg.source_path = source.catalog.path.string();
        app.series_cache[series_name] = entry.series;
        registry.set(app.series_cache.at(series_name));
        app.series_errors.erase(series_name);
      }
    }
  }
}

inline void refresh_live_cache(AppState& app, const std::vector<std::size_t>& changed_source_indices) {
  ensure_workspace_defaults(app);
  const auto derived_names = collect_derived_series_names(app);
  erase_series_cache_entries(app, derived_names);
  tsv::SeriesRegistry registry;
  for (const auto source_index : changed_source_indices) {
    if (source_index < app.sources.size()) refresh_live_source_cache(app, source_index, registry);
  }
  populate_registry_from_raw_series(app, registry);
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
          app.series_errors.erase(series_cfg.name);
        } catch (const std::exception& ex) {
          app.series_errors[series_cfg.name] = ex.what();
          app.series_cache.erase(series_cfg.name);
        }
      }
    }
  }
}

} // namespace tsv::app::detail