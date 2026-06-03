#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "timeseries_viewer/app_model.hpp"

namespace tsv::ui {
namespace detail {

constexpr std::uint32_t kFixedSidebarFlags = 0x1u | 0x4u | 0x10u;

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
void render_parameter_tree(tsv::app::AppState& app, Ui& ui, const tsv::app::OpenSource& source, const tsv::TreeNode& node, std::size_t window_index, std::size_t tab_index) {
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);

  if (node.children.empty()) {
    const auto binding = tsv::app::find_bindable_parameter(source, node.full_name);
    if (!binding.has_value()) {
      return;
    }

    bool selected = tsv::app::parameter_is_selected(tab, source, binding->table_name, binding->value_column);

    if (ui.checkbox(node.label, selected, node.full_name + "::selected")) {
      tsv::app::set_parameter_selected(app, window_index, tab_index, source, binding->table_name, binding->value_column, selected);
    }
    return;
  }

  if (ui.tree_node(node.label, node.full_name)) {
    for (const auto& child : node.children) {
      render_parameter_tree(app, ui, source, child, window_index, tab_index);
    }
    ui.tree_pop();
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
    if (tab.x_range.has_value()) {
      range = *tab.x_range;
    }
    if (ui.input_double_range("X range", range, "x-range")) {
      tab.x_range = range;
    }
  }

