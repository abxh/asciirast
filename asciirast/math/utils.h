#pragma once

#include <numbers>
#include <type_traits>

namespace asciirast::math {

template<typename T>
static const T PI = std::numbers::pi; // PI constant

template<typename T>
static const T PI_2 = std::numbers::pi + std::numbers::pi; // PI_2 constant

template<typename T>
    requires(std::is_floating_point_v<T>)
T
angle_as_radians(const T degrees)
{
    return PI_2<T> * degrees / T{ 360 };
}

template<typename T>
    requires(std::is_floating_point_v<T>)
T
angle_as_degrees(const T radians)
{
    return T{ 360 } * radians / PI_2<T>;
}

/**
 * @brief Check if equal to floating type value, given a precision for ulps
 * (units in last place).
 *
 * The lower, the more precise --- desirable for small floats.
 * The higher, the less precise --- desirable for large floats
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
bool
almost_equal(const T& x, const T& y, const unsigned ulps_)
{
    // Based on:
    // https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon

    const T ulps = static_cast<T>(ulps_);
    const T min = std::min(std::fabs(x), std::fabs(y));
    const T exp = min < std::numeric_limits<T>::min() ? std::numeric_limits<T>::min_exponent - 1 : std::ilogb(min);

    return std::fabs(x - y) <= ulps * std::ldexp(std::numeric_limits<T>::epsilon(), exp);
}

/**
 * @brief Check if almost equal to another floating value with default
 * precision.
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
bool
almost_equal(const T& x, const T& y)
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
almost_equal(const T& x, const T& y)
    requires(std::is_same_v<T, double>)
{
    // default precision based on:
    // https://en.wikipedia.org/wiki/Double-precision_floating-point_format

    return almost_equal(x, y, 17);
}

/**
 * @brief Check if less than floating type value, given a precision for ulps
 * (units in last place).
 *
 * The lower, the more precise --- desirable for small floats.
 * The higher, the less precise --- desirable for large floats
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
bool
almost_less_than(const T& x, const T& y, const unsigned ulps_)
{
    const T ulps = static_cast<T>(ulps_);
    const T min = std::min(std::fabs(x), std::fabs(y));
    const T exp = min < std::numeric_limits<T>::min() ? std::numeric_limits<T>::min_exponent - 1 : std::ilogb(min);

    return x - y < -ulps * std::ldexp(std::numeric_limits<T>::epsilon(), exp);
}

/**
 * @brief Check if almost less than another floating value with default
 * precision.
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
bool
almost_less_than(const T& x, const T& y)
    requires(std::is_same_v<T, float>)
{
    // default precision based on:
    // https://en.wikipedia.org/wiki/Single-precision_floating-point_format

    return almost_less_than(x, y, 9);
}

/**
 * @brief Check if almost less than another floating value with default
 * precision.
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
bool
almost_less_than(const T& x, const T& y)
    requires(std::is_same_v<T, double>)
{
    // default precision based on:
    // https://en.wikipedia.org/wiki/Double-precision_floating-point_format

    return almost_less_than(x, y, 17);
}

} // namespace asciirast::math
