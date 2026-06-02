#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include <sqlite3.h>

#include "timeseries_viewer/core.hpp"

namespace fs = std::filesystem;

namespace {

fs::path fixture_path(const std::string& name) {
  return fs::path(TSV_SOURCE_DIR) / "tests" / "fixtures" / name;
}

fs::path make_temp_dir() {
  const auto base = fs::temp_directory_path() / "timeseries_viewer_tests";
  fs::create_directories(base);
  return base;
}

fs::path make_sqlite_fixture(const fs::path& path) {
  std::error_code ec;
  fs::remove(path, ec);
  sqlite3* db = nullptr;
  if (sqlite3_open(path.string().c_str(), &db) != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(db));
  }

  const char* schema =
    "CREATE TABLE telemetry (timestamp TEXT, speed REAL, altitude REAL);"
    "INSERT INTO telemetry VALUES ('2024-01-01T00:00:00', 10.0, 100.0);"
    "INSERT INTO telemetry VALUES ('2024-01-01T00:00:01', 11.0, 101.0);"
    "INSERT INTO telemetry VALUES ('2024-01-01T00:00:02', 12.0, 102.0);"
    "CREATE TABLE metadata (id INTEGER, label TEXT);"
    "INSERT INTO metadata VALUES (1, 'alpha');"
    "INSERT INTO metadata VALUES (2, 'beta');";

  char* error = nullptr;
  if (sqlite3_exec(db, schema, nullptr, nullptr, &error) != SQLITE_OK) {
    const std::string message = error != nullptr ? error : "unknown sqlite error";
    sqlite3_free(error);
    sqlite3_close(db);
    throw std::runtime_error(message);
  }
  sqlite3_close(db);
  return path;
}

} // namespace

TEST_CASE("CSV import detects numeric and date-like time columns", "[csv]") {
  const auto csv_numeric = tsv::load_csv_catalog(fixture_path("sample_numeric.csv"));
  REQUIRE(csv_numeric.tables.size() == 1);
  const auto& numeric_table = csv_numeric.tables.front();
  REQUIRE(numeric_table.columns.front().kind == tsv::ColumnKind::Numeric);
  REQUIRE(numeric_table.columns.front().time_candidate);
  REQUIRE(tsv::infer_time_column(numeric_table.columns) == "time");

  const auto csv_datetime = tsv::load_csv_catalog(fixture_path("sample_datetime.csv"));
  REQUIRE(csv_datetime.tables.size() == 1);
  const auto& datetime_table = csv_datetime.tables.front();
  REQUIRE(datetime_table.columns.front().kind == tsv::ColumnKind::DateTime);
  REQUIRE(datetime_table.columns.front().time_candidate);
  REQUIRE(tsv::infer_time_column(datetime_table.columns) == "timestamp");
}

TEST_CASE("CSV time-column override is honored", "[csv]") {
  const auto outcome = tsv::load_csv_series(
    fixture_path("sample_numeric.csv"),
    tsv::SeriesRequest{std::nullopt, "time", "pressure"}
  );
  REQUIRE(outcome.ok);
  REQUIRE(outcome.series.time_column == "time");
  REQUIRE(outcome.series.value_column == "pressure");
  REQUIRE(outcome.series.time.size() == 3);
  REQUIRE(outcome.series.value[0] == Catch::Approx(101.0));
}

TEST_CASE("SQLite import detects tables and columns", "[sqlite]") {
  const auto temp_db = make_sqlite_fixture(make_temp_dir() / "fixture.sqlite");
  const auto catalog = tsv::load_sqlite_catalog(temp_db);
  REQUIRE(catalog.kind == tsv::SourceKind::Sqlite);
  REQUIRE(catalog.tables.size() == 2);
  REQUIRE(catalog.tables[0].name == "metadata");
  REQUIRE(catalog.tables[1].name == "telemetry");
  REQUIRE(catalog.tables[1].columns.size() >= 3);
  REQUIRE(catalog.tables[1].columns[0].time_candidate);
}

TEST_CASE("SQLite import supports explicit time-column override", "[sqlite]") {
  const auto temp_db = make_sqlite_fixture(make_temp_dir() / "override.sqlite");
  const auto outcome = tsv::load_sqlite_series(
    temp_db,
    "telemetry",
    tsv::SeriesRequest{std::nullopt, "timestamp", "altitude"}
  );
  REQUIRE(outcome.ok);
  REQUIRE(outcome.series.name == "override.telemetry.altitude");
  REQUIRE(outcome.series.time.size() == 3);
  REQUIRE(outcome.series.value[2] == Catch::Approx(102.0));
}

