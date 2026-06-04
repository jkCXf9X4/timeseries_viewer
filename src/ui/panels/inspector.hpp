#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "timeseries_viewer/app_model.hpp"
#include "ui/constants.hpp"

namespace tsv::ui::detail {

template <typename Ui>
void render_active_plot_summary(tsv::app::AppState& app, Ui& ui) {
  auto& window = tsv::app::active_window(app);
  auto& tab = tsv::app::active_tab(app);
  ui.separator_text("Active Plot");
  ui.text(std::string("Window: ") + window.title);
  ui.text(std::string("Tab: ") + tab.title);
  if (tab.active_series_index < tab.series.size()) {
    ui.text(std::string("Series: ") + tab.series[tab.active_series_index].name);
  } else {
    ui.text_disabled("Series: none");
  }
}

template <typename Ui>
void render_plot_inspector(tsv::app::AppState& app, Ui& ui) {
  tsv::app::ensure_workspace_defaults(app);
  render_active_plot_summary(app, ui);
  if (app.sources.empty()) {
    ui.text_disabled("Open a CSV file or SQLite database to inspect the active plot.");
    return;
  }
  auto& window = tsv::app::active_window(app);
  auto& tab = tsv::app::active_tab(app);
  const std::size_t window_index = static_cast<std::size_t>(app.active_window);
  const std::size_t tab_index = window.active_tab;
  const std::string scope = "w" + std::to_string(window_index) + "_t" + std::to_string(tab_index);

  ui.separator_text("Plot Settings");
  ui.input_text("Tab title", tab.title, scope + "::tab-title");
  ui.checkbox("Autoscale X", tab.autoscale_x, scope + "::autoscale-x");
  ui.checkbox("Autoscale Y", tab.autoscale_y, scope + "::autoscale-y");

  std::uint64_t budget = static_cast<std::uint64_t>(app.workspace.point_budget);
  if (ui.input_u64("Point budget", budget, "point-budget")) {
    app.workspace.point_budget = static_cast<std::size_t>(budget);
    tsv::app::rebuild_cache(app);
  }
  ui.text_disabled("0 means no explicit downsampling limit");

  if (!tab.autoscale_x) {
    std::array<double, 2> range{0.0, 1.0};
    if (tab.x_range.has_value()) range = *tab.x_range;
    if (ui.input_double_range("X range", range, "x-range")) tab.x_range = range;
  }
  if (!tab.autoscale_y) {
    std::array<double, 2> range{0.0, 1.0};
    if (tab.y_range.has_value()) range = *tab.y_range;
    if (ui.input_double_range("Y range", range, "y-range")) tab.y_range = range;
  }

  ui.separator_text("Series");
  for (std::size_t i = 0; i < tab.series.size(); ++i) {
    auto& series = tab.series[i];
    const auto series_key = series.name.empty() ? tab.title + "::series::" + std::to_string(i) : series.name;
    const bool selected = tab.active_series_index == i;
    ui.push_id(series_key);
    if (ui.selectable(series.name, selected, series_key + "::select")) {
      tsv::app::select_series(app, window_index, tab_index, i);
    }
    ui.pop_id();
  }

  if (tab.active_series_index < tab.series.size()) {
    auto& series = tab.series[tab.active_series_index];
    ui.separator_text("Selected Series");
    ui.input_text("Series name", series.name, scope + "::series-name");
    ui.checkbox("Visible##selected", series.visible, scope + "::selected-visible");
    ui.color_edit4("Series color", series.color, scope + "::selected-color");
    if (series.derived) {
      ui.input_text("Expression", series.expression, scope + "::selected-expression");
      if (ui.button("Update derived", scope + "::update-derived")) {
        tsv::app::rebuild_cache(app);
      }
    } else {
      const std::string source = series.source_alias.has_value() ? *series.source_alias : std::string{"?"};
      ui.text(std::string("Source: ") + source);
      std::string binding = source;
      if (series.table_name.has_value()) binding += "." + *series.table_name;
      if (series.value_column.has_value()) binding += "." + *series.value_column;
      ui.text(std::string("Binding: ") + binding);
      if (series.time_column.has_value()) ui.text(std::string("Time: ") + *series.time_column);
      ui.text(std::string("Plot label: ") + tsv::app::plot_legend_label(series));
    }
    if (ui.button("Remove selected", scope + "::remove-selected")) {
      tsv::app::remove_series(app, window_index, tab_index, tab.active_series_index);
    }
  }

  ui.separator_text("Derived Series");
  ui.input_text("Expression draft", tab.expression_draft, scope + "::expression-draft");
  if (ui.button("Add derived", scope + "::add-derived")) {
    tsv::app::add_derived_series_to_tab(app, window_index, tab_index);
  }
}

template <typename Ui>
void render_plot_inspector_panel(tsv::app::AppState& app, Ui& ui) {
  tsv::app::ensure_workspace_defaults(app);
  const auto viewport = ui.viewport_size();
  const float body_height = std::max(0.0f, viewport[1] - kStatusBarHeight);
  ui.set_next_window_size(app.plot_inspector_width, body_height);
  const auto x = viewport[0] - app.plot_inspector_width;
  ui.set_next_window_pos(x, 0.0f);
  if (!ui.begin_window("Plot Inspector", "plot-inspector-window", kFixedSidebarFlags)) return;
  render_plot_inspector(app, ui);
  app.plot_inspector_width = ui.window_size()[0];
  ui.end_window();
}

} // namespace tsv::ui::detail