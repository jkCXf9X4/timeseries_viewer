#include <chrono>
#include <algorithm>
#include <array>
#include <cctype>
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

struct OpenSource {
  tsv::SourceCatalog catalog;
  std::string alias;
  std::optional<fs::file_time_type> last_write_time;
};

struct AppState {
  std::vector<OpenSource> sources;
  tsv::WorkspaceConfig workspace;
  std::unordered_map<std::string, tsv::SeriesData> series_cache;
  std::unordered_map<std::string, std::string> series_errors;
  std::array<char, 1024> expression_buffer{};
  std::string status{"Ready"};
  bool live_mode{false};
  std::chrono::steady_clock::time_point last_poll{std::chrono::steady_clock::now()};
  fs::path project_path;
  int active_window{0};
};

std::string sanitize_identifier(std::string value) {
  for (char& ch : value) {
    const auto u = static_cast<unsigned char>(ch);
    if (!std::isalnum(u) && ch != '_') {
      ch = '_';
    }
  }
  while (value.find("__") != std::string::npos) {
    value.erase(value.find("__"), 1);
  }
  if (value.empty()) {
    value = "source";
  }
  return value;
}

std::string unique_alias(const std::vector<OpenSource>& sources, const std::string& preferred) {
  const auto base = sanitize_identifier(preferred);
  std::string alias = base;
  int suffix = 2;
  const auto exists = [&](const std::string& candidate) {
    return std::any_of(sources.begin(), sources.end(), [&](const OpenSource& src) {
      return src.alias == candidate;
    });
  };
  while (exists(alias)) {
    alias = base + "_" + std::to_string(suffix++);
  }
  return alias;
}

void ensure_workspace_defaults(AppState& app) {
  if (app.workspace.windows.empty()) {
    tsv::AnalysisWindowConfig window;
    window.title = "Window 1";
    window.tabs.push_back(tsv::PlotTabConfig{});
    app.workspace.windows.push_back(std::move(window));
  }
  for (std::size_t index = 0; index < app.workspace.windows.size(); ++index) {
    auto& window = app.workspace.windows[index];
    if (window.title.empty()) {
      window.title = "Window " + std::to_string(index + 1);
    }
    if (window.tabs.empty()) {
      window.tabs.push_back(tsv::PlotTabConfig{});
    }
    if (window.active_tab >= window.tabs.size()) {
      window.active_tab = 0;
    }
    for (std::size_t tab_index = 0; tab_index < window.tabs.size(); ++tab_index) {
      auto& tab = window.tabs[tab_index];
      if (tab.title.empty()) {
        tab.title = "Plot " + std::to_string(tab_index + 1);
      }
    }
  }
  if (app.active_window < 0 || app.active_window >= static_cast<int>(app.workspace.windows.size())) {
    app.active_window = 0;
  }
}

tsv::PlotTabConfig& active_tab(AppState& app) {
  ensure_workspace_defaults(app);
  auto& window = app.workspace.windows.at(static_cast<std::size_t>(app.active_window));
  return window.tabs.at(window.active_tab);
}

const tsv::PlotTabConfig& active_tab(const AppState& app) {
  return app.workspace.windows.at(static_cast<std::size_t>(app.active_window)).tabs.at(app.workspace.windows.at(static_cast<std::size_t>(app.active_window)).active_tab);
}

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

std::string make_series_name(const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column) {
  if (table_name.has_value()) {
    return source.alias + "." + *table_name + "." + value_column;
  }
  return source.alias + "." + value_column;
}

const OpenSource* find_source_by_path(const AppState& app, const std::filesystem::path& path) {
  const auto it = std::find_if(app.sources.begin(), app.sources.end(), [&](const OpenSource& source) {
    return source.catalog.path == path;
  });
  return it == app.sources.end() ? nullptr : &*it;
}

const OpenSource* find_source_by_alias(const AppState& app, const std::string& alias) {
  const auto it = std::find_if(app.sources.begin(), app.sources.end(), [&](const OpenSource& source) {
    return source.alias == alias;
  });
  return it == app.sources.end() ? nullptr : &*it;
}

std::optional<tsv::SeriesData> load_raw_series(
  const OpenSource& source,
  const std::optional<std::string>& table_name,
  const std::optional<std::string>& time_column_override,
  const std::string& value_column,
  std::string& error,
  std::string& time_column_out
) {
  const auto table = find_table(source.catalog, table_name);
  if (!table.has_value()) {
    error = "Table not found";
    return std::nullopt;
  }

  tsv::SeriesRequest request;
  request.table_name = table_name;
  request.time_column = time_column_override.value_or(default_time_column(*table));
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
  outcome.series.name = make_series_name(source, table_name, value_column);
  return outcome.series;
}

