#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "timeseries_viewer/core.hpp"

namespace tsv::test {

namespace fs = std::filesystem;

class ScriptedGuiBackend {
 public:
  void click(std::string id_or_label) {
    clicks_.insert(std::move(id_or_label));
  }

  void set_text(std::string id_or_label, std::string value) {
    text_values_[std::move(id_or_label)] = std::move(value);
  }

  void set_checkbox(std::string id_or_label, bool value) {
    checkbox_values_[std::move(id_or_label)] = value;
  }

  void set_u64(std::string id_or_label, std::uint64_t value) {
    u64_values_[std::move(id_or_label)] = value;
  }

  void set_range(std::string id_or_label, std::array<double, 2> value) {
    range_values_[std::move(id_or_label)] = value;
  }

  void set_open_source_dialog(fs::path path) {
    open_source_path_ = std::move(path);
  }

  void set_open_project_dialog(fs::path path) {
    open_project_path_ = std::move(path);
  }

  void set_save_project_dialog(fs::path path) {
    save_project_path_ = std::move(path);
  }

  bool begin_window(std::string_view, std::string_view) {
    return true;
  }

  void end_window() {}

  bool button(std::string_view label, std::string_view id) {
    return consume_click(label, id);
  }

  bool small_button(std::string_view label, std::string_view id) {
    return consume_click(label, id);
  }

  bool selectable(std::string_view label, bool, std::string_view id) {
    return consume_click(label, id);
  }

  bool checkbox(std::string_view label, bool& value, std::string_view id) {
    const std::string key = id.empty() ? std::string(label) : std::string(id);
    const auto it = checkbox_values_.find(key);
    if (it != checkbox_values_.end()) {
      const bool target = it->second;
      checkbox_values_.erase(it);
      if (target != value) {
        value = target;
        return true;
      }
      return false;
    }
    return false;
  }

  bool input_text(std::string_view label, std::string& value, std::string_view id) {
    if (const auto target = lookup_text(label, id); target.has_value() && *target != value) {
      value = *target;
      return true;
    }
    return false;
  }

  bool input_u64(std::string_view label, std::uint64_t& value, std::string_view id) {
    if (const auto target = lookup_u64(label, id); target.has_value() && *target != value) {
      value = *target;
      return true;
    }
    return false;
  }

  bool input_double_range(std::string_view label, std::array<double, 2>& value, std::string_view id) {
    if (const auto target = lookup_range(label, id); target.has_value() && *target != value) {
      value = *target;
      return true;
    }
    return false;
  }

  bool begin_combo(std::string_view, std::string_view, std::string_view) {
    return true;
  }

  void end_combo() {}

  bool tree_node(std::string_view, std::string_view) {
    return true;
  }

  void tree_pop() {}

  bool begin_table(std::string_view, int) {
    return true;
  }

  void end_table() {}

  void table_setup_column(std::string_view) {}

  void table_headers_row() {}

  void table_next_row() {}

  void table_set_column_index(int) {}

  bool begin_tab_bar(std::string_view) {
    return true;
  }

  void end_tab_bar() {}

  bool begin_tab_item(std::string_view, bool selected, std::string_view) {
    return selected;
  }

  void end_tab_item() {}

  bool begin_plot(std::string_view) {
    return true;
  }

  void setup_axes(std::string_view, std::string_view, bool, bool) {}

  void setup_axis_limits(std::string_view, double, double) {}

  void plot_line(std::string_view label, const tsv::SeriesData&) {
    plot_labels.push_back(std::string(label));
  }

  void end_plot() {}

  void separator_text(std::string_view) {}

  void same_line() {}

  void text(std::string_view) {}

  void text_disabled(std::string_view) {}

  void push_id(std::string_view) {}

  void pop_id() {}

  bool color_edit4(std::string_view label, std::array<double, 4>& value, std::string_view id) {
    if (const auto target = lookup_color(label, id); target.has_value() && *target != value) {
      value = *target;
      return true;
    }
    return false;
  }

  std::optional<fs::path> open_source_dialog() {
    return take_path(open_source_path_);
  }

  std::optional<fs::path> open_project_dialog() {
    return take_path(open_project_path_);
  }

  std::optional<fs::path> save_project_dialog() {
    return take_path(save_project_path_);
  }

  std::vector<std::string> plot_labels;

 private:
  bool consume_click(std::string_view label, std::string_view id) {
    const std::string key = id.empty() ? std::string(label) : std::string(id);
    const auto it = clicks_.find(key);
    if (it != clicks_.end()) {
      clicks_.erase(it);
      return true;
    }

    const auto fallback = clicks_.find(std::string(label));
    if (fallback != clicks_.end()) {
      clicks_.erase(fallback);
      return true;
    }

    return false;
  }

  std::optional<std::string> lookup_text(std::string_view label, std::string_view id) const {
    return lookup_value(text_values_, label, id);
  }

  std::optional<std::uint64_t> lookup_u64(std::string_view label, std::string_view id) const {
    return lookup_value(u64_values_, label, id);
  }

  std::optional<std::array<double, 2>> lookup_range(std::string_view label, std::string_view id) const {
    return lookup_value(range_values_, label, id);
  }

  std::optional<std::array<double, 4>> lookup_color(std::string_view label, std::string_view id) const {
    return lookup_value(color_values_, label, id);
  }

  template <typename T>
  static std::optional<T> lookup_value(const std::unordered_map<std::string, T>& values, std::string_view label, std::string_view id) {
    const std::string key = id.empty() ? std::string(label) : std::string(id);
    const auto it = values.find(key);
    if (it != values.end()) {
      return it->second;
    }

    const auto fallback = values.find(std::string(label));
    if (fallback != values.end()) {
      return fallback->second;
    }

    return std::nullopt;
  }

  static std::optional<fs::path> take_path(std::optional<fs::path>& path) {
    if (!path.has_value()) {
      return std::nullopt;
    }
    const auto result = path;
    path.reset();
    return result;
  }

  std::unordered_set<std::string> clicks_;
  std::unordered_map<std::string, std::string> text_values_;
  std::unordered_map<std::string, bool> checkbox_values_;
  std::unordered_map<std::string, std::uint64_t> u64_values_;
  std::unordered_map<std::string, std::array<double, 2>> range_values_;
  std::unordered_map<std::string, std::array<double, 4>> color_values_;
  std::optional<fs::path> open_source_path_;
  std::optional<fs::path> open_project_path_;
  std::optional<fs::path> save_project_path_;
};

} // namespace tsv::test
