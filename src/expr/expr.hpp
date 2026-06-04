#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include <lua.hpp>
#include <sol/sol.hpp>

#include "model/types.hpp"
#include "model/registry.hpp"
#include "model/utility.hpp"

namespace tsv {

inline double interpolate_value(const SeriesData& series, double x) {
  if (series.time.empty()) return 0.0;
  if (series.time.size() == 1) return series.value.front();
  if (x <= series.time.front()) return series.value.front();
  if (x >= series.time.back()) return series.value.back();

  const auto upper = std::upper_bound(series.time.begin(), series.time.end(), x);
  const std::size_t idx = static_cast<std::size_t>(std::distance(series.time.begin(), upper));
  const std::size_t left = idx - 1;
  const std::size_t right = idx;
  const double x0 = series.time[left];
  const double x1 = series.time[right];
  const double y0 = series.value[left];
  const double y1 = series.value[right];
  if (x1 == x0) return y0;
  return y0 + (x - x0) / (x1 - x0) * (y1 - y0);
}

inline SeriesData remap_to_grid(const SeriesData& series, const std::vector<double>& target_time) {
  SeriesData remapped;
  remapped.name = series.name;
  remapped.source_name = series.source_name;
  remapped.table_name = series.table_name;
  remapped.time_column = series.time_column;
  remapped.value_column = series.value_column;
  remapped.time = target_time;
  remapped.value.reserve(target_time.size());
  for (const auto x : target_time) {
    remapped.value.push_back(interpolate_value(series, x));
  }
  return remapped;
}

inline SeriesData apply_series_series(const SeriesData& lhs, const SeriesData& rhs, const std::function<double(double, double)>& op, const std::string& name) {
  SeriesData result;
  result.name = name;
  result.source_name = lhs.source_name.empty() ? rhs.source_name : lhs.source_name;
  result.table_name = lhs.table_name;
  result.time = lhs.time;
  const auto remapped = remap_to_grid(rhs, lhs.time);
  result.value.reserve(lhs.value.size());
  for (std::size_t i = 0; i < lhs.value.size(); ++i) {
    result.value.push_back(op(lhs.value[i], remapped.value[i]));
  }
  return result;
}

inline SeriesData apply_series_scalar(const SeriesData& lhs, double rhs, const std::function<double(double, double)>& op, const std::string& name) {
  SeriesData result = lhs;
  result.name = name;
  for (auto& value : result.value) value = op(value, rhs);
  return result;
}

inline SeriesData apply_scalar_series(double lhs, const SeriesData& rhs, const std::function<double(double, double)>& op, const std::string& name) {
  SeriesData result = rhs;
  result.name = name;
  for (auto& value : result.value) value = op(lhs, value);
  return result;
}

inline SeriesData series_add(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return a + b; }, lhs.name + "_plus_" + rhs.name);
}
inline SeriesData series_sub(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return a - b; }, lhs.name + "_minus_" + rhs.name);
}
inline SeriesData series_mul(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return a * b; }, lhs.name + "_times_" + rhs.name);
}
inline SeriesData series_div(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return b == 0.0 ? 0.0 : a / b; }, lhs.name + "_div_" + rhs.name);
}
inline SeriesData series_add(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return a + b; }, lhs.name + "_plus_scalar");
}
inline SeriesData series_sub(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return a - b; }, lhs.name + "_minus_scalar");
}
inline SeriesData series_mul(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return a * b; }, lhs.name + "_times_scalar");
}
inline SeriesData series_div(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return b == 0.0 ? 0.0 : a / b; }, lhs.name + "_div_scalar");
}
inline SeriesData series_add(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return a + b; }, rhs.name + "_scalar_plus");
}
inline SeriesData series_sub(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return a - b; }, rhs.name + "_scalar_minus");
}
inline SeriesData series_mul(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return a * b; }, rhs.name + "_scalar_times");
}
inline SeriesData series_div(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return b == 0.0 ? 0.0 : a / b; }, rhs.name + "_scalar_div");
}

inline SeriesData normalize_series_name(SeriesData series, const std::string& name) {
  series.name = name;
  return series;
}

inline SeriesData evaluate_expression(const std::string& expression, const SeriesRegistry& registry) {
  sol::state lua;
  lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);

  lua.new_usertype<SeriesData>(
    "SeriesData",
    sol::constructors<SeriesData()>(),
    "time", &SeriesData::time,
    "value", &SeriesData::value,
    "name", &SeriesData::name,
    sol::meta_function::addition, sol::overload(
      [](const SeriesData& lhs, const SeriesData& rhs) { return series_add(lhs, rhs); },
      [](const SeriesData& lhs, double rhs) { return series_add(lhs, rhs); },
      [](double lhs, const SeriesData& rhs) { return series_add(lhs, rhs); }
    ),
    sol::meta_function::subtraction, sol::overload(
      [](const SeriesData& lhs, const SeriesData& rhs) { return series_sub(lhs, rhs); },
      [](const SeriesData& lhs, double rhs) { return series_sub(lhs, rhs); },
      [](double lhs, const SeriesData& rhs) { return series_sub(lhs, rhs); }
    ),
    sol::meta_function::multiplication, sol::overload(
      [](const SeriesData& lhs, const SeriesData& rhs) { return series_mul(lhs, rhs); },
      [](const SeriesData& lhs, double rhs) { return series_mul(lhs, rhs); },
      [](double lhs, const SeriesData& rhs) { return series_mul(lhs, rhs); }
    ),
    sol::meta_function::division, sol::overload(
      [](const SeriesData& lhs, const SeriesData& rhs) { return series_div(lhs, rhs); },
      [](const SeriesData& lhs, double rhs) { return series_div(lhs, rhs); },
      [](double lhs, const SeriesData& rhs) { return series_div(lhs, rhs); }
    )
  );

  lua.set_function("series", [&](const std::string& name) -> SeriesData {
    if (!registry.contains(name)) throw std::runtime_error("Unknown series: " + name);
    return registry.at(name);
  });

  const auto result = lua.safe_script("return " + expression, sol::script_pass_on_error);
  if (!result.valid()) {
    const sol::error err = result;
    throw std::runtime_error("Expression error: " + std::string(err.what()));
  }

  if (result.get_type() == sol::type::userdata) {
    return result.get<SeriesData>();
  }

  if (result.get_type() == sol::type::number) {
    const auto value = result.get<double>();
    if (registry.names().empty()) throw std::runtime_error("Scalar expressions require at least one reference series");
    const auto& reference = registry.at(registry.names().front());
    SeriesData series;
    series.name = expression;
    series.time = reference.time;
    series.value.assign(reference.time.size(), value);
    return series;
  }

  throw std::runtime_error("Expression did not evaluate to a series: " + expression);
}

} // namespace tsv