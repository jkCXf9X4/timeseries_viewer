#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

#include "timeseries_viewer/app_model.hpp"

namespace tsv::ui::detail {

template <typename Ui>
void render_analysis_windows(tsv::app::AppState& app, Ui& ui) {
  tsv::app::ensure_workspace_defaults(app);
  std::size_t window_index = 0;
  while (window_index < app.workspace.windows.size()) {
    auto& window = app.workspace.windows[window_index];
    if (!ui.begin_window(window.title, std::string("analysis-window-") + std::to_string(window_index), 0u)) {
      ++window_index;
      continue;
    }
    if (ui.window_focused()) app.active_window = static_cast<int>(window_index);
    if (ui.button("New tab", std::string("window-new-tab-") + std::to_string(window_index))) {
      tsv::app::add_tab(app, window_index);
    }
    ui.same_line();
    ui.text_disabled(std::string("Window ") + std::to_string(window_index + 1));
    if (ui.begin_tab_bar(std::string("tabs##") + std::to_string(window_index))) {
      for (std::size_t tab_index = 0; tab_index < window.tabs.size(); ++tab_index) {
        auto& tab = window.tabs[tab_index];
        const bool was_active_tab = window.active_tab == tab_index;
        if (ui.begin_tab_item(tab.title, window.active_tab == tab_index, std::string("tab-") + std::to_string(window_index) + "-" + std::to_string(tab_index))) {
          if (!was_active_tab) {
            window.active_tab = tab_index;
            app.active_window = static_cast<int>(window_index);
          }
          tsv::app::ensure_tab_data(app, window_index, tab_index);
          const auto plot_size = ui.content_region_avail();
          if (ui.begin_plot(std::string("plot##") + std::to_string(window_index) + "_" + std::to_string(tab_index), {plot_size[0], plot_size[1]})) {
            ui.setup_axes("time", "value", tab.autoscale_x, tab.autoscale_y);
            if (!tab.autoscale_x && tab.x_range.has_value()) ui.setup_axis_limits("x", tab.x_range->at(0), tab.x_range->at(1));
            if (!tab.autoscale_y && tab.y_range.has_value()) ui.setup_axis_limits("y", tab.y_range->at(0), tab.y_range->at(1));
            if (ui.plot_clicked()) {
              app.active_window = static_cast<int>(window_index);
              window.active_tab = tab_index;
            }
            for (const auto& series_cfg : tab.series) {
              const auto it = app.series_cache.find(series_cfg.name);
              if (it == app.series_cache.end()) continue;
              const auto& series = it->second;
              if (series_cfg.visible && !series.time.empty() && !series.value.empty()) {
                ui.plot_line(tsv::app::plot_legend_label(series_cfg), series, series_cfg.color);
              }
            }
            ui.end_plot();
          }
          ui.end_tab_item();
        }
      }
      ui.end_tab_bar();
    }
    bool removed = false;
    if (ui.begin_popup_context_window()) {
      if (ui.menu_item("Close", std::string("close-window-") + std::to_string(window_index))) {
        tsv::app::remove_window(app, window_index);
        removed = true;
      }
      ui.end_popup();
    }
    ui.end_window();
    if (!removed) ++window_index;
  }
}

} // namespace tsv::ui::detail