#include <chrono>
#include <algorithm>
#include <array>
#include <filesystem>
#include <cstring>
#include <map>
#include <optional>
#include <string>
#include <set>
#include <unordered_map>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <implot.h>
#include <nfd.h>

#include "timeseries_viewer/core.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace fs = std::filesystem;

namespace {

struct RawSeriesDef {
  tsv::SourceKind kind{tsv::SourceKind::Csv};
  fs::path path;
  std::optional<std::string> table_name;
  std::string time_column;
  std::string value_column;
  std::string series_name;
  std::string error;
  bool available{false};
};

struct DerivedSeriesDef {
  std::string expression;
  std::string series_name;
  std::string error;
  bool available{false};
};

struct PlotView {
  std::string title{"Plot 1"};
  std::vector<std::string> series_names;
};

struct SourceKey {
  fs::path path;
  std::optional<std::string> table_name;

  bool operator<(const SourceKey& other) const {
    if (path != other.path) {
      return path < other.path;
    }
    return table_name < other.table_name;
  }
};

struct OpenSource {
  tsv::SourceCatalog catalog;
  std::optional<fs::file_time_type> last_write_time;
};

struct AppState {
  std::vector<OpenSource> sources;
  std::vector<RawSeriesDef> raw_series;
  std::vector<DerivedSeriesDef> derived_series;
  std::vector<PlotView> views{PlotView{}};
  int active_view{0};
  std::unordered_map<std::string, tsv::SeriesData> series_cache;
  std::array<char, 1024> expression_buffer{};
  std::string status{"Ready"};
  bool live_mode{false};
  std::chrono::steady_clock::time_point last_poll{std::chrono::steady_clock::now()};
  fs::path project_path;
};

std::optional<fs::path> open_dialog() {
  nfdu8char_t* out_path = nullptr;
  const nfdu8filteritem_t filters[] = {
    {"Time series", "csv,sqlite,db"}
  };
  const auto result = NFD_OpenDialogU8(&out_path, filters, 1, nullptr);
  if (result != NFD_OKAY) {
    return std::nullopt;
  }
  fs::path path(reinterpret_cast<const char*>(out_path));
  NFD_FreePathU8(out_path);
  return path;
}

std::optional<fs::path> save_dialog() {
  nfdu8char_t* out_path = nullptr;
  const nfdu8filteritem_t filters[] = {
    {"Project", "json"}
  };
  const auto result = NFD_SaveDialogU8(&out_path, filters, 1, nullptr, nullptr);
  if (result != NFD_OKAY) {
    return std::nullopt;
  }
  fs::path path(reinterpret_cast<const char*>(out_path));
  NFD_FreePathU8(out_path);
  return path;
}

std::optional<fs::path> open_project_dialog() {
  nfdu8char_t* out_path = nullptr;
  const nfdu8filteritem_t filters[] = {
    {"Project", "json"}
  };
  const auto result = NFD_OpenDialogU8(&out_path, filters, 1, nullptr);
  if (result != NFD_OKAY) {
    return std::nullopt;
  }
  fs::path path(reinterpret_cast<const char*>(out_path));
  NFD_FreePathU8(out_path);
  return path;
}

std::optional<fs::file_time_type> file_timestamp(const fs::path& path) {
  std::error_code ec;
  const auto stamp = fs::last_write_time(path, ec);
  if (ec) {
    return std::nullopt;
  }
  return stamp;
}

std::optional<tsv::TableCatalog> find_table(const tsv::SourceCatalog& catalog, const std::optional<std::string>& table_name) {
  if (catalog.tables.empty()) {
    return std::nullopt;
  }
  if (!table_name.has_value()) {
    return catalog.tables.front();
  }
  for (const auto& table : catalog.tables) {
    if (table.name == *table_name) {
      return table;
    }
  }
  return std::nullopt;
}

std::string default_time_column(const tsv::TableCatalog& table) {
  if (const auto inferred = tsv::infer_time_column(table.columns); inferred.has_value()) {
    return *inferred;
  }
  return {};
}

std::optional<tsv::SeriesData> load_raw_series(const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column, std::string& error, std::string& time_column_out) {
  const auto table = find_table(source.catalog, table_name);
  if (!table.has_value()) {
    error = "Table not found";
    return std::nullopt;
  }

  tsv::SeriesRequest request;
  request.table_name = table_name;
  request.time_column = default_time_column(*table);
  request.value_column = value_column;
  time_column_out = request.time_column;

  tsv::LoadOutcome outcome;
  if (source.catalog.kind == tsv::SourceKind::Csv) {
    outcome = tsv::load_csv_series(source.catalog.path, request);
  } else {
    outcome = tsv::load_sqlite_series(source.catalog.path, table->name, request);
  }

  if (!outcome.ok) {
    error = outcome.error;
    return std::nullopt;
  }
  return outcome.series;
}

void rebuild_cache(AppState& app) {
  app.series_cache.clear();
  tsv::SeriesRegistry registry;

  for (auto& raw : app.raw_series) {
    raw.available = false;
    raw.error.clear();
    const auto source_it = std::find_if(app.sources.begin(), app.sources.end(), [&](const OpenSource& src) {
      return src.catalog.path == raw.path;
    });
    if (source_it == app.sources.end()) {
      raw.error = "Source missing";
      continue;
    }

    std::string load_error;
    std::string time_column;
    const auto series = load_raw_series(*source_it, raw.table_name, raw.value_column, load_error, time_column);
    if (!series.has_value()) {
      raw.error = load_error;
      continue;
    }

    raw.time_column = time_column;
    raw.series_name = series->name;
    raw.available = true;
    app.series_cache[raw.series_name] = *series;
    registry.set(*series);
  }

  for (auto& derived : app.derived_series) {
    derived.available = false;
    derived.error.clear();
    try {
      auto series = tsv::evaluate_expression(derived.expression, registry);
      series.name = derived.series_name.empty() ? derived.expression : derived.series_name;
      derived.series_name = series.name;
      derived.available = true;
      app.series_cache[series.name] = std::move(series);
      registry.set(app.series_cache.at(derived.series_name));
    } catch (const std::exception& ex) {
      derived.error = ex.what();
    }
  }
}

void add_raw_series(AppState& app, const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column) {
  std::string error;
  std::string time_column;
  const auto series = load_raw_series(source, table_name, value_column, error, time_column);
  if (!series.has_value()) {
    app.status = error;
    return;
  }

  RawSeriesDef def;
  def.kind = source.catalog.kind;
  def.path = source.catalog.path;
  def.table_name = table_name;
  def.time_column = time_column;
  def.value_column = value_column;
  def.series_name = series->name;
  def.available = true;
  app.raw_series.push_back(def);
  app.series_cache[def.series_name] = *series;

  if (app.active_view < 0 || app.active_view >= static_cast<int>(app.views.size())) {
    app.views.push_back(PlotView{});
    app.active_view = static_cast<int>(app.views.size()) - 1;
  }
  app.views[app.active_view].series_names.push_back(def.series_name);
  app.status = "Loaded " + def.series_name;
}

void add_derived_series(AppState& app) {
  tsv::SeriesRegistry registry;
  for (const auto& [name, series] : app.series_cache) {
    if (std::find_if(app.raw_series.begin(), app.raw_series.end(), [&](const RawSeriesDef& raw) {
          return raw.available && raw.series_name == name;
        }) != app.raw_series.end()) {
      registry.set(series);
    }
  }

  try {
    const std::string expression(app.expression_buffer.data());
    auto series = tsv::evaluate_expression(expression, registry);
    DerivedSeriesDef def;
    def.expression = expression;
    def.series_name = series.name;
    def.available = true;
    app.derived_series.push_back(def);
    app.series_cache[series.name] = std::move(series);
    if (app.active_view < 0 || app.active_view >= static_cast<int>(app.views.size())) {
      app.views.push_back(PlotView{});
      app.active_view = static_cast<int>(app.views.size()) - 1;
    }
    app.views[app.active_view].series_names.push_back(def.series_name);
    app.status = "Added derived series";
  } catch (const std::exception& ex) {
    app.status = ex.what();
  }
}

void open_source(AppState& app, const fs::path& path) {
  try {
    OpenSource source;
    source.catalog = path.extension() == ".csv" ? tsv::load_csv_catalog(path) : tsv::load_sqlite_catalog(path);
    source.last_write_time = file_timestamp(path);
    app.sources.push_back(std::move(source));
    app.status = "Opened " + path.filename().string();
  } catch (const std::exception& ex) {
    app.status = ex.what();
  }
}

void save_project_file(AppState& app, const fs::path& path) {
  tsv::ProjectState project;
  std::map<SourceKey, tsv::ProjectSource> entries;
  for (const auto& source : app.sources) {
    for (const auto& table : source.catalog.tables) {
      SourceKey key{source.catalog.path, source.catalog.kind == tsv::SourceKind::Sqlite ? std::optional<std::string>{table.name} : std::nullopt};
      auto& entry = entries[key];
      entry.kind = source.catalog.kind;
      entry.path = source.catalog.path.string();
      entry.table_name = key.table_name;
      if (!table.columns.empty()) {
        if (const auto inferred = tsv::infer_time_column(table.columns); inferred.has_value()) {
          entry.time_column = inferred;
        }
      }
    }
  }
  for (const auto& raw : app.raw_series) {
    SourceKey key{raw.path, raw.table_name};
    entries[key].selected_variables.push_back(raw.series_name);
  }
  for (auto& [_, entry] : entries) {
    project.sources.push_back(std::move(entry));
  }

  tsv::PlotViewConfig view;
  view.title = app.views.empty() ? "Plot 1" : app.views.front().title;
  for (const auto& name : app.views.front().series_names) {
    const auto derived = std::find_if(app.derived_series.begin(), app.derived_series.end(), [&](const DerivedSeriesDef& item) {
      return item.series_name == name;
    });
    tsv::PlotSeriesConfig series;
    series.name = name;
    if (derived != app.derived_series.end()) {
      series.derived = true;
      series.expression = derived->expression;
    }
    view.series.push_back(series);
  }
  project.views.push_back(std::move(view));
  tsv::save_project(path, project);
  app.project_path = path;
  app.status = "Saved project";
}

void load_project_file(AppState& app, const fs::path& path) {
  try {
    const auto project = tsv::load_project(path);
    app.sources.clear();
    app.raw_series.clear();
    app.derived_series.clear();
    app.series_cache.clear();
    app.views.clear();

    std::set<fs::path> opened_paths;
    for (const auto& source : project.sources) {
      fs::path source_path = source.path;
      if (opened_paths.insert(source_path).second) {
        open_source(app, source_path);
      }
      const auto source_it = std::find_if(app.sources.begin(), app.sources.end(), [&](const OpenSource& item) {
        return item.catalog.path == source_path;
      });
      if (source_it == app.sources.end()) {
        continue;
      }
      for (const auto& selected : source.selected_variables) {
        if (source.kind == tsv::SourceKind::Csv) {
          add_raw_series(app, *source_it, std::nullopt, selected.substr(selected.find_last_of('.') + 1));
        } else if (source.table_name.has_value()) {
          add_raw_series(app, *source_it, source.table_name, selected.substr(selected.find_last_of('.') + 1));
        }
      }
    }

    if (!project.views.empty()) {
      app.views.resize(project.views.size());
      for (std::size_t i = 0; i < project.views.size(); ++i) {
        app.views[i].title = project.views[i].title;
        for (const auto& series : project.views[i].series) {
          if (series.derived) {
            std::snprintf(app.expression_buffer.data(), app.expression_buffer.size(), "%s", series.expression.c_str());
            add_derived_series(app);
          }
          app.views[i].series_names.push_back(series.name);
        }
      }
    }
    app.project_path = path;
    app.status = "Loaded project";
    rebuild_cache(app);
  } catch (const std::exception& ex) {
    app.status = ex.what();
  }
}

void poll_live_reload(AppState& app) {
  if (!app.live_mode) {
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  if (now - app.last_poll < std::chrono::seconds(1)) {
    return;
  }
  app.last_poll = now;

  bool changed = false;
  for (auto& source : app.sources) {
    const auto current = file_timestamp(source.catalog.path);
    if (current.has_value() && source.last_write_time.has_value() && *current != *source.last_write_time) {
      source.last_write_time = current;
      changed = true;
    }
  }
  if (changed) {
    rebuild_cache(app);
    app.status = "Live data refreshed";
  }
}

void render_source_tree(AppState& app) {
  ImGui::Begin("Sources");
  if (ImGui::Button("Open")) {
    if (const auto path = open_dialog(); path.has_value()) {
      open_source(app, *path);
      rebuild_cache(app);
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Reload")) {
    rebuild_cache(app);
  }
  ImGui::SameLine();
  ImGui::Checkbox("Live", &app.live_mode);

  if (ImGui::Button("Open project")) {
    if (const auto path = open_project_dialog(); path.has_value()) {
      load_project_file(app, *path);
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Save project")) {
    if (const auto path = save_dialog(); path.has_value()) {
      save_project_file(app, *path);
    }
  }

  ImGui::Separator();

  for (const auto& source : app.sources) {
    if (ImGui::TreeNode(source.catalog.source_name.c_str())) {
      for (const auto& table : source.catalog.tables) {
        if (source.catalog.kind == tsv::SourceKind::Sqlite) {
          if (ImGui::TreeNode(table.name.c_str())) {
            for (const auto& column : table.columns) {
              if (!column.time_candidate && ImGui::Selectable(column.name.c_str())) {
                add_raw_series(app, source, table.name, column.name);
              }
            }
            ImGui::TreePop();
          }
        } else {
          for (const auto& column : table.columns) {
            if (!column.time_candidate && ImGui::Selectable(column.name.c_str())) {
              add_raw_series(app, source, std::nullopt, column.name);
            }
          }
        }
      }
      ImGui::TreePop();
    }
  }

  ImGui::TextWrapped("%s", app.status.c_str());
  ImGui::End();
}

void render_plot_workspace(AppState& app) {
  ImGui::Begin("Plots");
  if (ImGui::Button("New view")) {
    app.views.push_back(PlotView{});
    app.active_view = static_cast<int>(app.views.size()) - 1;
  }
  ImGui::SameLine();
  ImGui::InputText("Expression", app.expression_buffer.data(), app.expression_buffer.size());
  ImGui::SameLine();
  if (ImGui::Button("Add derived")) {
    add_derived_series(app);
  }

  if (ImGui::BeginTabBar("plot_views")) {
    for (std::size_t index = 0; index < app.views.size(); ++index) {
      if (ImGui::BeginTabItem(app.views[index].title.c_str())) {
        app.active_view = static_cast<int>(index);
        if (ImPlot::BeginPlot(("##plot" + std::to_string(index)).c_str(), ImVec2(-1, 400))) {
          for (const auto& name : app.views[index].series_names) {
            const auto it = app.series_cache.find(name);
            if (it == app.series_cache.end()) {
              continue;
            }
            const auto& series = it->second;
            if (!series.time.empty() && !series.value.empty()) {
              ImPlot::PlotLine(name.c_str(), series.time.data(), series.value.data(), static_cast<int>(series.time.size()));
            }
          }
          ImPlot::EndPlot();
        }
        ImGui::Separator();
        for (const auto& name : app.views[index].series_names) {
          ImGui::BulletText("%s", name.c_str());
        }
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
  }

  ImGui::End();
}

} // namespace

int main() {
  if (NFD_Init() != NFD_OKAY) {
    return 1;
  }

  if (!glfwInit()) {
    NFD_Quit();
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(1600, 900, "Time Series Viewer", nullptr, nullptr);
  if (window == nullptr) {
    glfwTerminate();
    NFD_Quit();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    glfwDestroyWindow(window);
    glfwTerminate();
    NFD_Quit();
    return 1;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();

  ImGuiIO& io = ImGui::GetIO();

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  AppState app;
  std::snprintf(app.expression_buffer.data(), app.expression_buffer.size(), "%s", "series(\"source.variable\")");

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    poll_live_reload(app);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    render_source_tree(app);
    render_plot_workspace(app);

    ImGui::Render();
    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
  NFD_Quit();
  return 0;
}
