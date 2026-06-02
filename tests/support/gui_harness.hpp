#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <algorithm>

#include "timeseries_viewer/app_model.hpp"

namespace tsv::test {

namespace fs = std::filesystem;

class GuiHarness {
 public:
  void open_source(const fs::path& path, const std::string& alias = {}) {
    tsv::app::open_source(app_, path, alias.empty() ? std::optional<std::string>{} : std::optional<std::string>{alias}, tsv::SourceKind::Csv);
    tsv::app::rebuild_cache(app_);
  }

  void open_sqlite(const fs::path& path, const std::string& alias = {}) {
    tsv::app::open_source(app_, path, alias.empty() ? std::optional<std::string>{} : std::optional<std::string>{alias}, tsv::SourceKind::Sqlite);
    tsv::app::rebuild_cache(app_);
  }

  void select_window(std::size_t window_index) {
    tsv::app::ensure_workspace_defaults(app_);
    app_.active_window = static_cast<int>(window_index);
  }

  void select_tab(std::size_t window_index, std::size_t tab_index) {
    select_window(window_index);
    app_.workspace.windows.at(window_index).active_tab = tab_index;
  }

  void click_new_window(const std::string& title = {}) {
    tsv::app::add_window(app_, title);
  }

  void click_new_tab(std::size_t window_index, const std::string& title = {}) {
    tsv::app::add_tab(app_, window_index, title);
  }

  void click_parameter(std::size_t window_index, std::size_t tab_index, const std::string& source_alias, const std::optional<std::string>& table_name, const std::string& value_column) {
    const auto* source = source_by_alias(source_alias);
    if (source == nullptr) {
      app_.status = "Source not found";
      return;
    }
    select_tab(window_index, tab_index);
    tsv::app::add_raw_series(app_, *source, table_name, value_column);
  }

  void click_add_raw_to_active(const std::string& source_alias, const std::optional<std::string>& table_name, const std::string& value_column) {
    const auto* source = source_by_alias(source_alias);
    if (source == nullptr) {
      app_.status = "Source not found";
      return;
    }
    tsv::app::add_raw_series(app_, *source, table_name, value_column);
  }

  void click_bind_selected(std::size_t window_index, std::size_t tab_index, std::size_t series_index, const std::string& source_alias, const std::optional<std::string>& table_name, const std::string& value_column, const std::optional<std::string>& time_column = std::nullopt) {
    const auto* source = source_by_alias(source_alias);
    if (source == nullptr) {
      app_.status = "Source not found";
      return;
    }
    tsv::app::bind_series_to_source(app_, window_index, tab_index, series_index, *source, table_name, value_column, time_column);
  }

  void set_expression_draft(std::size_t window_index, std::size_t tab_index, const std::string& expression) {
    app_.workspace.windows.at(window_index).tabs.at(tab_index).expression_draft = expression;
  }

  void click_add_derived(std::size_t window_index, std::size_t tab_index) {
    tsv::app::add_derived_series_to_tab(app_, window_index, tab_index);
  }

  void click_toggle_series_visibility(std::size_t window_index, std::size_t tab_index, std::size_t series_index, bool visible) {
    tsv::app::toggle_series_visibility(app_, window_index, tab_index, series_index, visible);
  }

  void click_remove_series(std::size_t window_index, std::size_t tab_index, std::size_t series_index) {
    tsv::app::remove_series(app_, window_index, tab_index, series_index);
  }

  void set_point_budget(std::size_t budget) {
    app_.workspace.point_budget = budget;
    tsv::app::rebuild_cache(app_);
  }

  void save(const fs::path& path) {
    tsv::app::save_project_file(app_, path);
  }

  void load(const fs::path& path) {
    tsv::app::load_project_file(app_, path);
  }

  void refresh() {
    tsv::app::rebuild_cache(app_);
  }

  [[nodiscard]] tsv::app::AppState& state() {
    return app_;
  }

  [[nodiscard]] const tsv::app::AppState& state() const {
    return app_;
  }

 private:
  const tsv::app::OpenSource* source_by_alias(const std::string& alias) const {
    const auto it = std::find_if(app_.sources.begin(), app_.sources.end(), [&](const auto& source) {
      return source.alias == alias;
    });
    return it == app_.sources.end() ? nullptr : &*it;
  }

  tsv::app::AppState app_;
};

} // namespace tsv::test
