#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <implot.h>
#include <nfd.h>

#include "timeseries_viewer/app_model.hpp"
#include "timeseries_viewer/app_ui.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace fs = std::filesystem;

namespace {

struct ImGuiBackend {
  bool begin_window(std::string_view title, std::string_view) {
    const std::string label(title);
    return ImGui::Begin(label.c_str());
  }

  void end_window() {
    ImGui::End();
  }

  bool button(std::string_view label, std::string_view) {
    const std::string text(label);
    return ImGui::Button(text.c_str());
  }

  bool small_button(std::string_view label, std::string_view) {
    const std::string text(label);
    return ImGui::SmallButton(text.c_str());
  }

  bool selectable(std::string_view label, bool selected, std::string_view) {
    const std::string text(label);
    return ImGui::Selectable(text.c_str(), selected);
  }

  bool checkbox(std::string_view label, bool& value, std::string_view) {
    const std::string text(label);
    return ImGui::Checkbox(text.c_str(), &value);
  }

  bool input_text(std::string_view label, std::string& value, std::string_view) {
    std::vector<char> buffer(1024, '\0');
    std::snprintf(buffer.data(), buffer.size(), "%s", value.c_str());
    const std::string text(label);
    if (ImGui::InputText(text.c_str(), buffer.data(), buffer.size())) {
      value = buffer.data();
      return true;
    }
    return false;
  }

  bool input_u64(std::string_view label, std::uint64_t& value, std::string_view) {
    const std::string text(label);
    return ImGui::InputScalar(text.c_str(), ImGuiDataType_U64, &value);
  }

  bool input_double_range(std::string_view label, std::array<double, 2>& value, std::string_view) {
    const std::string text(label);
    return ImGui::InputScalarN(text.c_str(), ImGuiDataType_Double, value.data(), 2);
  }

  bool begin_combo(std::string_view label, std::string_view preview, std::string_view) {
    const std::string combo_label(label);
    const std::string combo_preview(preview);
    return ImGui::BeginCombo(combo_label.c_str(), combo_preview.c_str());
  }

  void end_combo() {
    ImGui::EndCombo();
  }

  bool tree_node(std::string_view label, std::string_view) {
    const std::string text(label);
    return ImGui::TreeNode(text.c_str());
  }

  void tree_pop() {
    ImGui::TreePop();
  }

  bool begin_table(std::string_view id, int columns) {
    const std::string table_id(id);
    return ImGui::BeginTable(table_id.c_str(), columns, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp);
  }

  void end_table() {
    ImGui::EndTable();
  }

  void table_setup_column(std::string_view label) {
    const std::string text(label);
    ImGui::TableSetupColumn(text.c_str());
  }

  void table_headers_row() {
    ImGui::TableHeadersRow();
  }

  void table_next_row() {
    ImGui::TableNextRow();
  }

  void table_set_column_index(int index) {
    ImGui::TableSetColumnIndex(index);
  }

  bool begin_tab_bar(std::string_view id) {
    const std::string tab_id(id);
    return ImGui::BeginTabBar(tab_id.c_str());
  }

  void end_tab_bar() {
    ImGui::EndTabBar();
  }

  bool begin_tab_item(std::string_view label, bool, std::string_view) {
    const std::string text(label);
    return ImGui::BeginTabItem(text.c_str());
  }

  void end_tab_item() {
    ImGui::EndTabItem();
  }

  bool begin_plot(std::string_view id) {
    const std::string plot_id(id);
    return ImPlot::BeginPlot(plot_id.c_str(), ImVec2(-1, 360));
  }

  void setup_axes(std::string_view x_label, std::string_view y_label, bool autoscale_x, bool autoscale_y) {
    const std::string x(x_label);
    const std::string y(y_label);
    ImPlot::SetupAxes(x.c_str(), y.c_str(), autoscale_x ? ImPlotAxisFlags_AutoFit : ImPlotAxisFlags_None, autoscale_y ? ImPlotAxisFlags_AutoFit : ImPlotAxisFlags_None);
  }

  void setup_axis_limits(std::string_view axis, double min_value, double max_value) {
    if (axis == "x") {
      ImPlot::SetupAxisLimits(ImAxis_X1, min_value, max_value, ImGuiCond_Always);
    } else {
      ImPlot::SetupAxisLimits(ImAxis_Y1, min_value, max_value, ImGuiCond_Always);
    }
  }

  void plot_line(std::string_view label, const tsv::SeriesData& series) {
    const std::string text(label);
    ImPlot::PlotLine(text.c_str(), series.time.data(), series.value.data(), static_cast<int>(series.time.size()));
  }

  void end_plot() {
    ImPlot::EndPlot();
  }

  void separator_text(std::string_view text) {
    const std::string label(text);
    ImGui::SeparatorText(label.c_str());
  }

  void same_line() {
    ImGui::SameLine();
  }

  void text(std::string_view value) {
    const std::string text_value(value);
    ImGui::TextUnformatted(text_value.c_str());
  }

  void text_disabled(std::string_view value) {
    const std::string text_value(value);
    ImGui::TextDisabled("%s", text_value.c_str());
  }

  void push_id(std::string_view id) {
    const std::string text(id);
    ImGui::PushID(text.c_str());
  }

  void pop_id() {
    ImGui::PopID();
  }

  bool color_edit4(std::string_view label, std::array<double, 4>& value, std::string_view) {
    float color[4] = {
      static_cast<float>(value[0]),
      static_cast<float>(value[1]),
      static_cast<float>(value[2]),
      static_cast<float>(value[3])
    };
    const std::string text(label);
    if (ImGui::ColorEdit4(text.c_str(), color, ImGuiColorEditFlags_NoInputs)) {
      for (std::size_t index = 0; index < 4; ++index) {
        value[index] = color[index];
      }
      return true;
    }
    return false;
  }

  std::optional<fs::path> open_source_dialog() {
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

  std::optional<fs::path> save_project_dialog() {
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
};

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

  ImGuiBackend ui;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    tsv::app::poll_live_reload(app);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    tsv::ui::render_app(app, ui);

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