void open_source(AppState& app, const fs::path& path, const std::optional<std::string>& alias_override, const std::optional<tsv::SourceKind>& kind_override);

void open_source(AppState& app, const fs::path& path) {
  open_source(app, path, std::nullopt, std::nullopt);
}

void open_source(AppState& app, const fs::path& path, const std::optional<std::string>& alias_override, const std::optional<tsv::SourceKind>& kind_override) {
  try {
    OpenSource source;
    const auto kind = kind_override.has_value()
      ? *kind_override
      : (path.extension() == ".csv" ? tsv::SourceKind::Csv : tsv::SourceKind::Sqlite);
    source.catalog = kind == tsv::SourceKind::Csv ? tsv::load_csv_catalog(path) : tsv::load_sqlite_catalog(path);
    source.alias = alias_override.has_value() ? *alias_override : unique_alias(app.sources, path.stem().string());
    source.catalog.source_name = source.alias;
    source.last_write_time = file_timestamp(path);
    app.sources.push_back(std::move(source));
    app.status = "Opened " + path.filename().string();
  } catch (const std::exception& ex) {
    app.status = ex.what();
  }
}

void rebuild_cache(AppState& app) {
  ensure_workspace_defaults(app);
  app.series_cache.clear();
  app.series_errors.clear();

  tsv::SeriesRegistry registry;

  for (auto& window : app.workspace.windows) {
    for (auto& tab : window.tabs) {
      for (auto& series_cfg : tab.series) {
        if (series_cfg.derived) {
          continue;
        }

        const auto source_path = series_cfg.source_path.has_value() ? fs::path(*series_cfg.source_path) : fs::path{};
        const OpenSource* source = nullptr;
        if (!source_path.empty()) {
          source = find_source_by_path(app, source_path);
        }
        if (source == nullptr && series_cfg.source_alias.has_value()) {
          source = find_source_by_alias(app, *series_cfg.source_alias);
        }
        if (source == nullptr) {
          app.series_errors[series_cfg.name] = "Source missing";
          continue;
        }
        if (series_cfg.value_column.has_value() == false) {
          app.series_errors[series_cfg.name] = "Value column missing";
          continue;
        }

        std::string error;
        std::string time_column;
        const auto series = load_raw_series(
          *source,
          series_cfg.table_name,
          series_cfg.time_column,
          *series_cfg.value_column,
          error,
          time_column
        );
        if (!series.has_value()) {
          app.series_errors[series_cfg.name] = error;
          continue;
        }

        if (series_cfg.name.empty()) {
          series_cfg.name = series->name;
        }
        if (!series_cfg.source_alias.has_value()) {
          series_cfg.source_alias = source->alias;
        }
        if (!series_cfg.source_path.has_value()) {
          series_cfg.source_path = source->catalog.path.string();
        }
        if (!series_cfg.time_column.has_value()) {
          series_cfg.time_column = time_column;
        }

        auto loaded = *series;
        loaded.name = series_cfg.name;
        loaded.source_name = source->alias;
        app.series_cache[loaded.name] = loaded;
        registry.set(loaded);
      }
    }
  }

  for (auto& window : app.workspace.windows) {
    for (auto& tab : window.tabs) {
      for (auto& series_cfg : tab.series) {
        if (!series_cfg.derived) {
          continue;
        }

        if (series_cfg.name.empty()) {
          series_cfg.name = series_cfg.expression.empty() ? "derived" : series_cfg.expression;
        }

        try {
          auto series = tsv::evaluate_expression(series_cfg.expression, registry);
          series.name = series_cfg.name;
          app.series_cache[series.name] = std::move(series);
          registry.set(app.series_cache.at(series_cfg.name));
        } catch (const std::exception& ex) {
          app.series_errors[series_cfg.name] = ex.what();
        }
      }
    }
  }
}

void add_raw_series(AppState& app, const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column) {
  tsv::PlotSeriesConfig series_cfg;
  series_cfg.source_alias = source.alias;
  series_cfg.source_path = source.catalog.path.string();
  series_cfg.table_name = table_name;
  series_cfg.value_column = value_column;

  std::string error;
  std::string time_column;
  const auto preview = load_raw_series(source, table_name, std::nullopt, value_column, error, time_column);
  if (!preview.has_value()) {
    app.status = error;
    return;
  }

  series_cfg.name = preview->name;
  series_cfg.time_column = time_column;

  active_tab(app).series.push_back(std::move(series_cfg));
  app.status = "Added " + preview->name;
  rebuild_cache(app);
}

