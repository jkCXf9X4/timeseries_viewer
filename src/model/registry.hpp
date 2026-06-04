#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "model/types.hpp"

namespace tsv {

struct SeriesRegistry {
  void set(SeriesData series) {
    const auto key = series.name;
    entries_[key] = std::move(series);
  }

  [[nodiscard]] bool contains(const std::string& name) const {
    return entries_.find(name) != entries_.end();
  }

  [[nodiscard]] const SeriesData& at(const std::string& name) const {
    return entries_.at(name);
  }

  [[nodiscard]] std::vector<std::string> names() const {
    std::vector<std::string> result;
    result.reserve(entries_.size());
    for (const auto& [key, _] : entries_) {
      result.push_back(key);
    }
    std::sort(result.begin(), result.end());
    return result;
  }

 private:
  std::unordered_map<std::string, SeriesData> entries_;
};

} // namespace tsv