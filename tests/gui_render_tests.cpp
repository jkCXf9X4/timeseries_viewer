#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "support/scripted_gui_backend.hpp"
#include "support/temp_dir.hpp"
#include "timeseries_viewer/app_model.hpp"
#include "timeseries_viewer/app_ui.hpp"

namespace fs = std::filesystem;

namespace {

fs::path write_csv(const fs::path& path, const std::vector<std::string>& headers, std::size_t rows, double scale = 1.0) {
  std::ofstream out(path);
  for (std::size_t i = 0; i < headers.size(); ++i) {
    if (i > 0) {
      out << ",";
    }
    out << headers[i];
  }
  out << "\n";

  for (std::size_t row = 0; row < rows; ++row) {
    for (std::size_t col = 0; col < headers.size(); ++col) {
      if (col > 0) {
        out << ",";
      }
      if (col == 0) {
        out << row;
      } else {
        out << scale * static_cast<double>(row + col);
      }
    }
    out << "\n";
  }

  return path;
}

} // namespace

TEST_CASE("Rendered GUI workflow can open sources, browse parameters, bind, and save/load", "[ui][workflow]") {
  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_gui_render_tests");
  const auto source_a = write_csv(
    temp_dir / "run_a.csv",
    {"time", "ECS_HW.consumerFeed.hh", "ECS_HW.consumerFeed.m", "ECS_HW.consumerRet.temp"},
    8,
    1.0
  );
  const auto source_b = write_csv(
    temp_dir / "run_b.csv",
    {"time", "ECS_HW.consumerFeed.hh", "ECS_HW.consumerFeed.m", "ECS_HW.consumerRet.temp"},
    8,
    2.0
  );
  const auto project_file = temp_dir / "workspace.json";

  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);

  tsv::test::ScriptedGuiBackend ui;
  ui.set_open_source_dialog(source_a);
  ui.click("open-source");
  tsv::ui::render_app(app, ui);

  ui.set_open_source_dialog(source_b);
  ui.click("open-source");
  tsv::ui::render_app(app, ui);

  REQUIRE(app.sources.size() == 2);
  REQUIRE(tsv::app::list_bindable_parameters(app.sources.front()).size() == 3);

  ui.click("new-window");
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows.size() == 2);

  ui.click("new-tab");
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[1].tabs.size() == 2);

  ui.set_checkbox("run_a.ECS_HW.consumerFeed.hh::selected", true);
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[1].tabs[1].series.size() == 1);
  REQUIRE(app.workspace.windows[1].tabs[1].series[0].name == "run_a.ECS_HW.consumerFeed.hh");

  ui.set_checkbox("run_b.ECS_HW.consumerFeed.hh::selected", true);
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[1].tabs[1].series.size() == 2);
  REQUIRE(app.workspace.windows[1].tabs[1].series[1].name == "run_b.ECS_HW.consumerFeed.hh");

  ui.click("run_a.ECS_HW.consumerFeed.hh::select");
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[1].tabs[1].active_series_index == 0);

  ui.set_text("w1_t1::expression-draft", "series(\"run_a.ECS_HW.consumerFeed.hh\") - series(\"run_b.ECS_HW.consumerFeed.hh\")");
  ui.click("w1_t1::add-derived");
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[1].tabs[1].series.size() == 3);
  REQUIRE(app.workspace.windows[1].tabs[1].series[2].derived);

  ui.set_save_project_dialog(project_file);
  ui.click("save-project");
  tsv::ui::render_app(app, ui);
  REQUIRE(fs::exists(project_file));

  tsv::test::ScriptedGuiBackend reload_ui;
  reload_ui.set_open_project_dialog(project_file);
  reload_ui.click("open-project");
  tsv::app::AppState reloaded;
  tsv::ui::render_app(reloaded, reload_ui);

  REQUIRE(reloaded.workspace.windows.size() == 2);
  REQUIRE(reloaded.workspace.windows[1].tabs.size() == 2);
  REQUIRE(reloaded.workspace.windows[1].tabs[1].series.size() == 3);
  REQUIRE(reloaded.workspace.windows[1].tabs[1].series[0].name == "run_a.ECS_HW.consumerFeed.hh");
  REQUIRE(reloaded.workspace.windows[1].tabs[1].series[1].name == "run_b.ECS_HW.consumerFeed.hh");
  REQUIRE(reloaded.workspace.windows[1].tabs[1].series[2].derived);

  tsv::test::ScriptedGuiBackend recorder;
  tsv::ui::render_app(reloaded, recorder);
  REQUIRE(recorder.plot_labels.size() == 3);
  REQUIRE(std::any_of(recorder.plot_labels.begin(), recorder.plot_labels.end(), [](const std::string& label) {
    return label.find("run_a.ECS_HW.consumerFeed.hh") != std::string::npos;
  }));
  REQUIRE(std::any_of(recorder.plot_labels.begin(), recorder.plot_labels.end(), [](const std::string& label) {
    return label.find("run_b.ECS_HW.consumerFeed.hh") != std::string::npos;
  }));
  REQUIRE(std::any_of(recorder.plot_labels.begin(), recorder.plot_labels.end(), [](const std::string& label) {
    return label.find(":=") != std::string::npos;
  }));
  std::vector<std::string> unique_labels = recorder.plot_labels;
  std::sort(unique_labels.begin(), unique_labels.end());
  REQUIRE(std::adjacent_find(unique_labels.begin(), unique_labels.end()) == unique_labels.end());
}

