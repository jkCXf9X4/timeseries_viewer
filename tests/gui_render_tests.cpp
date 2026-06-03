#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "support/scripted_gui_backend.hpp"
#include "timeseries_viewer/app_model.hpp"
#include "timeseries_viewer/app_ui.hpp"

namespace fs = std::filesystem;

namespace {

fs::path make_temp_dir() {
  const auto base = fs::temp_directory_path() / "timeseries_viewer_gui_render_tests";
  fs::create_directories(base);
  return base;
}

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
  const auto temp_dir = make_temp_dir();
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

  ui.click("run_a.ECS_HW.consumerFeed.hh");
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[1].tabs[1].series.size() == 1);
  REQUIRE(app.workspace.windows[1].tabs[1].series[0].name == "run_a.ECS_HW.consumerFeed.hh");

  ui.click("run_b.ECS_HW.consumerFeed.hh");
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[1].tabs[1].series.size() == 2);
  REQUIRE(app.workspace.windows[1].tabs[1].series[1].name == "run_b.ECS_HW.consumerFeed.hh");

  ui.click("run_a.ECS_HW.consumerFeed.hh::select");
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[1].tabs[1].active_series_index == 0);

  ui.click("run_b.ECS_HW.consumerFeed.m::bind");
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[1].tabs[1].series[0].name == "run_b.ECS_HW.consumerFeed.m");
  REQUIRE(app.workspace.windows[1].tabs[1].series[1].name == "run_b.ECS_HW.consumerFeed.hh");

  ui.set_text("w1_t1::expression-draft", "series(\"run_b.ECS_HW.consumerFeed.hh\") - series(\"run_b.ECS_HW.consumerFeed.m\")");
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
  REQUIRE(reloaded.workspace.windows[1].tabs[1].series[0].name == "run_b.ECS_HW.consumerFeed.m");
  REQUIRE(reloaded.workspace.windows[1].tabs[1].series[1].name == "run_b.ECS_HW.consumerFeed.hh");
  REQUIRE(reloaded.workspace.windows[1].tabs[1].series[2].derived);

  tsv::test::ScriptedGuiBackend recorder;
  tsv::ui::render_app(reloaded, recorder);
  REQUIRE(recorder.plot_labels.size() == 3);
  REQUIRE(std::any_of(recorder.plot_labels.begin(), recorder.plot_labels.end(), [](const std::string& label) {
    return label.find("run_b.ECS_HW.consumerFeed.m") != std::string::npos;
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
  const auto temp_dir = make_temp_dir();
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

  ui.click("dotted.ECS_HW.node17.signal");
  tsv::ui::render_app(app, ui);
  REQUIRE(app.workspace.windows[0].tabs[0].series.size() == 1);
  REQUIRE(app.workspace.windows[0].tabs[0].series[0].name == "dotted.ECS_HW.node17.signal");
}

TEST_CASE("Rendered plot labels remain distinct for similar series names", "[ui][labels]") {
  const auto temp_dir = make_temp_dir();
  const auto source_a = write_csv(temp_dir / "label_a.csv", {"time", "speed"}, 8, 1.0);
  const auto source_b = write_csv(temp_dir / "label_b.csv", {"time", "speed"}, 8, 2.0);

  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);
  tsv::app::open_source(app, source_a, "run_a", tsv::SourceKind::Csv);
  tsv::app::open_source(app, source_b, "run_b", tsv::SourceKind::Csv);
  tsv::app::rebuild_cache(app);

  tsv::test::ScriptedGuiBackend ui;
  ui.click("run_a.speed");
  tsv::ui::render_app(app, ui);
  ui.click("run_b.speed");
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
