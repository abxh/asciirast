/**
 * @file utils.h
 * @brief Miscellaneous functions operating on floats
 */

#pragma once

#include <cmath>
#include <numbers>
#include <type_traits>

namespace asciirast::math {

/**
 * @brief Convert from degrees to radians
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
radians(const T degrees) -> T
{
    return std::numbers::pi_v<T> * degrees / T{ 180 };
}

/**
 * @brief Convert from radians to degrees
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
degrees(const T radians) -> T
{
    return T{ 180 } * radians / std::numbers::pi_v<T>;
}

/**
 * @brief Check if equal to floating type value, given a precision for
 *        ulps (units in last place).
 *
 * The lower, the more precise --- desirable for small floats.
 * The higher, the less precise --- desirable for large floats
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
almost_equal(const T x, const T y, const unsigned ulps_) -> bool
{
    // Based on:
    // https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon

    const T ulps = static_cast<T>(ulps_);
    const T min = std::min(std::fabs(x), std::fabs(y));
    const T exp = min < std::numeric_limits<T>::min() ? std::numeric_limits<T>::min_exponent - 1 : std::ilogb(min);
    const T small_diff = ulps * std::ldexp(std::numeric_limits<T>::epsilon(), static_cast<int>(exp));

    return std::fabs(x - y) <= small_diff;
}

/**
 * @brief Check if almost equal to another floating value with default
 *        precision.
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
almost_equal(const T x, const T y) -> bool
    requires(std::is_same_v<T, float>)
{
    // default precision based on:
    // https://en.wikipedia.org/wiki/Single-precision_floating-point_format

    return almost_equal(x, y, 9);
}

/**
 * @brief Check if almost equal to another floating value with default
 *        precision.
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
almost_equal(const T x, const T y) -> bool
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
constexpr auto
almost_less_than(const T x, const T y, const unsigned ulps_) -> bool
{
    const T ulps = static_cast<T>(ulps_);
    const T min = std::min(std::fabs(x), std::fabs(y));
    const T exp = min < std::numeric_limits<T>::min() ? std::numeric_limits<T>::min_exponent - 1 : std::ilogb(min);
    const T small_diff = ulps * std::ldexp(std::numeric_limits<T>::epsilon(), static_cast<int>(exp));

    return x - y < -small_diff; // x < y - small_diff
}

/**
 * @brief Check if almost less than another floating value with default
 *        precision.
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
almost_less_than(const T x, const T y) -> bool
    requires(std::is_same_v<T, float>)
{
    return almost_less_than(x, y, 9);
}

/**
 * @brief Check if almost less than another floating value with default
 *        precision.
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
almost_less_than(const T x, const T y) -> bool
    requires(std::is_same_v<T, double>)
{
    return almost_less_than(x, y, 17);
}

namespace detail {

template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
sqrt_newton_raphson(const T x, const T curr, const T prev) -> T
{
    // constexpr sqrt:
    // https://stackoverflow.com/a/34134071

    return curr == prev ? curr : sqrt_newton_raphson<T>(x, T{ 0.5 } * (curr + x / curr), curr);
}

};

template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
sqrt(const T x) -> T
{
    if (std::is_constant_evaluated()) {
        return x >= 0 && x < std::numeric_limits<T>::infinity() ? detail::sqrt_newton_raphson<T>(x, x, 0)
                                                                : std::numeric_limits<T>::quiet_NaN();
    } else {
        return std::sqrt(x);
    }
}

} // namespace asciirast::math
