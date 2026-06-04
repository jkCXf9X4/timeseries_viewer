#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#define GLFW_INCLUDE_NONE
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

namespace fs = std::filesystem;

struct ImGuiBackend {
  GLFWwindow* parent_window = nullptr;

  void set_parent_window(GLFWwindow* window);
  void set_next_window_pos(float x, float y);
  void set_next_window_size(float width, float height);
  std::array<float, 2> viewport_size() const;
  std::array<float, 2> window_size() const;
  std::array<float, 2> content_region_avail() const;
  bool begin_window(std::string_view title, std::string_view id, std::uint32_t flags);
  bool window_focused() const;
  void end_window();
  bool begin_popup_context_window();
  bool menu_item(std::string_view label, std::string_view id);
  void end_popup();
  bool button(std::string_view label, std::string_view id);
  bool small_button(std::string_view label, std::string_view id);
  bool selectable(std::string_view label, bool selected, std::string_view id);
  bool checkbox(std::string_view label, bool& value, std::string_view id);
  bool input_text(std::string_view label, std::string& value, std::string_view id);
  bool input_u64(std::string_view label, std::uint64_t& value, std::string_view id);
  bool input_double_range(std::string_view label, std::array<double, 2>& value, std::string_view id);
  bool begin_combo(std::string_view label, std::string_view preview, std::string_view id);
  void end_combo();
  bool tree_node(std::string_view label, std::string_view id);
  void tree_pop();
  bool begin_table(std::string_view id, int columns);
  void end_table();
  void table_setup_column(std::string_view label);
  void table_headers_row();
  void table_next_row();
  void table_set_column_index(int index);
  bool begin_tab_bar(std::string_view id);
  void end_tab_bar();
  bool begin_tab_item(std::string_view label, bool, std::string_view id);
  void end_tab_item();
  bool begin_plot(std::string_view id, std::array<float, 2> size);
  bool plot_clicked() const;
  void setup_axes(std::string_view x_label, std::string_view y_label, bool autoscale_x, bool autoscale_y);
  void setup_axis_limits(std::string_view axis, double min_value, double max_value);
  void plot_line(std::string_view label, const tsv::SeriesData& series, const std::array<double, 4>& color);
  void end_plot();
  void separator_text(std::string_view text);
  void same_line();
  void text(std::string_view value);
  void text_disabled(std::string_view value);
  void push_id(std::string_view id);
  void pop_id();
  std::optional<nfdwindowhandle_t> native_parent_window() const;
  bool color_edit4(std::string_view label, std::array<double, 4>& value, std::string_view id);
  std::optional<fs::path> open_source_dialog();
  std::optional<fs::path> open_project_dialog();
  std::optional<fs::path> save_project_dialog();
};
