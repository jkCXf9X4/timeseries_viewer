#pragma once

#include <cstdint>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>

namespace tsv::test {

namespace fs = std::filesystem;

inline fs::path make_temp_dir(std::string_view prefix) {
  static std::atomic_uint64_t counter{0};
  const auto stamp = static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
  const auto unique = std::to_string(stamp) + "_" + std::to_string(counter.fetch_add(1, std::memory_order_relaxed));
  const auto base = fs::temp_directory_path() / (std::string(prefix) + "_" + unique);
  fs::create_directories(base);
  return base;
}

} // namespace tsv::test
