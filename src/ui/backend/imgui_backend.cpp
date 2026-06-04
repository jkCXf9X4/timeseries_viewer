#include "imgui_backend.hpp"

#include <cstdio>

#include <glad/glad.h>

namespace {

std::string with_hidden_id(std::string_view label, std::string_view id) {
  if (id.empty()) return std::string(label);
  std::string result(label);
  result += "##";
  result += id;
  return result;
}

} // namespace

void ImGuiBackend::set_parent_window(GLFWwindow* window) {
  parent_window = window;
}

void ImGuiBackend::set_next_window_pos(float x, float y) {
  ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
}

void ImGuiBackend::set_next_window_size(float width, float height) {
  ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
}

std::array<float, 2> ImGuiBackend::viewport_size() const {
  const auto* viewport = ImGui::GetMainViewport();
  return {viewport->Size.x, viewport->Size.y};
}

std::array<float, 2> ImGuiBackend::window_size() const {
  const ImVec2 size = ImGui::GetWindowSize();
  return {size.x, size.y};
}

std::array<float, 2> ImGuiBackend::content_region_avail() const {
  const ImVec2 size = ImGui::GetContentRegionAvail();
  return {size.x, size.y};
}

bool ImGuiBackend::begin_window(std::string_view title, std::string_view, std::uint32_t flags) {
  const std::string label(title);
  return ImGui::Begin(label.c_str(), nullptr, static_cast<ImGuiWindowFlags>(flags));
}

bool ImGuiBackend::window_focused() const {
  return ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
}

void ImGuiBackend::end_window() {
  ImGui::End();
}

bool ImGuiBackend::begin_popup_context_window() {
  return ImGui::BeginPopupContextWindow();
}

bool ImGuiBackend::menu_item(std::string_view label, std::string_view id) {
  const std::string text = with_hidden_id(label, id);
  return ImGui::MenuItem(text.c_str());
}

void ImGuiBackend::end_popup() {
  ImGui::EndPopup();
}

bool ImGuiBackend::button(std::string_view label, std::string_view id) {
  const std::string text = with_hidden_id(label, id);
  return ImGui::Button(text.c_str());
}

bool ImGuiBackend::small_button(std::string_view label, std::string_view id) {
  const std::string text = with_hidden_id(label, id);
  return ImGui::SmallButton(text.c_str());
}

bool ImGuiBackend::selectable(std::string_view label, bool selected, std::string_view id) {
  const std::string text = with_hidden_id(label, id);
  return ImGui::Selectable(text.c_str(), selected);
}

bool ImGuiBackend::checkbox(std::string_view label, bool& value, std::string_view id) {
  const std::string text = with_hidden_id(label, id);
  return ImGui::Checkbox(text.c_str(), &value);
}

bool ImGuiBackend::input_text(std::string_view label, std::string& value, std::string_view id) {
  std::vector<char> buffer(1024, '\0');
  std::snprintf(buffer.data(), buffer.size(), "%s", value.c_str());
  const std::string text = with_hidden_id(label, id);
  if (ImGui::InputText(text.c_str(), buffer.data(), buffer.size())) {
    value = buffer.data();
    return true;
  }
  return false;
}

bool ImGuiBackend::input_u64(std::string_view label, std::uint64_t& value, std::string_view id) {
  const std::string text = with_hidden_id(label, id);
  return ImGui::InputScalar(text.c_str(), ImGuiDataType_U64, &value);
}

bool ImGuiBackend::input_double_range(std::string_view label, std::array<double, 2>& value, std::string_view id) {
  const std::string text = with_hidden_id(label, id);
  return ImGui::InputScalarN(text.c_str(), ImGuiDataType_Double, value.data(), 2);
}

bool ImGuiBackend::begin_combo(std::string_view label, std::string_view preview, std::string_view id) {
  const std::string combo_label = with_hidden_id(label, id);
  const std::string combo_preview(preview);
  return ImGui::BeginCombo(combo_label.c_str(), combo_preview.c_str());
}

void ImGuiBackend::end_combo() {
  ImGui::EndCombo();
}

bool ImGuiBackend::tree_node(std::string_view label, std::string_view id) {
  const std::string text = with_hidden_id(label, id);
  return ImGui::TreeNode(text.c_str());
}

void ImGuiBackend::tree_pop() {
  ImGui::TreePop();
}

bool ImGuiBackend::begin_table(std::string_view id, int columns) {
  const std::string table_id(id);
  return ImGui::BeginTable(table_id.c_str(), columns, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp);
}

void ImGuiBackend::end_table() {
  ImGui::EndTable();
}

void ImGuiBackend::table_setup_column(std::string_view label) {
  const std::string text(label);
  ImGui::TableSetupColumn(text.c_str());
}

void ImGuiBackend::table_headers_row() {
  ImGui::TableHeadersRow();
}

void ImGuiBackend::table_next_row() {
  ImGui::TableNextRow();
}

void ImGuiBackend::table_set_column_index(int index) {
  ImGui::TableSetColumnIndex(index);
}

bool ImGuiBackend::begin_tab_bar(std::string_view id) {
  const std::string tab_id(id);
  return ImGui::BeginTabBar(tab_id.c_str());
}

void ImGuiBackend::end_tab_bar() {
  ImGui::EndTabBar();
}

