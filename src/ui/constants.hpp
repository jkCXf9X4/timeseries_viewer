#pragma once

#include <cstdint>

namespace tsv::ui::detail {

constexpr std::uint32_t kFixedSidebarFlags = 0x1u | 0x4u | 0x10u;
constexpr std::uint32_t kStatusBarFlags = 0x1u | 0x4u | 0x10u;
constexpr float kStatusBarHeight = 28.0f;

} // namespace tsv::ui::detail