TEST_CASE("Rendered variable browsing keeps parent nodes navigation-only and scales with many parameters", "[ui][browser]") {
  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_gui_render_tests");
  std::vector<std::string> headers = {"time"};
  for (int i = 0; i < 40; ++i) {
    headers.push_back("ECS_HW.node" + std::to_string(i) + ".signal");
  }
  const auto source_path = write_csv(temp_dir / "large_dotted.csv", headers, 12, 1.0);

  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);
  tsv::app::open_source(app, source_path, "dotted", tsv::SourceKind::Csv);
  tsv::app::rebuild_cache(app);

  REQUIRE(tsv::app::list_bindable_parameters(app.sources.front()).size() == 40);

  const auto tree = tsv::app::build_bindable_parameter_tree(app.sources.front());
  REQUIRE(tree.children.size() == 1);
  REQUIRE(tree.children.front().label == "dotted");

  tsv::test::ScriptedGuiBackend ui;
  ui.click("dotted.ECS_HW");
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[0].tabs[0].series.empty());

  ui.set_checkbox("dotted.ECS_HW.node17.signal::selected", true);
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[0].tabs[0].series.size() == 1);
  REQUIRE(app.workspace.windows[0].tabs[0].series[0].name == "dotted.ECS_HW.node17.signal");

  ui.set_checkbox("dotted.ECS_HW.node17.signal::selected", false);
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[0].tabs[0].series.empty());
}

TEST_CASE("Rendered plot labels remain distinct for similar series names", "[ui][labels]") {
  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_gui_render_tests");
  const auto source_a = write_csv(temp_dir / "label_a.csv", {"time", "speed"}, 8, 1.0);
  const auto source_b = write_csv(temp_dir / "label_b.csv", {"time", "speed"}, 8, 2.0);

  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);
  tsv::app::open_source(app, source_a, "run_a", tsv::SourceKind::Csv);
  tsv::app::open_source(app, source_b, "run_b", tsv::SourceKind::Csv);
  tsv::app::rebuild_cache(app);

  tsv::test::ScriptedGuiBackend ui;
  ui.set_checkbox("run_a.speed::selected", true);
  tsv::ui::render_app(app, ui);
  ui.set_checkbox("run_b.speed::selected", true);
  tsv::ui::render_app(app, ui);

  REQUIRE(app.workspace.windows[0].tabs[0].series.size() == 2);

  tsv::test::ScriptedGuiBackend recorder;
  tsv::ui::render_app(app, recorder);

  REQUIRE(recorder.plot_labels.size() == 2);
  REQUIRE(recorder.plot_labels[0] != recorder.plot_labels[1]);
  REQUIRE(std::any_of(recorder.plot_labels.begin(), recorder.plot_labels.end(), [](const std::string& label) {
    return label.find("run_a") != std::string::npos;
  }));
  REQUIRE(std::any_of(recorder.plot_labels.begin(), recorder.plot_labels.end(), [](const std::string& label) {
    return label.find("run_b") != std::string::npos;
  }));
}

