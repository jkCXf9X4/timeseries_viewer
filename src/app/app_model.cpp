#include "timeseries_viewer/app_model.hpp"

namespace tsv::app {

bool SeriesBindingKey::operator==(const SeriesBindingKey& other) const {
  return path == other.path && table_name == other.table_name && time_column == other.time_column && value_column == other.value_column;
}

std::size_t SeriesBindingKeyHash::operator()(const SeriesBindingKey& key) const noexcept {
  std::size_t seed = std::hash<std::string>{}(key.path.string());
  auto mix = [&](const auto& value) {
    using T = std::decay_t<decltype(value)>;
    const std::size_t h = std::hash<T>{}(value);
    seed ^= h + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
  };
  mix(key.table_name.has_value() ? *key.table_name : std::string{});
  mix(key.time_column.has_value() ? *key.time_column : std::string{});
  mix(key.value_column);
  return seed;
}

} // namespace tsv::app