#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

#include "timeseries_viewer/app_model.hpp"
#include "ui/constants.hpp"

namespace tsv::ui::detail {

template <typename Ui>
void render_parameter_tree(tsv::app::AppState& app, Ui& ui, const tsv::app::OpenSource& source, const tsv::TreeNode& node, std::size_t window_index, std::size_t tab_index) {
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  if (node.children.empty()) {
    const auto* binding = tsv::app::lookup_bindable_parameter(source, node.full_name);
    if (binding == nullptr) return;
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
void render_parameter_panel(tsv::app::AppState& app, Ui& ui) {
  tsv::app::ensure_workspace_defaults(app);
  const auto viewport = ui.viewport_size();
  const float body_height = std::max(0.0f, viewport[1] - kStatusBarHeight);
  ui.set_next_window_size(app.parameter_panel_width, body_height);
  ui.set_next_window_pos(0.0f, 0.0f);
  if (!ui.begin_window("Parameters", "parameters-window", kFixedSidebarFlags)) return;

  if (ui.button("Open", "open-source")) {
    if (const auto path = ui.open_source_dialog(); path.has_value()) {
      tsv::app::open_source(app, *path);
      tsv::app::rebuild_cache_metadata(app);
    }
  }
  ui.same_line();
  if (ui.button("Reload", "reload")) {
    tsv::app::reload_sources(app);
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
      const auto& tree = tsv::app::bindable_parameter_tree(source);
      const auto active_tab_index = app.workspace.windows.at(static_cast<std::size_t>(app.active_window)).active_tab;
      for (const auto& child : tree.children) {
        render_parameter_tree(app, ui, source, child, static_cast<std::size_t>(app.active_window), active_tab_index);
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
  app.parameter_panel_width = ui.window_size()[0];
  ui.end_window();
}

} // namespace tsv::ui::detail