bool ImGuiBackend::begin_tab_item(std::string_view label, bool, std::string_view id) {
  const std::string text = with_hidden_id(label, id);
  return ImGui::BeginTabItem(text.c_str());
}

void ImGuiBackend::end_tab_item() {
  ImGui::EndTabItem();
}

bool ImGuiBackend::begin_plot(std::string_view id, std::array<float, 2> size) {
  const std::string plot_id(id);
  return ImPlot::BeginPlot(plot_id.c_str(), ImVec2(size[0], size[1]));
}

bool ImGuiBackend::plot_clicked() const {
  return ImPlot::IsPlotHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
}

void ImGuiBackend::setup_axes(std::string_view x_label, std::string_view y_label, bool autoscale_x, bool autoscale_y) {
  const std::string x(x_label);
  const std::string y(y_label);
  ImPlot::SetupAxes(x.c_str(), y.c_str(), autoscale_x ? ImPlotAxisFlags_AutoFit : ImPlotAxisFlags_None, autoscale_y ? ImPlotAxisFlags_AutoFit : ImPlotAxisFlags_None);
}

void ImGuiBackend::setup_axis_limits(std::string_view axis, double min_value, double max_value) {
  if (axis == "x") {
    ImPlot::SetupAxisLimits(ImAxis_X1, min_value, max_value, ImGuiCond_Always);
  } else {
    ImPlot::SetupAxisLimits(ImAxis_Y1, min_value, max_value, ImGuiCond_Always);
  }
}

void ImGuiBackend::plot_line(std::string_view label, const tsv::SeriesData& series, const std::array<double, 4>& color) {
  const std::string text(label);
  const ImPlotSpec spec{
    ImPlotProp_LineColor, ImVec4(
      static_cast<float>(color[0]),
      static_cast<float>(color[1]),
      static_cast<float>(color[2]),
      static_cast<float>(color[3]))
  };
  ImPlot::PlotLine(text.c_str(), series.time.data(), series.value.data(), static_cast<int>(series.time.size()), spec);
}

void ImGuiBackend::end_plot() {
  ImPlot::EndPlot();
}

void ImGuiBackend::separator_text(std::string_view text) {
  const std::string label(text);
  ImGui::SeparatorText(label.c_str());
}

void ImGuiBackend::same_line() {
  ImGui::SameLine();
}

void ImGuiBackend::text(std::string_view value) {
  const std::string text_value(value);
  ImGui::TextUnformatted(text_value.c_str());
}

void ImGuiBackend::text_disabled(std::string_view value) {
  const std::string text_value(value);
  ImGui::TextDisabled("%s", text_value.c_str());
}

void ImGuiBackend::push_id(std::string_view id) {
  const std::string text(id);
  ImGui::PushID(text.c_str());
}

void ImGuiBackend::pop_id() {
  ImGui::PopID();
}

std::optional<nfdwindowhandle_t> ImGuiBackend::native_parent_window() const {
  if (parent_window == nullptr) return std::nullopt;
  nfdwindowhandle_t native_window{};
  if (!NFD_GetNativeWindowFromGLFWWindow(parent_window, &native_window)) {
    return std::nullopt;
  }
  return native_window;
}

bool ImGuiBackend::color_edit4(std::string_view label, std::array<double, 4>& value, std::string_view id) {
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

std::optional<std::filesystem::path> ImGuiBackend::open_source_dialog() {
  nfdu8char_t* out_path = nullptr;
  const nfdu8filteritem_t filters[] = {
    {"Time series", "csv,sqlite,db"}
  };
  const auto parent_window = native_parent_window();
  const nfdopendialogu8args_t args{
    filters,
    1,
    nullptr,
    parent_window.has_value() ? parent_window.value() : nfdwindowhandle_t{}
  };
  const auto result = NFD_OpenDialogU8_With(&out_path, &args);
  if (result != NFD_OKAY) return std::nullopt;
  std::filesystem::path path(reinterpret_cast<const char*>(out_path));
  NFD_FreePathU8(out_path);
  return path;
}

std::optional<std::filesystem::path> ImGuiBackend::open_project_dialog() {
  nfdu8char_t* out_path = nullptr;
  const nfdu8filteritem_t filters[] = {
    {"Project", "json"}
  };
  const auto parent_window = native_parent_window();
  const nfdopendialogu8args_t args{
    filters,
    1,
    nullptr,
    parent_window.has_value() ? parent_window.value() : nfdwindowhandle_t{}
  };
  const auto result = NFD_OpenDialogU8_With(&out_path, &args);
  if (result != NFD_OKAY) return std::nullopt;
  std::filesystem::path path(reinterpret_cast<const char*>(out_path));
  NFD_FreePathU8(out_path);
  return path;
}

std::optional<std::filesystem::path> ImGuiBackend::save_project_dialog() {
  nfdu8char_t* out_path = nullptr;
  const nfdu8filteritem_t filters[] = {
    {"Project", "json"}
  };
  const auto parent_window = native_parent_window();
  const nfdsavedialogu8args_t args{
    filters,
    1,
    nullptr,
    nullptr,
    parent_window.has_value() ? parent_window.value() : nfdwindowhandle_t{}
  };
  const auto result = NFD_SaveDialogU8_With(&out_path, &args);
  if (result != NFD_OKAY) return std::nullopt;
  std::filesystem::path path(reinterpret_cast<const char*>(out_path));
  NFD_FreePathU8(out_path);
  return path;
}
