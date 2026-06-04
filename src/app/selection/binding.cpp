#include "app_internal.hpp"

namespace tsv::app {

std::vector<BindableParameter> list_bindable_parameters(const OpenSource& source) {
  const auto& parameters = bindable_parameters(source);
  return {parameters.begin(), parameters.end()};
}

tsv::TreeNode build_bindable_parameter_tree(const OpenSource& source) {
  return bindable_parameter_tree(source);
}

std::optional<BindableParameter> find_bindable_parameter(const OpenSource& source, const std::string& display_name) {
  const auto* parameter = lookup_bindable_parameter(source, display_name);
  if (parameter == nullptr) return std::nullopt;
  return *parameter;
}

const std::vector<BindableParameter>& bindable_parameters(const OpenSource& source) {
  if (!source.bindable_parameter_cache_ready) detail::rebuild_bindable_parameter_cache(source);
  return source.bindable_parameter_cache;
}

const tsv::TreeNode& bindable_parameter_tree(const OpenSource& source) {
  if (!source.bindable_parameter_cache_ready) detail::rebuild_bindable_parameter_cache(source);
  return source.bindable_parameter_tree_cache;
}

const BindableParameter* lookup_bindable_parameter(const OpenSource& source, const std::string& display_name) {
  if (!source.bindable_parameter_cache_ready) detail::rebuild_bindable_parameter_cache(source);
  const auto it = source.bindable_parameter_lookup.find(display_name);
  if (it == source.bindable_parameter_lookup.end()) return nullptr;
  return &source.bindable_parameter_cache.at(it->second);
}

std::string plot_legend_label(const tsv::PlotSeriesConfig& series) {
  if (series.derived) {
    if (!series.expression.empty()) return series.name + " := " + series.expression;
    return series.name + " (derived)";
  }
  std::string label = series.name;
  if (series.source_alias.has_value() && !series.source_alias->empty()) label += " [" + *series.source_alias + "]";
  if (series.source_path.has_value() && !series.source_path->empty()) label += " (" + fs::path(*series.source_path).filename().string() + ")";
  return label;
}

bool parameter_is_selected(const tsv::PlotTabConfig& tab, const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column) {
  return std::any_of(tab.series.begin(), tab.series.end(), [&](const tsv::PlotSeriesConfig& series) { return detail::series_matches_binding(series, source, table_name, value_column); });
}

std::size_t remove_parameter_series(AppState& app, std::size_t window_index, std::size_t tab_index, const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column) {
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  const auto before = tab.series.size();
  tab.series.erase(std::remove_if(tab.series.begin(), tab.series.end(), [&](const tsv::PlotSeriesConfig& series) { return detail::series_matches_binding(series, source, table_name, value_column); }), tab.series.end());
  if (tab.series.empty()) { tab.active_series_index = 0; }
  else if (tab.active_series_index >= tab.series.size()) { tab.active_series_index = tab.series.size() - 1; }
  if (tab.series.size() != before) rebuild_cache(app);
  return before - tab.series.size();
}

void set_parameter_selected(AppState& app, std::size_t window_index, std::size_t tab_index, const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column, bool selected) {
  const auto is_selected = parameter_is_selected(app.workspace.windows.at(window_index).tabs.at(tab_index), source, table_name, value_column);
  if (selected == is_selected) return;
  if (selected) { add_raw_series(app, source, table_name, value_column); return; }
  remove_parameter_series(app, window_index, tab_index, source, table_name, value_column);
}

} // namespace tsv::app