#include <cstdio>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glad/glad.h>

#include <imgui.h>
#include <implot.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "timeseries_viewer/app_model.hpp"
#include "timeseries_viewer/app_ui.hpp"
#include "ui/backend/imgui_backend.hpp"

int main() {
  if (NFD_Init() != NFD_OKAY) return 1;

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
  ui.set_parent_window(window);

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
