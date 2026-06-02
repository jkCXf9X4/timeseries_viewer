#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <implot.h>
#include <nfd.h>

#include "timeseries_viewer/app_model.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace fs = std::filesystem;

namespace {

bool input_text_string(const char* label, std::string& value, std::size_t buffer_size = 1024) {
  std::vector<char> buffer(buffer_size, '\0');
  std::snprintf(buffer.data(), buffer.size(), "%s", value.c_str());
  if (ImGui::InputText(label, buffer.data(), buffer.size())) {
    value = buffer.data();
    return true;
  }
  return false;
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

const tsv::app::OpenSource* source_by_index(const tsv::app::AppState& app, std::size_t index) {
  if (index >= app.sources.size()) {
    return nullptr;
  }
  return &app.sources[index];
}

void render_series_editor(tsv::app::AppState& app, std::size_t window_index, std::size_t tab_index) {
  auto& tab = app.workspace.windows.at(window_index).tabs.at(tab_index);

  ImGui::SeparatorText("Plot Settings");
  input_text_string("Tab title", tab.title);
  ImGui::Checkbox("Autoscale X", &tab.autoscale_x);
  ImGui::SameLine();
  ImGui::Checkbox("Autoscale Y", &tab.autoscale_y);

  if (!tab.autoscale_x) {
    double range[2] = {0.0, 1.0};
    if (tab.x_range.has_value()) {
      range[0] = tab.x_range->at(0);
      range[1] = tab.x_range->at(1);
    }
    if (ImGui::InputScalarN("X range", ImGuiDataType_Double, range, 2)) {
      tab.x_range = std::array<double, 2>{range[0], range[1]};
    }
  }
  if (!tab.autoscale_y) {
    double range[2] = {0.0, 1.0};
    if (tab.y_range.has_value()) {
      range[0] = tab.y_range->at(0);
      range[1] = tab.y_range->at(1);
    }
    if (ImGui::InputScalarN("Y range", ImGuiDataType_Double, range, 2)) {
      tab.y_range = std::array<double, 2>{range[0], range[1]};
    }
  }

  ImGui::SeparatorText("Series");
  if (ImGui::BeginTable("series_table", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp)) {
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Visible");
    ImGui::TableSetupColumn("Color");
    ImGui::TableSetupColumn("Remove");
    ImGui::TableHeadersRow();

    for (std::size_t i = 0; i < tab.series.size(); ++i) {
      auto& series = tab.series[i];
      ImGui::PushID(static_cast<int>(i));
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      const bool selected = tab.active_series_index == i;
      if (ImGui::Selectable(series.name.c_str(), selected)) {
        tsv::app::select_series(app, window_index, tab_index, i);
      }
      ImGui::TableSetColumnIndex(1);
      ImGui::Checkbox("##visible", &series.visible);
      ImGui::TableSetColumnIndex(2);
      float color[4] = {
        static_cast<float>(series.color[0]),
        static_cast<float>(series.color[1]),
        static_cast<float>(series.color[2]),
        static_cast<float>(series.color[3])
      };
      if (ImGui::ColorEdit4("##color", color, ImGuiColorEditFlags_NoInputs)) {
        for (std::size_t j = 0; j < 4; ++j) {
          series.color[j] = color[j];
        }
      }
      ImGui::TableSetColumnIndex(3);
      if (ImGui::SmallButton("X")) {
        tsv::app::remove_series(app, window_index, tab_index, i);
        ImGui::PopID();
        break;
      }
      ImGui::PopID();
    }
    ImGui::EndTable();
  }

  if (tab.active_series_index < tab.series.size()) {
    auto& series = tab.series[tab.active_series_index];
    ImGui::SeparatorText("Selected Series");
    input_text_string("Series name", series.name);
    ImGui::Checkbox("Visible##selected", &series.visible);

    float color[4] = {
      static_cast<float>(series.color[0]),
      static_cast<float>(series.color[1]),
      static_cast<float>(series.color[2]),
      static_cast<float>(series.color[3])
    };
    if (ImGui::ColorEdit4("Series color", color, ImGuiColorEditFlags_NoInputs)) {
      for (std::size_t j = 0; j < 4; ++j) {
        series.color[j] = color[j];
      }
    }

    if (series.derived) {
      input_text_string("Expression", series.expression);
      if (ImGui::Button("Update derived")) {
        tsv::app::rebuild_cache(app);
      }
    } else {
      ImGui::Text("Source: %s", series.source_alias.value_or(std::string{"?"}).c_str());
      ImGui::Text("Binding: %s%s%s",
        series.table_name.has_value() ? series.table_name->c_str() : "",
        series.table_name.has_value() ? "." : "",
        series.value_column.has_value() ? series.value_column->c_str() : ""
      );
      if (series.time_column.has_value()) {
        ImGui::Text("Time: %s", series.time_column->c_str());
      }
    }

    if (ImGui::Button("Remove selected")) {
      tsv::app::remove_series(app, window_index, tab_index, tab.active_series_index);
    }
  }

  ImGui::SeparatorText("Derived Series");
  input_text_string("Expression draft", tab.expression_draft);
  if (ImGui::Button("Add derived")) {
    tsv::app::add_derived_series_to_tab(app, window_index, tab_index);
  }
}

void render_parameter_panel(tsv::app::AppState& app) {
  tsv::app::ensure_workspace_defaults(app);
  ImGui::Begin("Parameters");

  if (ImGui::Button("Open")) {
    if (const auto path = open_dialog(); path.has_value()) {
      tsv::app::open_source(app, *path);
      tsv::app::rebuild_cache(app);
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Reload")) {
    tsv::app::rebuild_cache(app);
  }
  ImGui::SameLine();
  ImGui::Checkbox("Live", &app.live_mode);

  if (ImGui::Button("Open project")) {
    if (const auto path = open_project_dialog(); path.has_value()) {
      tsv::app::load_project_file(app, *path);
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Save project")) {
    if (const auto path = save_dialog(); path.has_value()) {
      tsv::app::save_project_file(app, *path);
    }
  }

  ImGui::SameLine();
  if (ImGui::Button("New window")) {
    tsv::app::add_window(app);
  }

  ImGui::SameLine();
  if (ImGui::Button("New tab")) {
    tsv::app::add_tab(app, static_cast<std::size_t>(app.active_window));
  }

  ImGui::SeparatorText("Workspace");
  if (ImGui::BeginCombo("Active window", app.workspace.windows.at(static_cast<std::size_t>(app.active_window)).title.c_str())) {
    for (std::size_t i = 0; i < app.workspace.windows.size(); ++i) {
      const bool selected = static_cast<int>(i) == app.active_window;
      if (ImGui::Selectable(app.workspace.windows[i].title.c_str(), selected)) {
        app.active_window = static_cast<int>(i);
      }
      if (selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  unsigned long long budget = static_cast<unsigned long long>(app.workspace.point_budget);
  if (ImGui::InputScalar("Point budget", ImGuiDataType_U64, &budget)) {
    app.workspace.point_budget = static_cast<std::size_t>(budget);
    tsv::app::rebuild_cache(app);
  }
  ImGui::TextDisabled("0 means no explicit downsampling limit");

  auto& window = tsv::app::active_window(app);
  auto& tab = tsv::app::active_tab(app);
  ImGui::SeparatorText("Active Target");
  ImGui::Text("Window: %s", window.title.c_str());
  ImGui::Text("Tab: %s", tab.title.c_str());
  if (tab.active_series_index < tab.series.size()) {
    ImGui::Text("Series: %s", tab.series[tab.active_series_index].name.c_str());
  } else {
    ImGui::TextDisabled("Series: none");
  }

  ImGui::SeparatorText("Parameter Browser");
  if (app.sources.empty()) {
    ImGui::TextDisabled("Open a CSV file or SQLite database to browse parameters.");
  }

  for (const auto& source : app.sources) {
    if (ImGui::TreeNode(source.alias.c_str())) {
      ImGui::TextDisabled("%s", source.catalog.path.string().c_str());
      for (const auto& table : source.catalog.tables) {
        const bool nested = source.catalog.kind == tsv::SourceKind::Sqlite || source.catalog.tables.size() > 1;
        const auto table_label = nested ? table.name.c_str() : "";
        if (nested) {
          if (ImGui::TreeNode(table_label)) {
            for (const auto& column : table.columns) {
              if (column.time_candidate) {
                continue;
              }
              const auto selected_series_index = tab.active_series_index;
              const bool can_bind = selected_series_index < tab.series.size() && !tab.series[selected_series_index].derived;
              ImGui::PushID(column.name.c_str());
              if (ImGui::Selectable(column.name.c_str())) {
                tsv::app::add_raw_series(app, source, table.name, column.name);
              }
              if (can_bind) {
                ImGui::SameLine();
                if (ImGui::SmallButton("Bind selected")) {
                  tsv::app::bind_series_to_source(app, static_cast<std::size_t>(app.active_window), window.active_tab, selected_series_index, source, table.name, column.name);
                }
              }
              ImGui::PopID();
            }
            ImGui::TreePop();
          }
        } else {
          for (const auto& column : table.columns) {
            if (column.time_candidate) {
              continue;
            }
            const auto selected_series_index = tab.active_series_index;
            const bool can_bind = selected_series_index < tab.series.size() && !tab.series[selected_series_index].derived;
            ImGui::PushID(column.name.c_str());
            if (ImGui::Selectable(column.name.c_str())) {
              tsv::app::add_raw_series(app, source, std::nullopt, column.name);
            }
            if (can_bind) {
              ImGui::SameLine();
              if (ImGui::SmallButton("Bind selected")) {
                tsv::app::bind_series_to_source(app, static_cast<std::size_t>(app.active_window), window.active_tab, selected_series_index, source, std::nullopt, column.name);
              }
            }
            ImGui::PopID();
          }
        }
      }
      ImGui::TreePop();
    }
  }

  if (!app.series_errors.empty()) {
    ImGui::SeparatorText("Series Errors");
    for (const auto& [name, error] : app.series_errors) {
      ImGui::BulletText("%s: %s", name.c_str(), error.c_str());
    }
  }

  ImGui::TextWrapped("%s", app.status.c_str());
  ImGui::End();
}

void render_analysis_windows(tsv::app::AppState& app) {
  tsv::app::ensure_workspace_defaults(app);
  for (std::size_t window_index = 0; window_index < app.workspace.windows.size(); ++window_index) {
    auto& window = app.workspace.windows[window_index];
    if (ImGui::Begin(window.title.c_str())) {
      if (ImGui::Button("New tab")) {
        tsv::app::add_tab(app, window_index);
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

            render_series_editor(app, window_index, tab_index);
            ImGui::SeparatorText("Plot");

            if (ImPlot::BeginPlot(("plot##" + std::to_string(window_index) + "_" + std::to_string(tab_index)).c_str(), ImVec2(-1, 360))) {
              if (tab.autoscale_x && tab.autoscale_y) {
                ImPlot::SetupAxes("time", "value", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
              } else {
                ImPlot::SetupAxes("time", "value", tab.autoscale_x ? ImPlotAxisFlags_AutoFit : ImPlotAxisFlags_None, tab.autoscale_y ? ImPlotAxisFlags_AutoFit : ImPlotAxisFlags_None);
                if (!tab.autoscale_x && tab.x_range.has_value()) {
                  ImPlot::SetupAxisLimits(ImAxis_X1, tab.x_range->at(0), tab.x_range->at(1), ImGuiCond_Always);
                }
                if (!tab.autoscale_y && tab.y_range.has_value()) {
                  ImPlot::SetupAxisLimits(ImAxis_Y1, tab.y_range->at(0), tab.y_range->at(1), ImGuiCond_Always);
                }
              }

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

  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);
  app.workspace.windows.front().tabs.front().expression_draft = "series(\"source.variable\")";

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    tsv::app::poll_live_reload(app);

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
