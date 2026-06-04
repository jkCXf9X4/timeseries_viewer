#pragma once

#include <string>
#include <string_view>

#include "timeseries_viewer/app_model.hpp"
#include "ui/constants.hpp"

namespace tsv::ui::detail {

template <typename Ui>
void render_status_bar(tsv::app::AppState& app, Ui& ui) {
  const auto viewport = ui.viewport_size();
  ui.set_next_window_size(viewport[0], kStatusBarHeight);
  ui.set_next_window_pos(0.0f, viewport[1] - kStatusBarHeight);
  if (!ui.begin_window("Status", "status-bar", kStatusBarFlags)) return;
  ui.text(app.status);
  ui.end_window();
}

} // namespace tsv::ui::detail