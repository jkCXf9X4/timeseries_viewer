#include "app_internal.hpp"

namespace tsv::app {

std::string sanitize_identifier(std::string value) {
  for (char& ch : value) {
    const auto u = static_cast<unsigned char>(ch);
    if (!std::isalnum(u) && ch != '_') ch = '_';
  }
  while (value.find("__") != std::string::npos) value.erase(value.find("__"), 1);
  if (value.empty()) value = "source";
  return value;
}

std::string unique_alias(const std::vector<OpenSource>& sources, const std::string& preferred) {
  const auto base = sanitize_identifier(preferred);
  std::string alias = base;
  int suffix = 2;
  const auto exists = [&](const std::string& candidate) {
    return std::any_of(sources.begin(), sources.end(), [&](const OpenSource& src) { return src.alias == candidate; });
  };
  while (exists(alias)) alias = base + "_" + std::to_string(suffix++);
  return alias;
}

void open_source(AppState& app, const fs::path& path, const std::optional<std::string>& alias_override, const std::optional<tsv::SourceKind>& kind_override) {
  try {
    OpenSource source;
    const auto kind = kind_override.has_value() ? *kind_override : (path.extension() == ".csv" ? tsv::SourceKind::Csv : tsv::SourceKind::Sqlite);
    source.catalog = kind == tsv::SourceKind::Csv ? tsv::load_csv_catalog(path) : tsv::load_sqlite_catalog(path);
    source.alias = alias_override.has_value() ? *alias_override : unique_alias(app.sources, path.stem().string());
    source.catalog.source_name = source.alias;
    source.last_write_time = detail::source_stamp(path, source.catalog.kind);
    app.sources.push_back(std::move(source));
    app.status = "Opened " + path.filename().string();
  } catch (const std::exception& ex) { app.status = ex.what(); }
}

void reload_sources(AppState& app) {
  for (auto& source : app.sources) source.last_write_time.reset();
  app.raw_series_cache.clear();
  rebuild_cache(app);
  app.status = "Reloaded data";
}

void poll_live_reload(AppState& app) {
  if (!app.live_mode) return;
  const auto now = std::chrono::steady_clock::now();
  if (now - app.last_poll < std::chrono::seconds(1)) return;
  app.last_poll = now;
  bool changed = false;
  std::vector<std::size_t> changed_sources;
  changed_sources.reserve(app.sources.size());
  for (std::size_t index = 0; index < app.sources.size(); ++index) {
    auto& source = app.sources[index];
    const auto current = detail::source_stamp(source.catalog.path, source.catalog.kind);
    if (current.has_value() && current != source.last_write_time) {
      source.last_write_time = current;
      changed = true;
      changed_sources.push_back(index);
    }
  }
  if (changed) {
    detail::refresh_live_cache(app, changed_sources);
    app.status = "Live data refreshed";
  }
}

} // namespace tsv::app