TEST_CASE("Rendered UI pins the parameter browser left and the plot inspector right", "[ui][layout]") {
  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_gui_render_tests");
  const auto source = write_csv(temp_dir / "layout.csv", {"time", "speed"}, 8, 1.0);

  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);
  tsv::app::open_source(app, source, "run", tsv::SourceKind::Csv);
  tsv::app::rebuild_cache(app);

  tsv::test::ScriptedGuiBackend ui;
  ui.set_u64("point-budget", 1024);
  tsv::ui::render_app(app, ui);
  ui.set_viewport_size({1600.0f, 720.0f});
  tsv::ui::render_app(app, ui);

  REQUIRE(std::any_of(ui.window_size_log.begin(), ui.window_size_log.end(), [](const auto& item) {
    return item.first == "Parameters" && item.second[0] == Catch::Approx(360.0f) && item.second[1] == Catch::Approx(900.0f);
  }));
  REQUIRE(std::any_of(ui.window_size_log.begin(), ui.window_size_log.end(), [](const auto& item) {
    return item.first == "Plot Inspector" && item.second[0] == Catch::Approx(420.0f) && item.second[1] == Catch::Approx(900.0f);
  }));
  REQUIRE(std::any_of(ui.window_size_log.begin(), ui.window_size_log.end(), [](const auto& item) {
    return item.first == "Parameters" && item.second[0] == Catch::Approx(360.0f) && item.second[1] == Catch::Approx(720.0f);
  }));
  REQUIRE(std::any_of(ui.window_size_log.begin(), ui.window_size_log.end(), [](const auto& item) {
    return item.first == "Plot Inspector" && item.second[0] == Catch::Approx(420.0f) && item.second[1] == Catch::Approx(720.0f);
  }));
  REQUIRE(std::any_of(ui.window_pos_log.begin(), ui.window_pos_log.end(), [](const auto& item) {
    return item.first == "Parameters" && item.second[0] == Catch::Approx(0.0f) && item.second[1] == Catch::Approx(0.0f);
  }));
  REQUIRE(std::any_of(ui.window_pos_log.begin(), ui.window_pos_log.end(), [](const auto& item) {
    return item.first == "Plot Inspector" && item.second[0] == Catch::Approx(1180.0f) && item.second[1] == Catch::Approx(0.0f);
  }));
  REQUIRE(std::count(ui.separator_log.begin(), ui.separator_log.end(), "Active Plot") == 2);
  REQUIRE(app.workspace.point_budget == 1024);
  REQUIRE(std::any_of(ui.text_log.begin(), ui.text_log.end(), [](const std::string& text) {
    return text == "Window: Window 1";
  }));
  REQUIRE(std::any_of(ui.text_log.begin(), ui.text_log.end(), [](const std::string& text) {
    return text == "Tab: Plot 1";
  }));
}

TEST_CASE("Rendered UI surfaces empty and error states", "[ui][state]") {
  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);

  tsv::test::ScriptedGuiBackend ui;
  tsv::ui::render_app(app, ui);

  REQUIRE(std::count(ui.text_disabled_log.begin(), ui.text_disabled_log.end(), "Open a CSV file or SQLite database to browse parameters.") == 1);
  REQUIRE(std::count(ui.text_disabled_log.begin(), ui.text_disabled_log.end(), "Open a CSV file or SQLite database to inspect the active plot.") == 1);

  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_gui_render_tests");
  const auto source = write_csv(temp_dir / "empty_state.csv", {"time", "speed"}, 4, 1.0);
  tsv::app::open_source(app, source, "run", tsv::SourceKind::Csv);
  tsv::app::rebuild_cache(app);

  tsv::test::ScriptedGuiBackend no_series_ui;
  tsv::ui::render_app(app, no_series_ui);
  REQUIRE(std::count(no_series_ui.text_disabled_log.begin(), no_series_ui.text_disabled_log.end(), "No series selected") == 1);

  app.series_errors["missing.series"] = "Value column not found";
  tsv::test::ScriptedGuiBackend error_ui;
  tsv::ui::render_app(app, error_ui);

  REQUIRE(std::count(error_ui.separator_log.begin(), error_ui.separator_log.end(), "Series Errors") == 1);
  REQUIRE(std::any_of(error_ui.text_log.begin(), error_ui.text_log.end(), [](const std::string& text) {
    return text.find("missing.series: Value column not found") != std::string::npos;
  }));
}

