/**
 * @file utils.h
 * @brief Miscellaneous functions operating on floats
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>
#include <type_traits>

namespace asciirast::math {

/**
 * @brief Check if type conversion is a non-narrowing conversion.
 */
template<typename From, typename To>
concept non_narrowing_conversion = requires(From f) { To{ f }; };

/**
 * @brief Convert from degrees to radians
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
static T
radians(const T degrees)
{
    return std::numbers::pi_v<T> * degrees / T{ 180 };
}

/**
 * @brief Convert from radians to degrees
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
static T
degrees(const T radians)
{
    return T{ 180 } * radians / std::numbers::pi_v<T>;
}

/**
 * @brief Check if equal to floating type value, given a precision for
 * ulps (units in last place).
 *
 * The lower, the more precise --- desirable for small floats.
 * The higher, the less precise --- desirable for large floats
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
bool
almost_equal(const T x, const T y, const unsigned ulps_)
{
    // Based on:
    // https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon

    const T ulps = static_cast<T>(ulps_);
    const T min  = std::min(std::fabs(x), std::fabs(y));
    const T exp  = min < std::numeric_limits<T>::min() ? std::numeric_limits<T>::min_exponent - 1 : std::ilogb(min);
    const T small_diff = ulps * std::ldexp(std::numeric_limits<T>::epsilon(), static_cast<int>(exp));

    return std::fabs(x - y) <= small_diff;
}

/**
 * @brief Check if almost equal to another floating value with default
 * precision.
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
bool
almost_equal(const T x, const T y)
    requires(std::is_same_v<T, float>)
{
    // default precision based on:
    // https://en.wikipedia.org/wiki/Single-precision_floating-point_format

    return almost_equal(x, y, 9);
}

/**
 * @brief Check if almost equal to another floating value with default
 * precision.
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
bool
almost_equal(const T x, const T y)
    requires(std::is_same_v<T, double>)
{
    // default precision based on:
    // https://en.wikipedia.org/wiki/Double-precision_floating-point_format

    return almost_equal(x, y, 17);
}

/**
 * @brief Check if less than floating type value, given a precision for
 * ulps (units in last place).
 *
 * The lower, the more precise --- desirable for small floats.
 * The higher, the less precise --- desirable for large floats
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
bool
almost_less_than(const T x, const T y, const unsigned ulps_)
{
    const T ulps = static_cast<T>(ulps_);
    const T min  = std::min(std::fabs(x), std::fabs(y));
    const T exp  = min < std::numeric_limits<T>::min() ? std::numeric_limits<T>::min_exponent - 1 : std::ilogb(min);
    const T small_diff = ulps * std::ldexp(std::numeric_limits<T>::epsilon(), static_cast<int>(exp));

    return x - y < -small_diff; // x < y - small_diff
}

/**
 * @brief Check if almost less than another floating value with default
 * precision.
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
bool
almost_less_than(const T x, const T y)
    requires(std::is_same_v<T, float>)
{
    return almost_less_than(x, y, 9);
}

/**
 * @brief Check if almost less than another floating value with default
 * precision.
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
bool
almost_less_than(const T x, const T y)
    requires(std::is_same_v<T, double>)
{
    return almost_less_than(x, y, 17);
}

} // namespace asciirast::math