void add_derived_series(AppState& app) {
  const std::string expression(app.expression_buffer.data());
  if (expression.empty()) {
    app.status = "Expression is empty";
    return;
  }

  tsv::SeriesRegistry registry;
  for (const auto& [name, series] : app.series_cache) {
    registry.set(series);
  }

  try {
    auto series = tsv::evaluate_expression(expression, registry);
    auto& tab = active_tab(app);
    tsv::PlotSeriesConfig series_cfg;
    series_cfg.name = series.name;
    series_cfg.expression = expression;
    series_cfg.derived = true;
    tab.series.push_back(std::move(series_cfg));
    app.series_cache[series.name] = std::move(series);
    app.status = "Added derived series";
    rebuild_cache(app);
  } catch (const std::exception& ex) {
    app.status = ex.what();
  }
}

void save_project_file(AppState& app, const fs::path& path) {
  tsv::ProjectState project;
  std::map<std::string, tsv::ProjectSource> entries;
  for (const auto& source : app.sources) {
    tsv::ProjectSource entry;
    entry.kind = source.catalog.kind;
    entry.path = source.catalog.path.string();
    entry.alias = source.alias;
    entries.emplace(entry.path, std::move(entry));
  }

  for (const auto& window : app.workspace.windows) {
    for (const auto& tab : window.tabs) {
      for (const auto& series : tab.series) {
        if (series.derived) {
          continue;
        }
        if (!series.source_path.has_value()) {
          continue;
        }
        auto& entry = entries[*series.source_path];
        entry.selected_variables.push_back(series.name);
        if (!entry.table_name.has_value() && series.table_name.has_value()) {
          entry.table_name = series.table_name;
        }
        if (!entry.time_column.has_value() && series.time_column.has_value()) {
          entry.time_column = series.time_column;
        }
      }
    }
  }

  for (auto& [_, entry] : entries) {
    std::sort(entry.selected_variables.begin(), entry.selected_variables.end());
    entry.selected_variables.erase(std::unique(entry.selected_variables.begin(), entry.selected_variables.end()), entry.selected_variables.end());
    project.sources.push_back(std::move(entry));
  }

  project.workspace = app.workspace;
  tsv::save_project(path, project);
  app.project_path = path;
  app.status = "Saved project";
}

