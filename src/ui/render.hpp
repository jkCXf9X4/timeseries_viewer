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

/// Renders the full application UI for the current frame.
/// Delegates to panel-specific render functions for analysis windows,
/// parameter panel, plot inspector, and status bar.
/// @tparam Ui  The UI backend type (e.g., ImGuiBackend or test harness).
/// @param app  The application state to render.
/// @param ui   The UI backend instance used for rendering primitives.
template <typename Ui>
void render_app(tsv::app::AppState& app, Ui& ui) {
  detail::render_analysis_windows(app, ui);
  detail::render_parameter_panel(app, ui);
  detail::render_plot_inspector_panel(app, ui);
  detail::render_status_bar(app, ui);
  app.sidebar_layout_initialized = true;
}

} // namespace tsv::ui