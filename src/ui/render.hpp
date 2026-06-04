#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include "timeseries_viewer/app_model.hpp"
#include "ui/panels/parameter.hpp"
#include "ui/panels/inspector.hpp"
#include "ui/panels/status.hpp"
#include "ui/plot/workspace.hpp"

namespace tsv::ui {

template <typename Ui>
void render_app(tsv::app::AppState& app, Ui& ui) {
  detail::render_analysis_windows(app, ui);
  detail::render_parameter_panel(app, ui);
  detail::render_plot_inspector_panel(app, ui);
  detail::render_status_bar(app, ui);
  app.sidebar_layout_initialized = true;
}

} // namespace tsv::ui