  if (!tab.autoscale_y) {
    std::array<double, 2> range{0.0, 1.0};
    if (tab.y_range.has_value()) {
      range = *tab.y_range;
    }
    if (ui.input_double_range("Y range", range, "y-range")) {
      tab.y_range = range;
    }
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
    ui.same_line();
    ui.checkbox("Visible", series.visible, series_key + "::visible");
    ui.same_line();
    ui.color_edit4("Color", series.color, series_key + "::color");
    ui.same_line();
    if (ui.small_button("Remove", series_key + "::remove")) {
      tsv::app::remove_series(app, window_index, tab_index, i);
      ui.pop_id();
      break;
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
      if (series.table_name.has_value()) {
        binding += "." + *series.table_name;
      }
      if (series.value_column.has_value()) {
        binding += "." + *series.value_column;
      }
      ui.text(std::string("Binding: ") + binding);
      if (series.time_column.has_value()) {
        ui.text(std::string("Time: ") + *series.time_column);
      }
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
void render_parameter_panel(tsv::app::AppState& app, Ui& ui) {
  tsv::app::ensure_workspace_defaults(app);
  const auto viewport = ui.viewport_size();
  ui.set_next_window_size(app.parameter_panel_width, viewport[1]);
  ui.set_next_window_pos(0.0f, 0.0f);
  if (!ui.begin_window("Parameters", "parameters-window", kFixedSidebarFlags)) {
    return;
  }

  if (ui.button("Open", "open-source")) {
    if (const auto path = ui.open_source_dialog(); path.has_value()) {
      tsv::app::open_source(app, *path);
      tsv::app::rebuild_cache(app);
    }
  }
  ui.same_line();
  if (ui.button("Reload", "reload")) {
    tsv::app::rebuild_cache(app);
  }
  ui.same_line();
  ui.checkbox("Live", app.live_mode, "live-mode");

  if (ui.button("Open project", "open-project")) {
    if (const auto path = ui.open_project_dialog(); path.has_value()) {
      tsv::app::load_project_file(app, *path);
    }
  }
  ui.same_line();
  if (ui.button("Save project", "save-project")) {
    if (const auto path = ui.save_project_dialog(); path.has_value()) {
      tsv::app::save_project_file(app, *path);
    }
  }

  ui.same_line();
  if (ui.button("New window", "new-window")) {
    tsv::app::add_window(app);
  }

  ui.same_line();
  if (ui.button("New tab", "new-tab")) {
    tsv::app::add_tab(app, static_cast<std::size_t>(app.active_window));
  }

  ui.separator_text("Parameter Browser");
  if (app.sources.empty()) {
    ui.text_disabled("Open a CSV file or SQLite database to browse parameters.");
  }

  for (const auto& source : app.sources) {
    if (ui.tree_node(source.alias, source.alias)) {
      ui.text_disabled(source.catalog.path.string());
      const auto tree = tsv::app::build_bindable_parameter_tree(source);
      const auto active_tab_index = app.workspace.windows.at(static_cast<std::size_t>(app.active_window)).active_tab;
      for (const auto& child : tree.children) {
        detail::render_parameter_tree(app, ui, source, child, static_cast<std::size_t>(app.active_window), active_tab_index);
      }
      ui.tree_pop();
    }
  }

  if (!app.series_errors.empty()) {
    ui.separator_text("Series Errors");
    for (const auto& [name, error] : app.series_errors) {
      ui.text(name + ": " + error);
    }
  }

  ui.text(app.status);
  app.parameter_panel_width = ui.window_size()[0];
  ui.end_window();
}

template <typename Ui>
void render_analysis_windows(tsv::app::AppState& app, Ui& ui) {
  tsv::app::ensure_workspace_defaults(app);
  for (std::size_t window_index = 0; window_index < app.workspace.windows.size(); ++window_index) {
    auto& window = app.workspace.windows[window_index];
    if (!ui.begin_window(window.title, std::string("analysis-window-") + std::to_string(window_index), 0u)) {
      continue;
    }

    if (ui.window_focused()) {
      app.active_window = static_cast<int>(window_index);
    }

    if (ui.button("New tab", std::string("window-new-tab-") + std::to_string(window_index))) {
      tsv::app::add_tab(app, window_index);
    }
    ui.same_line();
    ui.text_disabled(std::string("Window ") + std::to_string(window_index + 1));

    if (ui.begin_tab_bar(std::string("tabs##") + std::to_string(window_index))) {
      for (std::size_t tab_index = 0; tab_index < window.tabs.size(); ++tab_index) {
        auto& tab = window.tabs[tab_index];
        if (ui.begin_tab_item(tab.title, window.active_tab == tab_index, std::string("tab-") + std::to_string(window_index) + "-" + std::to_string(tab_index))) {
          window.active_tab = tab_index;
          app.active_window = static_cast<int>(window_index);

          if (ui.begin_plot(std::string("plot##") + std::to_string(window_index) + "_" + std::to_string(tab_index))) {
            ui.setup_axes("time", "value", tab.autoscale_x, tab.autoscale_y);
            if (!tab.autoscale_x && tab.x_range.has_value()) {
              ui.setup_axis_limits("x", tab.x_range->at(0), tab.x_range->at(1));
            }
            if (!tab.autoscale_y && tab.y_range.has_value()) {
              ui.setup_axis_limits("y", tab.y_range->at(0), tab.y_range->at(1));
            }

            for (const auto& series_cfg : tab.series) {
              const auto it = app.series_cache.find(series_cfg.name);
              if (it == app.series_cache.end()) {
                continue;
              }
              const auto& series = it->second;
              if (series_cfg.visible && !series.time.empty() && !series.value.empty()) {
                ui.plot_line(tsv::app::plot_legend_label(series_cfg), series);
              }
            }
            ui.end_plot();
          }

          ui.end_tab_item();
        }
      }
      ui.end_tab_bar();
    }

    ui.end_window();
  }
}

template <typename Ui>
void render_plot_inspector_panel(tsv::app::AppState& app, Ui& ui) {
  tsv::app::ensure_workspace_defaults(app);
  const auto viewport = ui.viewport_size();
  ui.set_next_window_size(app.plot_inspector_width, viewport[1]);
  const auto x = viewport[0] - app.plot_inspector_width;
  ui.set_next_window_pos(x, 0.0f);
  if (!ui.begin_window("Plot Inspector", "plot-inspector-window", kFixedSidebarFlags)) {
    return;
  }

  render_plot_inspector(app, ui);
  app.plot_inspector_width = ui.window_size()[0];
  ui.end_window();
}

} // namespace detail

template <typename Ui>
void render_app(tsv::app::AppState& app, Ui& ui) {
  detail::render_analysis_windows(app, ui);
  detail::render_parameter_panel(app, ui);
  detail::render_plot_inspector_panel(app, ui);
  app.sidebar_layout_initialized = true;
}

} // namespace tsv::ui
