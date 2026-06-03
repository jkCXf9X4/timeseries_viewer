#include <array>
#include <algorithm>
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
#if defined(_WIN32)
#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>
#elif defined(__APPLE__)
#ifndef GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>
#else
#ifndef GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>
#endif
#include <imgui.h>
#include <implot.h>
#include <nfd_glfw3.h>

#include "timeseries_viewer/app_model.hpp"
#include "timeseries_viewer/app_ui.hpp"
#include "timeseries_viewer/nfd_dialog_helpers.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace fs = std::filesystem;

namespace {

std::string with_hidden_id(std::string_view label, std::string_view id) {
  if (id.empty()) {
    return std::string(label);
  }
  std::string result(label);
  result += "##";
  result += id;
  return result;
}

struct ImGuiBackend {
  GLFWwindow* parent_window = nullptr;

  void set_parent_window(GLFWwindow* window) {
    parent_window = window;
  }

  void set_next_window_pos(float x, float y) {
    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
  }

  void set_next_window_pos_appearing(float x, float y) {
    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Appearing);
  }

  void set_next_window_size(float width, float height) {
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
  }

  void set_next_window_size_appearing(float width, float height) {
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Appearing);
  }

  void set_next_window_focus() {
    ImGui::SetNextWindowFocus();
  }

  std::array<float, 2> viewport_size() const {
    const auto* viewport = ImGui::GetMainViewport();
    return {viewport->Size.x, viewport->Size.y};
  }

  std::array<float, 2> window_size() const {
    const ImVec2 size = ImGui::GetWindowSize();
    return {size.x, size.y};
  }

  bool begin_window(std::string_view title, std::string_view, std::uint32_t flags) {
    const std::string label(title);
    return ImGui::Begin(label.c_str(), nullptr, static_cast<ImGuiWindowFlags>(flags));
  }

  bool window_focused() const {
    return ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
  }

  void end_window() {
    ImGui::End();
  }

  bool button(std::string_view label, std::string_view id) {
    const std::string text = with_hidden_id(label, id);
    return ImGui::Button(text.c_str());
  }

  bool small_button(std::string_view label, std::string_view id) {
    const std::string text = with_hidden_id(label, id);
    return ImGui::SmallButton(text.c_str());
  }

  bool selectable(std::string_view label, bool selected, std::string_view id) {
    const std::string text = with_hidden_id(label, id);
    return ImGui::Selectable(text.c_str(), selected);
  }

  bool checkbox(std::string_view label, bool& value, std::string_view id) {
    const std::string text = with_hidden_id(label, id);
    return ImGui::Checkbox(text.c_str(), &value);
  }

  bool input_text(std::string_view label, std::string& value, std::string_view id) {
    std::vector<char> buffer(1024, '\0');
    std::snprintf(buffer.data(), buffer.size(), "%s", value.c_str());
    const std::string text = with_hidden_id(label, id);
    if (ImGui::InputText(text.c_str(), buffer.data(), buffer.size())) {
      value = buffer.data();
      return true;
    }
    return false;
  }

  bool input_u64(std::string_view label, std::uint64_t& value, std::string_view id) {
    const std::string text = with_hidden_id(label, id);
    return ImGui::InputScalar(text.c_str(), ImGuiDataType_U64, &value);
  }

  bool input_double_range(std::string_view label, std::array<double, 2>& value, std::string_view id) {
    const std::string text = with_hidden_id(label, id);
    return ImGui::InputScalarN(text.c_str(), ImGuiDataType_Double, value.data(), 2);
  }

  bool begin_combo(std::string_view label, std::string_view preview, std::string_view id) {
    const std::string combo_label = with_hidden_id(label, id);
    const std::string combo_preview(preview);
    return ImGui::BeginCombo(combo_label.c_str(), combo_preview.c_str());
  }

  void end_combo() {
    ImGui::EndCombo();
  }

  bool tree_node(std::string_view label, std::string_view id) {
    const std::string text = with_hidden_id(label, id);
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

  bool begin_tab_item(std::string_view label, bool, std::string_view id) {
    const std::string text = with_hidden_id(label, id);
    return ImGui::BeginTabItem(text.c_str());
  }

  void end_tab_item() {
    ImGui::EndTabItem();
  }

  bool begin_plot(std::string_view id) {
    const std::string plot_id(id);
    return ImPlot::BeginPlot(plot_id.c_str(), ImVec2(-1, 360));
  }

  bool plot_clicked() const {
    return ImPlot::IsPlotHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
  }

  void focus_window() {
    ImGui::SetWindowFocus();
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

  std::optional<nfdwindowhandle_t> native_parent_window() const {
    if (parent_window == nullptr) {
      return std::nullopt;
    }

    nfdwindowhandle_t native_window{};
    if (!NFD_GetNativeWindowFromGLFWWindow(parent_window, &native_window)) {
      return std::nullopt;
    }
    return native_window;
  }

  bool color_edit4(std::string_view label, std::array<double, 4>& value, std::string_view id) {
    float color[4] = {
      static_cast<float>(value[0]),
      static_cast<float>(value[1]),
      static_cast<float>(value[2]),
      static_cast<float>(value[3])
    };
    const std::string text = with_hidden_id(label, id);
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
      if (result == NFD_ERROR) {
        std::fprintf(stderr, "Open source dialog failed: %s\n", NFD_GetError());
      }
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
      if (result == NFD_ERROR) {
        std::fprintf(stderr, "Open project dialog failed: %s\n", NFD_GetError());
      }
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
      if (result == NFD_ERROR) {
        std::fprintf(stderr, "Save project dialog failed: %s\n", NFD_GetError());
      }
      return std::nullopt;
    }
    fs::path path(reinterpret_cast<const char*>(out_path));
    NFD_FreePathU8(out_path);
    return path;
  }
};

