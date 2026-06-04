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

/// Interpolates a value from a series at a given time point using linear interpolation.
/// @param series  The source series data.
/// @param x       The time point to interpolate at.
/// @return The interpolated value (clamped to series bounds if x is outside the range).
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

/// Remaps a series' values onto a target time grid using interpolation.
/// @param series      The source series data.
/// @param target_time The target time grid to interpolate onto.
/// @return A new SeriesData with the target time grid and interpolated values.
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

/// Applies a binary operation element-wise between two series (aligned to lhs time grid).
/// @param lhs   Left-hand series.
/// @param rhs   Right-hand series (will be remapped to lhs time grid).
/// @param op    Binary operation to apply.
/// @param name  Name for the result series.
/// @return A new SeriesData with the operation result.
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

/// Applies a binary operation between a series and a scalar value.
/// @param lhs   The series.
/// @param rhs   The scalar value.
/// @param op    Binary operation to apply.
/// @param name  Name for the result series.
/// @return A new SeriesData with the operation result.
inline SeriesData apply_series_scalar(const SeriesData& lhs, double rhs, const std::function<double(double, double)>& op, const std::string& name) {
  SeriesData result = lhs;
  result.name = name;
  for (auto& value : result.value) value = op(value, rhs);
  return result;
}

/// Applies a binary operation between a scalar value and a series.
/// @param lhs   The scalar value.
/// @param rhs   The series.
/// @param op    Binary operation to apply.
/// @param name  Name for the result series.
/// @return A new SeriesData with the operation result.
inline SeriesData apply_scalar_series(double lhs, const SeriesData& rhs, const std::function<double(double, double)>& op, const std::string& name) {
  SeriesData result = rhs;
  result.name = name;
  for (auto& value : result.value) value = op(lhs, value);
  return result;
}

/// Adds two series element-wise (series + series).
inline SeriesData series_add(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return a + b; }, lhs.name + "_plus_" + rhs.name);
}
/// Subtracts two series element-wise (series - series).
inline SeriesData series_sub(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return a - b; }, lhs.name + "_minus_" + rhs.name);
}
/// Multiplies two series element-wise (series * series).
inline SeriesData series_mul(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return a * b; }, lhs.name + "_times_" + rhs.name);
}
/// Divides two series element-wise (series / series). Division by zero yields 0.0.
inline SeriesData series_div(const SeriesData& lhs, const SeriesData& rhs) {
  return apply_series_series(lhs, rhs, [](double a, double b) { return b == 0.0 ? 0.0 : a / b; }, lhs.name + "_div_" + rhs.name);
}
/// Adds a scalar to a series (series + scalar).
inline SeriesData series_add(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return a + b; }, lhs.name + "_plus_scalar");
}
/// Subtracts a scalar from a series (series - scalar).
inline SeriesData series_sub(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return a - b; }, lhs.name + "_minus_scalar");
}
/// Multiplies a series by a scalar (series * scalar).
inline SeriesData series_mul(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return a * b; }, lhs.name + "_times_scalar");
}
/// Divides a series by a scalar (series / scalar). Division by zero yields 0.0.
inline SeriesData series_div(const SeriesData& lhs, double rhs) {
  return apply_series_scalar(lhs, rhs, [](double a, double b) { return b == 0.0 ? 0.0 : a / b; }, lhs.name + "_div_scalar");
}
/// Adds a scalar to a series (scalar + series).
inline SeriesData series_add(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return a + b; }, rhs.name + "_scalar_plus");
}
/// Subtracts a series from a scalar (scalar - series).
inline SeriesData series_sub(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return a - b; }, rhs.name + "_scalar_minus");
}
/// Multiplies a scalar by a series (scalar * series).
inline SeriesData series_mul(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return a * b; }, rhs.name + "_scalar_times");
}
/// Divides a scalar by a series (scalar / series). Division by zero yields 0.0.
inline SeriesData series_div(double lhs, const SeriesData& rhs) {
  return apply_scalar_series(lhs, rhs, [](double a, double b) { return b == 0.0 ? 0.0 : a / b; }, rhs.name + "_scalar_div");
}

/// Overrides the name of a series.
/// @param series  The series to rename.
/// @param name    The new name.
/// @return The series with the updated name.
inline SeriesData normalize_series_name(SeriesData series, const std::string& name) {
  series.name = name;
  return series;
}

/// Evaluates a Lua expression against a series registry.
/// The expression can reference series by name via the `series("name")` function
/// and supports arithmetic operators (+, -, *, /) between series and scalars.
/// @param expression  The Lua expression string to evaluate.
/// @param registry    The series registry providing named series references.
/// @return The resulting SeriesData.
/// @throws std::runtime_error if the expression is invalid, references unknown series, or evaluates to an unsupported type.
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