void load_project_file(AppState& app, const fs::path& path) {
  try {
    const auto project = tsv::load_project(path);
    app.sources.clear();
    app.series_cache.clear();
    app.series_errors.clear();
    app.workspace = project.workspace;

    for (const auto& source : project.sources) {
      const auto source_path = fs::path(source.path);
      open_source(app, source_path, source.alias.empty() ? std::optional<std::string>{} : std::optional<std::string>{source.alias}, source.kind);
    }
    app.project_path = path;
    app.status = "Loaded project";
    ensure_workspace_defaults(app);
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

void render_parameter_panel(AppState& app) {
  ensure_workspace_defaults(app);
  ImGui::Begin("Parameters");
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
  if (ImGui::Button("New window")) {
    tsv::AnalysisWindowConfig window;
    window.title = "Window " + std::to_string(app.workspace.windows.size() + 1);
    window.tabs.push_back(tsv::PlotTabConfig{});
    app.workspace.windows.push_back(std::move(window));
    app.active_window = static_cast<int>(app.workspace.windows.size()) - 1;
  }
  ImGui::SameLine();
  if (ImGui::Button("New tab")) {
    auto& window = app.workspace.windows.at(static_cast<std::size_t>(app.active_window));
    window.tabs.push_back(tsv::PlotTabConfig{});
    window.active_tab = window.tabs.size() - 1;
    window.tabs.back().title = "Plot " + std::to_string(window.tabs.size());
  }

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
  const auto& window = app.workspace.windows.at(static_cast<std::size_t>(app.active_window));
  ImGui::Text("Target window: %s", window.title.c_str());
  ImGui::Text("Target tab: %s", window.tabs.at(window.active_tab).title.c_str());
  ImGui::InputText("Expression", app.expression_buffer.data(), app.expression_buffer.size());
  ImGui::SameLine();
  if (ImGui::Button("Add derived")) {
    add_derived_series(app);
  }
  ImGui::Separator();

  for (const auto& source : app.sources) {
    if (ImGui::TreeNode(source.alias.c_str())) {
      ImGui::TextDisabled("%s", source.catalog.path.string().c_str());
      for (const auto& table : source.catalog.tables) {
        const bool has_nested_table = source.catalog.kind == tsv::SourceKind::Sqlite || source.catalog.tables.size() > 1;
        if (has_nested_table) {
          if (ImGui::TreeNode(table.name.c_str())) {
            for (const auto& column : table.columns) {
              if (!column.time_candidate && ImGui::Selectable(column.name.c_str())) {
                add_raw_series(app, source, source.catalog.kind == tsv::SourceKind::Sqlite ? std::optional<std::string>{table.name} : std::nullopt, column.name);
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

  if (!app.series_errors.empty()) {
    ImGui::Separator();
    ImGui::TextUnformatted("Series errors");
    for (const auto& [name, error] : app.series_errors) {
      ImGui::BulletText("%s: %s", name.c_str(), error.c_str());
    }
  }

  ImGui::TextWrapped("%s", app.status.c_str());
  ImGui::End();
}

void render_analysis_windows(AppState& app) {
  ensure_workspace_defaults(app);
  for (std::size_t window_index = 0; window_index < app.workspace.windows.size(); ++window_index) {
    auto& window = app.workspace.windows[window_index];
    if (ImGui::Begin(window.title.c_str())) {
      if (ImGui::Button("New tab")) {
        window.tabs.push_back(tsv::PlotTabConfig{});
        window.tabs.back().title = "Plot " + std::to_string(window.tabs.size());
        window.active_tab = window.tabs.size() - 1;
      }
      ImGui::SameLine();
      ImGui::TextDisabled("Window %zu", window_index + 1);

      const auto tab_bar_id = "tabs##" + std::to_string(window_index);
      if (ImGui::BeginTabBar(tab_bar_id.c_str())) {
        for (std::size_t tab_index = 0; tab_index < window.tabs.size(); ++tab_index) {
          auto& tab = window.tabs[tab_index];
          if (ImGui::BeginTabItem(tab.title.c_str())) {
            window.active_tab = tab_index;
            app.active_window = static_cast<int>(window_index);

            if (ImPlot::BeginPlot(("plot##" + std::to_string(window_index) + "_" + std::to_string(tab_index)).c_str(), ImVec2(-1, 360))) {
              for (const auto& series_cfg : tab.series) {
                const auto it = app.series_cache.find(series_cfg.name);
                if (it == app.series_cache.end()) {
                  continue;
                }
                const auto& series = it->second;
                if (series_cfg.visible && !series.time.empty() && !series.value.empty()) {
                  ImPlot::PlotLine(series_cfg.name.c_str(), series.time.data(), series.value.data(), static_cast<int>(series.time.size()));
                }
              }
              ImPlot::EndPlot();
            }

            ImGui::Separator();
            for (const auto& series_cfg : tab.series) {
              if (series_cfg.derived) {
                ImGui::BulletText("%s = %s", series_cfg.name.c_str(), series_cfg.expression.c_str());
              } else {
                const auto source_label = series_cfg.source_alias.value_or(std::string{"?"});
                const auto table_label = series_cfg.table_name.value_or(std::string{});
                const auto value_label = series_cfg.value_column.value_or(std::string{});
                if (table_label.empty()) {
                  ImGui::BulletText("%s -> %s.%s", series_cfg.name.c_str(), source_label.c_str(), value_label.c_str());
                } else {
                  ImGui::BulletText("%s -> %s.%s.%s", series_cfg.name.c_str(), source_label.c_str(), table_label.c_str(), value_label.c_str());
                }
              }
              const auto error_it = app.series_errors.find(series_cfg.name);
              if (error_it != app.series_errors.end()) {
                ImGui::Indent();
                ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "%s", error_it->second.c_str());
                ImGui::Unindent();
              }
            }

            ImGui::EndTabItem();
          }
        }
        ImGui::EndTabBar();
      }
    }
    ImGui::End();
  }
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

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  AppState app;
  ensure_workspace_defaults(app);
  std::snprintf(app.expression_buffer.data(), app.expression_buffer.size(), "%s", "series(\"source.variable\")");

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    poll_live_reload(app);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    render_parameter_panel(app);
    render_analysis_windows(app);

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