struct NativeAnalysisWindow {
  std::size_t workspace_index = 0;
  GLFWwindow* window = nullptr;
  ImGuiContext* imgui_context = nullptr;
  ImPlotContext* implot_context = nullptr;
  ImGuiBackend ui;

  bool create(GLFWwindow* shared_context, const std::string& title) {
    window = glfwCreateWindow(1100, 720, title.c_str(), nullptr, shared_context);
    if (window == nullptr) {
      return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    imgui_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(imgui_context);
    implot_context = ImPlot::CreateContext();
    ImPlot::SetCurrentContext(implot_context);
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ui.set_parent_window(window);
    return true;
  }

  void destroy() {
    if (imgui_context != nullptr) {
      ImGui::SetCurrentContext(imgui_context);
      ImPlot::SetCurrentContext(implot_context);
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImPlot::DestroyContext(implot_context);
      ImGui::DestroyContext(imgui_context);
      implot_context = nullptr;
      imgui_context = nullptr;
    }
    if (window != nullptr) {
      glfwDestroyWindow(window);
      window = nullptr;
    }
  }
};

bool native_window_closed(const std::vector<std::size_t>& closed_indices, std::size_t index) {
  return std::find(closed_indices.begin(), closed_indices.end(), index) != closed_indices.end();
}

void sync_native_analysis_windows(
  tsv::app::AppState& app,
  GLFWwindow* main_window,
  ImGuiContext* main_imgui_context,
  ImPlotContext* main_implot_context,
  std::vector<NativeAnalysisWindow>& native_windows,
  std::vector<std::size_t>& closed_indices
) {
  tsv::app::ensure_workspace_defaults(app);
  closed_indices.erase(
    std::remove_if(closed_indices.begin(), closed_indices.end(), [&](std::size_t index) {
      return index >= app.workspace.windows.size();
    }),
    closed_indices.end()
  );

  native_windows.erase(
    std::remove_if(native_windows.begin(), native_windows.end(), [&](NativeAnalysisWindow& native_window) {
      if (native_window.workspace_index < app.workspace.windows.size() && !glfwWindowShouldClose(native_window.window)) {
        glfwSetWindowTitle(native_window.window, app.workspace.windows[native_window.workspace_index].title.c_str());
        return false;
      }
      closed_indices.push_back(native_window.workspace_index);
      native_window.destroy();
      return true;
    }),
    native_windows.end()
  );

  for (std::size_t window_index = 1; window_index < app.workspace.windows.size(); ++window_index) {
    if (native_window_closed(closed_indices, window_index)) {
      continue;
    }
    const auto existing = std::find_if(native_windows.begin(), native_windows.end(), [&](const NativeAnalysisWindow& native_window) {
      return native_window.workspace_index == window_index;
    });
    if (existing != native_windows.end()) {
      continue;
    }

    NativeAnalysisWindow native_window;
    native_window.workspace_index = window_index;
    if (native_window.create(main_window, app.workspace.windows[window_index].title)) {
      native_windows.push_back(std::move(native_window));
    }
  }

  glfwMakeContextCurrent(main_window);
  ImGui::SetCurrentContext(main_imgui_context);
  ImPlot::SetCurrentContext(main_implot_context);
}

void render_native_analysis_windows(tsv::app::AppState& app, std::vector<NativeAnalysisWindow>& native_windows) {
  for (auto& native_window : native_windows) {
    if (native_window.window == nullptr || native_window.workspace_index >= app.workspace.windows.size()) {
      continue;
    }

    glfwMakeContextCurrent(native_window.window);
    ImGui::SetCurrentContext(native_window.imgui_context);
    ImPlot::SetCurrentContext(native_window.implot_context);
    native_window.ui.set_parent_window(native_window.window);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    tsv::ui::render_native_analysis_window(app, native_window.ui, native_window.workspace_index);

    ImGui::Render();
    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(native_window.window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(native_window.window);
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
  ImGuiContext* main_imgui_context = ImGui::CreateContext();
  ImGui::SetCurrentContext(main_imgui_context);
  ImPlotContext* main_implot_context = ImPlot::CreateContext();
  ImPlot::SetCurrentContext(main_implot_context);

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  tsv::app::AppState app;
  tsv::app::ensure_workspace_defaults(app);
  app.workspace.windows.front().tabs.front().expression_draft = "series(\"source.variable\")";

  ImGuiBackend ui;
  ui.set_parent_window(window);
  std::vector<NativeAnalysisWindow> native_windows;
  std::vector<std::size_t> closed_native_window_indices;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    tsv::app::poll_live_reload(app);

    sync_native_analysis_windows(
      app,
      window,
      main_imgui_context,
      main_implot_context,
      native_windows,
      closed_native_window_indices
    );

    glfwMakeContextCurrent(window);
    ImGui::SetCurrentContext(main_imgui_context);
    ImPlot::SetCurrentContext(main_implot_context);
    ui.set_parent_window(window);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    tsv::ui::render_main_native_app(app, ui);

    ImGui::Render();
    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);

    sync_native_analysis_windows(
      app,
      window,
      main_imgui_context,
      main_implot_context,
      native_windows,
      closed_native_window_indices
    );
    render_native_analysis_windows(app, native_windows);
  }

  for (auto& native_window : native_windows) {
    native_window.destroy();
  }
  native_windows.clear();
  glfwMakeContextCurrent(window);
  ImGui::SetCurrentContext(main_imgui_context);
  ImPlot::SetCurrentContext(main_implot_context);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImPlot::DestroyContext(main_implot_context);
  ImGui::DestroyContext(main_imgui_context);

  glfwDestroyWindow(window);
  glfwTerminate();
  NFD_Quit();
  return 0;
}
