#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "timeseries_viewer/app_model.hpp"
#include "support/gui_harness.hpp"
#include "support/sqlite_fixture.hpp"
#include "support/temp_dir.hpp"

namespace fs = std::filesystem;

namespace {

fs::path write_csv(const fs::path& path, int rows, double speed_scale = 1.0) {
  std::ofstream out(path);
  out << "time,speed,altitude\n";
  for (int i = 0; i < rows; ++i) {
    out << i << "," << (speed_scale * static_cast<double>(i)) << "," << (100.0 + i) << "\n";
  }
  return path;
}

} // namespace

TEST_CASE("Workspace state keeps per-tab expression drafts isolated", "[app][workspace]") {
  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);
  tsv::app::add_tab(app, 0, "second");

  app.workspace.windows[0].tabs[0].expression_draft = "series(\"run_a.speed\")";
  app.workspace.windows[0].tabs[1].expression_draft = "series(\"run_b.speed\")";

  REQUIRE(app.workspace.windows[0].tabs[0].expression_draft == "series(\"run_a.speed\")");
  REQUIRE(app.workspace.windows[0].tabs[1].expression_draft == "series(\"run_b.speed\")");
}

TEST_CASE("Binding manager can rebind a selected series across sources", "[app][binding]") {
  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_app_model_tests");
  const auto source_a = write_csv(temp_dir / "run_a.csv", 5, 1.0);
  const auto source_b = write_csv(temp_dir / "run_b.csv", 5, 2.0);

  tsv::test::GuiHarness harness;
  harness.open_source(source_a, "run_a");
  harness.open_source(source_b, "run_b");
  harness.click_add_raw_to_active("run_a", std::nullopt, "speed");
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series.size() == 1);
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series[0].name == "run_a.speed");
  harness.click_bind_selected(0, 0, 0, "run_b", std::nullopt, "altitude");
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series[0].name == "run_b.altitude");
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series[0].value_column == "altitude");
  REQUIRE(harness.state().series_cache.contains("run_b.altitude"));
}

TEST_CASE("Large series are downsampled to the configured point budget", "[app][scale]") {
  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_app_model_tests");
  const auto source = write_csv(temp_dir / "large.csv", 100, 1.0);

  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);
  app.workspace.point_budget = 10;
  tsv::app::open_source(app, source, "large", tsv::SourceKind::Csv);

  const auto src = std::find_if(app.sources.begin(), app.sources.end(), [&](const auto& item) {
    return item.alias == "large";
  });
  REQUIRE(src != app.sources.end());

  tsv::app::add_raw_series(app, *src, std::nullopt, "speed");
  REQUIRE(app.series_cache.contains("large.speed"));
  REQUIRE(app.series_cache.at("large.speed").time.size() == 10);
}

TEST_CASE("App project save/load preserves multi-tab state and point budget", "[app][project]") {
  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_app_model_tests");
  const auto source_a = write_csv(temp_dir / "project_run_a.csv", 8, 1.0);
  const auto source_b = write_csv(temp_dir / "project_run_b.csv", 8, 2.0);
  const auto project_file = temp_dir / "project.json";

  tsv::test::GuiHarness harness;
  harness.open_source(source_a, "run_a");
  harness.open_source(source_b, "run_b");
  harness.set_point_budget(12);
  harness.click_add_raw_to_active("run_a", std::nullopt, "speed");
  harness.click_new_tab(0, "compare");
  harness.click_add_raw_to_active("run_b", std::nullopt, "altitude");
  harness.set_expression_draft(0, 0, "series(\"run_a.speed\")");
  harness.set_expression_draft(0, 1, "series(\"run_b.altitude\")");
  harness.save(project_file);

  tsv::test::GuiHarness loaded;
  loaded.load(project_file);

  REQUIRE(loaded.state().workspace.point_budget == 12);
  REQUIRE(loaded.state().workspace.windows.size() == 1);
  REQUIRE(loaded.state().workspace.windows[0].tabs.size() == 2);
  REQUIRE(loaded.state().workspace.windows[0].tabs[0].expression_draft == "series(\"run_a.speed\")");
  REQUIRE(loaded.state().workspace.windows[0].tabs[1].expression_draft == "series(\"run_b.altitude\")");
  REQUIRE(loaded.state().workspace.windows[0].tabs[0].series.size() == 1);
  REQUIRE(loaded.state().workspace.windows[0].tabs[1].series.size() == 1);
}

