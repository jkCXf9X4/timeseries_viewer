#pragma once

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "timeseries_viewer/core.hpp"

namespace tsv::app {

namespace fs = std::filesystem;

struct OpenSource {
  tsv::SourceCatalog catalog;
  std::string alias;
  std::optional<fs::file_time_type> last_write_time;
};

struct BindableParameter {
  std::string display_name;
  std::string source_alias;
  std::optional<std::string> table_name;
  std::string value_column;
  std::string source_path;
};

struct SeriesBindingKey {
  fs::path path;
  std::optional<std::string> table_name;
  std::optional<std::string> time_column;
  std::string value_column;

  bool operator==(const SeriesBindingKey& other) const;
};

struct SeriesBindingKeyHash {
  std::size_t operator()(const SeriesBindingKey& key) const noexcept;
};

struct RawCacheEntry {
  std::optional<fs::file_time_type> source_stamp;
  tsv::SeriesData series;
  std::optional<std::size_t> max_points;
};

struct AppState {
  std::vector<OpenSource> sources;
  tsv::WorkspaceConfig workspace;
  std::unordered_map<std::string, tsv::SeriesData> series_cache;
  std::unordered_map<std::string, std::string> series_errors;
  std::unordered_map<SeriesBindingKey, RawCacheEntry, SeriesBindingKeyHash> raw_series_cache;
  std::string status{"Ready"};
  bool live_mode{false};
  std::chrono::steady_clock::time_point last_poll{std::chrono::steady_clock::now()};
  fs::path project_path;
  int active_window{0};
};

std::string sanitize_identifier(std::string value);
std::string unique_alias(const std::vector<OpenSource>& sources, const std::string& preferred);

void ensure_workspace_defaults(AppState& app);
tsv::AnalysisWindowConfig& active_window(AppState& app);
tsv::PlotTabConfig& active_tab(AppState& app);
const tsv::PlotTabConfig& active_tab(const AppState& app);

void add_window(AppState& app, std::string title = {});
void add_tab(AppState& app, std::size_t window_index, std::string title = {});
void select_series(AppState& app, std::size_t window_index, std::size_t tab_index, std::size_t series_index);
void remove_series(AppState& app, std::size_t window_index, std::size_t tab_index, std::size_t series_index);
void toggle_series_visibility(AppState& app, std::size_t window_index, std::size_t tab_index, std::size_t series_index, bool visible);
void rename_series(AppState& app, std::size_t window_index, std::size_t tab_index, std::size_t series_index, const std::string& name);

void open_source(AppState& app, const fs::path& path, const std::optional<std::string>& alias_override = std::nullopt, const std::optional<tsv::SourceKind>& kind_override = std::nullopt);
void rebuild_cache(AppState& app);
void add_raw_series(AppState& app, const OpenSource& source, const std::optional<std::string>& table_name, const std::string& value_column);
void bind_series_to_source(
  AppState& app,
  std::size_t window_index,
  std::size_t tab_index,
  std::size_t series_index,
  const OpenSource& source,
  const std::optional<std::string>& table_name,
  const std::string& value_column,
  const std::optional<std::string>& time_column_override = std::nullopt
);
void add_derived_series(AppState& app);
void add_derived_series_to_tab(AppState& app, std::size_t window_index, std::size_t tab_index);

[[nodiscard]] std::vector<BindableParameter> list_bindable_parameters(const OpenSource& source);
[[nodiscard]] tsv::TreeNode build_bindable_parameter_tree(const OpenSource& source);
[[nodiscard]] std::optional<BindableParameter> find_bindable_parameter(const OpenSource& source, const std::string& display_name);
[[nodiscard]] std::string plot_legend_label(const tsv::PlotSeriesConfig& series);

void save_project_file(AppState& app, const fs::path& path);
void load_project_file(AppState& app, const fs::path& path);
void poll_live_reload(AppState& app);

} // namespace tsv::app