TEST_CASE("Nested dotted variable names build a tree", "[tree]") {
  const auto tree = tsv::build_variable_tree({
    "aircraft.engine.rpm",
    "aircraft.engine.temperature",
    "aircraft.controls.aileron"
  });
  REQUIRE(tree.children.size() == 1);
  REQUIRE(tree.children.front().label == "aircraft");
  REQUIRE(tree.children.front().children.size() == 2);
  REQUIRE(tree.children.front().children.front().label == "controls");
}

TEST_CASE("Interpolation aligns different time grids on the left-hand series", "[interp]") {
  tsv::SeriesData left;
  left.name = "left";
  left.time = {0.0, 1.0, 2.0};
  left.value = {10.0, 20.0, 30.0};

  tsv::SeriesData right;
  right.name = "right";
  right.time = {0.0, 2.0};
  right.value = {1.0, 5.0};

  const auto result = tsv::series_sub(left, right);
  REQUIRE(result.time == left.time);
  REQUIRE(result.value[0] == Catch::Approx(9.0));
  REQUIRE(result.value[1] == Catch::Approx(17.0));
  REQUIRE(result.value[2] == Catch::Approx(25.0));
}

TEST_CASE("Lua expression evaluation supports arithmetic over series", "[expr]") {
  tsv::SeriesRegistry registry;

  tsv::SeriesData a;
  a.name = "run1.speed";
  a.time = {0.0, 1.0, 2.0};
  a.value = {10.0, 11.0, 12.0};
  registry.set(a);

  tsv::SeriesData b;
  b.name = "run2.speed";
  b.time = {0.0, 2.0};
  b.value = {2.0, 6.0};
  registry.set(b);

  const auto result = tsv::evaluate_expression("series('run1.speed') - series('run2.speed') + 5", registry);
  REQUIRE(result.time == a.time);
  REQUIRE(result.value[0] == Catch::Approx(13.0));
  REQUIRE(result.value[1] == Catch::Approx(12.0));
  REQUIRE(result.value[2] == Catch::Approx(11.0));
}

TEST_CASE("Project JSON round-trips", "[project]") {
  tsv::ProjectState state;
  state.sources.push_back(tsv::ProjectSource{
    tsv::SourceKind::Csv,
    "/tmp/source.csv",
    "source",
    std::nullopt,
    std::optional<std::string>{"time"},
    {"source.speed", "source.altitude"}
  });
  state.workspace.windows.push_back(tsv::AnalysisWindowConfig{
    "window-1",
    {
      tsv::PlotTabConfig{
        "main",
        {tsv::PlotSeriesConfig{
          "source.speed",
          "",
          std::optional<std::string>{"source"},
          std::optional<std::string>{"/tmp/source.csv"},
          std::optional<std::string>{},
          std::optional<std::string>{"time"},
          std::optional<std::string>{"speed"},
          true,
          false,
          {1.0, 0.0, 0.0, 1.0}
        }},
        true,
        true,
        std::nullopt,
        std::nullopt
      }
    },
    0
  });

  const auto temp = make_temp_dir() / "project.json";
  tsv::save_project(temp, state);
  const auto loaded = tsv::load_project(temp);
  REQUIRE(loaded == state);
}