TEST_CASE("GUI harness can open sqlite sources and compare them in one plot", "[app][sqlite][compare]") {
  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_app_model_tests");
  const auto sqlite_path = tsv::test::make_sqlite_fixture(temp_dir / "telemetry.sqlite");
  const auto csv_path = write_csv(temp_dir / "telemetry.csv", 6, 2.0);

  tsv::test::GuiHarness harness;
  harness.open_sqlite(sqlite_path, "run_sql");
  harness.open_source(csv_path, "run_csv");
  harness.click_add_raw_to_active("run_sql", std::optional<std::string>{"telemetry"}, "speed");
  harness.click_add_raw_to_active("run_csv", std::nullopt, "speed");

  REQUIRE(harness.state().workspace.windows[0].tabs[0].series.size() == 2);
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series[0].source_alias == "run_sql");
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series[0].table_name == "telemetry");
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series[1].source_alias == "run_csv");
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series[1].table_name == std::nullopt);
}

TEST_CASE("Parameter selection stays isolated between tabs", "[app][selection]") {
  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_app_model_tests");
  const auto source = write_csv(temp_dir / "selection.csv", 5, 1.0);

  tsv::test::GuiHarness harness;
  harness.open_source(source, "run");
  harness.click_new_tab(0, "secondary");

  harness.select_tab(0, 0);
  harness.click_add_raw_to_active("run", std::nullopt, "speed");
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series.size() == 1);
  REQUIRE(harness.state().workspace.windows[0].tabs[1].series.empty());

  harness.select_tab(0, 1);
  harness.click_add_raw_to_active("run", std::nullopt, "altitude");
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series.size() == 1);
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series[0].name == "run.speed");
  REQUIRE(harness.state().workspace.windows[0].tabs[1].series.size() == 1);
  REQUIRE(harness.state().workspace.windows[0].tabs[1].series[0].name == "run.altitude");
}

TEST_CASE("App loader reconstructs legacy project bindings", "[app][project]") {
  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_app_model_tests");
  const auto source = write_csv(temp_dir / "legacy.csv", 6, 1.0);
  const auto project_file = temp_dir / "legacy_project.json";

  const nlohmann::json legacy = {
    {"sources", {
      {
        {"kind", "csv"},
        {"path", source.string()},
        {"alias", "legacy"},
        {"selected_variables", {"legacy.speed"}}
      }
    }},
    {"views", {
      {
        {"title", "main"},
        {"series", {
          {
            {"name", "legacy.speed"},
            {"expression", ""},
            {"visible", true},
            {"derived", false},
            {"color", {1.0, 0.0, 0.0, 1.0}}
          }
        }},
        {"autoscale_x", true},
        {"autoscale_y", true}
      }
    }}
  };
  {
    std::ofstream out(project_file);
    out << legacy.dump(2) << '\n';
  }

  tsv::test::GuiHarness harness;
  harness.load(project_file);

  REQUIRE(harness.state().workspace.windows.size() == 1);
  REQUIRE(harness.state().workspace.windows[0].tabs.size() == 1);
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series.size() == 1);
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series[0].name == "legacy.speed");
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series[0].source_alias == "legacy");
  REQUIRE(harness.state().series_cache.contains("legacy.speed"));
}

TEST_CASE("GUI harness verifies the full scripted usage flow", "[app][harness]") {
  const auto temp_dir = tsv::test::make_temp_dir("timeseries_viewer_app_model_tests");
  const auto source_a = write_csv(temp_dir / "harness_a.csv", 20, 1.0);
  const auto source_b = write_csv(temp_dir / "harness_b.csv", 20, 2.0);
  const auto project_file = temp_dir / "harness_project.json";

  tsv::test::GuiHarness harness;
  harness.open_source(source_a, "run_a");
  harness.open_source(source_b, "run_b");
  harness.click_add_raw_to_active("run_a", std::nullopt, "speed");
  harness.click_new_window("comparison");
  harness.select_window(1);
  harness.click_new_tab(1, "delta");
  harness.click_add_raw_to_active("run_b", std::nullopt, "altitude");
  harness.set_expression_draft(1, 1, "series(\"run_a.speed\") - series(\"run_b.altitude\")");
  harness.click_add_derived(1, 1);
  harness.click_toggle_series_visibility(1, 1, 0, false);
  harness.save(project_file);

  REQUIRE(harness.state().workspace.windows.size() == 2);
  REQUIRE(harness.state().workspace.windows[0].tabs[0].series.size() == 1);
  REQUIRE(harness.state().workspace.windows[1].tabs[1].series.size() == 2);
  REQUIRE(harness.state().series_cache.contains("run_a.speed"));
  REQUIRE(harness.state().series_cache.contains("run_b.altitude"));

  tsv::test::GuiHarness reloaded;
  reloaded.load(project_file);
  REQUIRE(reloaded.state().workspace.windows.size() == 2);
  REQUIRE(reloaded.state().workspace.windows[1].tabs[1].series.size() == 2);
  REQUIRE(reloaded.state().series_cache.contains("run_a.speed"));
  REQUIRE(reloaded.state().series_cache.contains("run_b.altitude"));
}