TEST_CASE("Active plot follows focused windows and clicked tabs", "[ui][focus]") {
  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);
  tsv::app::add_window(app, "Window 2");
  tsv::app::add_tab(app, 1, "Plot 2");
  app.active_window = 0;
  app.workspace.windows[1].active_tab = 1;

  tsv::test::ScriptedGuiBackend ui;
  ui.set_focused_window("Window 2");
  ui.click("tab-1-1");
  tsv::ui::render_app(app, ui);

  REQUIRE(app.active_window == 1);
  REQUIRE(app.workspace.windows[1].active_tab == 1);
  REQUIRE(std::any_of(ui.text_log.begin(), ui.text_log.end(), [](const std::string& text) {
    return text == "Window: Window 2";
  }));
  REQUIRE(std::any_of(ui.text_log.begin(), ui.text_log.end(), [](const std::string& text) {
    return text == "Tab: Plot 2";
  }));
}

TEST_CASE("Clicking a different plot body promotes its window to active", "[ui][focus]") {
  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);
  tsv::app::add_window(app, "Window 2");
  tsv::app::add_tab(app, 1, "Plot 2");
  app.active_window = 0;
  app.workspace.windows[1].active_tab = 1;

  tsv::test::ScriptedGuiBackend ui;
  ui.click("plot##1_1");
  tsv::ui::render_app(app, ui);

  REQUIRE(app.active_window == 1);
  REQUIRE(app.workspace.windows[1].active_tab == 1);
  REQUIRE(std::count(ui.focus_requests.begin(), ui.focus_requests.end(), "Window 2") >= 1);
  REQUIRE_FALSE(ui.plot_setup_order_violated());
  REQUIRE(std::any_of(ui.text_log.begin(), ui.text_log.end(), [](const std::string& text) {
    return text == "Window: Window 2";
  }));
  REQUIRE(std::any_of(ui.text_log.begin(), ui.text_log.end(), [](const std::string& text) {
    return text == "Tab: Plot 2";
  }));
}

TEST_CASE("Creating a new window renders it immediately and its plot can take focus", "[ui][window][focus]") {
  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);

  tsv::test::ScriptedGuiBackend ui;
  ui.click("new-window");
  tsv::ui::render_app(app, ui);

  REQUIRE(app.workspace.windows.size() == 2);
  REQUIRE(app.active_window == 1);
  REQUIRE(std::count(ui.window_titles.begin(), ui.window_titles.end(), "Window 2") >= 1);
  REQUIRE(ui.begin_window_count() == ui.end_window_count());
  REQUIRE(std::any_of(ui.window_pos_log.begin(), ui.window_pos_log.end(), [](const auto& item) {
    return item.first == "Window 2" && item.second[0] > 360.0f;
  }));
  REQUIRE(std::count(ui.focus_requests.begin(), ui.focus_requests.end(), "Window 2") >= 1);

  tsv::test::ScriptedGuiBackend plot_ui;
  plot_ui.set_focused_window("Window 2");
  plot_ui.click("plot##1_0");
  tsv::ui::render_app(app, plot_ui);

  REQUIRE(app.active_window == 1);
  REQUIRE(app.workspace.windows[1].active_tab == 0);
  REQUIRE(std::count(plot_ui.focus_requests.begin(), plot_ui.focus_requests.end(), "Window 2") >= 1);
  REQUIRE(plot_ui.begin_window_count() == plot_ui.end_window_count());
}