TEST_CASE("Workspace layout round-trips across multiple windows and tabs", "[project]") {
  tsv::ProjectState state;
  state.sources.push_back(tsv::ProjectSource{
    tsv::SourceKind::Csv,
    "/tmp/run_a.csv",
    "run_a",
    std::nullopt,
    std::nullopt,
    {"run_a.speed", "run_a.altitude"}
  });
  state.sources.push_back(tsv::ProjectSource{
    tsv::SourceKind::Sqlite,
    "/tmp/run_b.sqlite",
    "run_b",
    std::optional<std::string>{"telemetry"},
    std::optional<std::string>{"timestamp"},
    {"run_b.telemetry.speed"}
  });

  state.workspace.windows.push_back(tsv::AnalysisWindowConfig{
    "window-a",
    {
      tsv::PlotTabConfig{
        "tab-1",
        {
          tsv::PlotSeriesConfig{
            "run_a.speed",
            "",
            std::optional<std::string>{"run_a"},
            std::optional<std::string>{"/tmp/run_a.csv"},
            std::optional<std::string>{},
            std::optional<std::string>{"time"},
            std::optional<std::string>{"speed"},
            true,
            false,
            {1.0, 0.0, 0.0, 1.0}
          }
        },
        true,
        true,
        std::nullopt,
        std::nullopt
      },
      tsv::PlotTabConfig{
        "tab-2",
        {
          tsv::PlotSeriesConfig{
            "run_a.altitude",
            "",
            std::optional<std::string>{"run_a"},
            std::optional<std::string>{"/tmp/run_a.csv"},
            std::optional<std::string>{},
            std::optional<std::string>{"time"},
            std::optional<std::string>{"altitude"},
            true,
            false,
            {0.0, 1.0, 0.0, 1.0}
          }
        },
        true,
        true,
        std::nullopt,
        std::nullopt
      }
    },
    1
  });
  state.workspace.windows.push_back(tsv::AnalysisWindowConfig{
    "window-b",
    {
      tsv::PlotTabConfig{
        "comparison",
        {
          tsv::PlotSeriesConfig{
            "run_b.telemetry.speed",
            "",
            std::optional<std::string>{"run_b"},
            std::optional<std::string>{"/tmp/run_b.sqlite"},
            std::optional<std::string>{"telemetry"},
            std::optional<std::string>{"timestamp"},
            std::optional<std::string>{"speed"},
            true,
            false,
            {0.0, 0.0, 1.0, 1.0}
          },
          tsv::PlotSeriesConfig{
            "run_a.speed_minus_run_b.telemetry.speed",
            "series('run_a.speed') - series('run_b.telemetry.speed')",
            std::nullopt,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            true,
            true,
            {1.0, 1.0, 0.0, 1.0}
          }
        },
        true,
        true,
        std::nullopt,
        std::nullopt
      }
    },
    0
  });

  const auto temp = make_temp_dir() / "workspace.json";
  tsv::save_project(temp, state);
  const auto loaded = tsv::load_project(temp);
  REQUIRE(loaded == state);
  REQUIRE(loaded.workspace.windows.size() == 2);
  REQUIRE(loaded.workspace.windows[0].tabs.size() == 2);
  REQUIRE(loaded.workspace.windows[1].tabs.size() == 1);
  REQUIRE(loaded.workspace.windows[1].tabs[0].series.size() == 2);
}

TEST_CASE("Legacy project views load into a workspace", "[project]") {
  const auto temp = make_temp_dir() / "legacy_project.json";
  const nlohmann::json legacy = {
    {"sources", {
      {
        {"kind", "csv"},
        {"path", "/tmp/source.csv"},
        {"alias", "source"},
        {"selected_variables", {"source.speed"}}
      }
    }},
    {"views", {
      {
        {"title", "main"},
        {"series", {
          {
            {"name", "source.speed"},
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
    std::ofstream out(temp);
    out << legacy.dump(2) << '\n';
  }

  const auto loaded = tsv::load_project(temp);
  REQUIRE(loaded.workspace.windows.size() == 1);
  REQUIRE(loaded.workspace.windows.front().tabs.size() == 1);
  REQUIRE(loaded.workspace.windows.front().tabs.front().title == "main");
  REQUIRE(loaded.workspace.windows.front().tabs.front().series.size() == 1);
  REQUIRE(loaded.workspace.windows.front().tabs.front().series.front().name == "source.speed");
}

TEST_CASE("Missing variables are reported after reload", "[reload]") {
  const auto temp_dir = make_temp_dir();
  const auto source = temp_dir / "reload.csv";
  {
    std::ofstream out(source);
    out << "time,speed,pressure\n";
    out << "0,10,100\n";
    out << "1,11,101\n";
  }

  const auto initial = tsv::load_csv_series(source, tsv::SeriesRequest{std::nullopt, "time", "pressure"});
  REQUIRE(initial.ok);

  {
    std::ofstream out(source);
    out << "time,speed\n";
    out << "0,10\n";
    out << "1,11\n";
  }

  const auto reloaded = tsv::load_csv_series(source, tsv::SeriesRequest{std::nullopt, "time", "pressure"});
  REQUIRE_FALSE(reloaded.ok);
  REQUIRE(reloaded.error.find("Value column not found") != std::string::npos);
}
