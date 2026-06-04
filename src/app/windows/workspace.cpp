#include "app_internal.hpp"

namespace tsv::app {

void ensure_workspace_defaults(AppState& app) {
  if (app.workspace.windows.empty()) {
    AnalysisWindowConfig window;
    window.title = "Window 1";
    window.tabs.push_back(PlotTabConfig{});
    app.workspace.windows.push_back(std::move(window));
  }
  for (std::size_t index = 0; index < app.workspace.windows.size(); ++index) {
    auto& window = app.workspace.windows[index];
    if (window.title.empty()) window.title = "Window " + std::to_string(index + 1);
    if (window.tabs.empty()) window.tabs.push_back(PlotTabConfig{});
    if (window.active_tab >= window.tabs.size()) window.active_tab = 0;
    for (std::size_t tab_index = 0; tab_index < window.tabs.size(); ++tab_index) {
      auto& tab = window.tabs[tab_index];
      if (tab.title.empty()) tab.title = "Plot " + std::to_string(tab_index + 1);
      if (tab.active_series_index >= tab.series.size() && !tab.series.empty()) tab.active_series_index = 0;
    }
  }
  if (app.active_window < 0 || app.active_window >= static_cast<int>(app.workspace.windows.size())) app.active_window = 0;
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

void remove_window(AppState& app, std::size_t window_index) {
  if (window_index >= app.workspace.windows.size()) return;
  app.workspace.windows.erase(app.workspace.windows.begin() + static_cast<std::ptrdiff_t>(window_index));
  if (app.workspace.windows.empty()) ensure_workspace_defaults(app);
  if (app.active_window >= static_cast<int>(app.workspace.windows.size())) app.active_window = static_cast<int>(app.workspace.windows.size()) - 1;
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
  if (series_index >= tab.series.size()) return;
  tab.series.erase(tab.series.begin() + static_cast<std::ptrdiff_t>(series_index));
  if (tab.active_series_index >= tab.series.size() && !tab.series.empty()) tab.active_series_index = tab.series.size() - 1;
}

void toggle_series_visibility(AppState& app, std::size_t window_index, std::size_t tab_index, std::size_t series_index, bool visible) {
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  if (series_index < tab.series.size()) tab.series[series_index].visible = visible;
}

void rename_series(AppState& app, std::size_t window_index, std::size_t tab_index, std::size_t series_index, const std::string& name) {
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);
  if (series_index < tab.series.size()) tab.series[series_index].name = name;
}

} // namespace tsv::app