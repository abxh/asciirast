/**
 * @file Float.h
 * @brief Functions operating on floats
 */

#pragma once

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <type_traits>

#include <external/gcem/include/gcem.hpp>

namespace asciirast::math {

/**
 * @brief Convert from degrees to radians
 *
 * @param degrees The angle in degrees
 * @return The angle in radians
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
[[nodiscard]]
constexpr auto
radians(const T degrees) -> T
{
    return std::numbers::pi_v<T> * degrees / T{ 180 };
}

/**
 * @brief Convert from radians to degrees
 *
 * @param radians The angle in radians
 * @return The angle in degrees
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
[[nodiscard]]
constexpr auto
degrees(const T radians) -> T
{
    return T{ 180 } * radians / std::numbers::pi_v<T>;
}

/**
 * @brief Check if two floating point values are approximately equal,
 *        given a precision for ulps (units in last place).
 *
 * The lower, the more precise --- desirable for small floats.
 * The higher, the less precise --- desirable for large floats
 *
 * @param lhs left hand side of operand
 * @param rhs right hand side of operand
 * @param ulps_ Units in last place
 * @return Whether the two numbers are approximately equal
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
[[nodiscard]]
constexpr auto
almost_equal(const T lhs, const T rhs, const unsigned ulps_) -> bool
{
    // Based on:
    // https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon

    const T ulps = static_cast<T>(ulps_);
    const T min = std::min(std::fabs(lhs), std::fabs(rhs));
    const T exp = min < std::numeric_limits<T>::min() ? std::numeric_limits<T>::min_exponent - 1 : std::ilogb(min);
    const T small_diff = ulps * std::ldexp(std::numeric_limits<T>::epsilon(), static_cast<int>(exp));

    return std::fabs(lhs - rhs) <= small_diff;
}

/**
 * @brief Check two float's are approximately equal with default precision
 *
 * @param lhs left hand side of operand
 * @param rhs right hand side of operand
 * @return Whether the two numbers are approximately equal
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
[[nodiscard]]
constexpr auto
almost_equal(const T lhs, const T rhs) -> bool
    requires(std::is_same_v<T, float>)
{
    // default precision based on:
    // https://en.wikipedia.org/wiki/Single-precision_floating-point_format

    return almost_equal(lhs, rhs, 9);
}

/**
 * @brief Check two double's are approximately equal with default precision
 *
 * @param lhs left hand side of operand
 * @param rhs right hand side of operand
 * @return Whether the two numbers are approximately equal
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
[[nodiscard]]
constexpr auto
almost_equal(const T lhs, const T rhs) -> bool
    requires(std::is_same_v<T, double>)
{
    // default precision based on:
    // https://en.wikipedia.org/wiki/Double-precision_floating-point_format

    return almost_equal(lhs, rhs, 17);
}

/**
 * @brief Check if two floating point values are approximately less than one another,
 *        given a precision for ulps (units in last place).
 *
 * The lower, the more precise --- desirable for small floats.
 * The higher, the less precise --- desirable for large floats
 *
 * @param lhs left hand side of operand
 * @param rhs right hand side of operand
 * @param ulps_ Units in last place
 * @return Whether lhs < rhs, with given precision
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
[[nodiscard]]
constexpr auto
almost_less_than(const T lhs, const T rhs, const unsigned ulps_) -> bool
{
    const T ulps = static_cast<T>(ulps_);
    const T min = std::min(std::fabs(lhs), std::fabs(rhs));
    const T exp = min < std::numeric_limits<T>::min() ? std::numeric_limits<T>::min_exponent - 1 : std::ilogb(min);
    const T small_diff = ulps * std::ldexp(std::numeric_limits<T>::epsilon(), static_cast<int>(exp));

    return lhs - rhs < -small_diff; // lhs < rhs - small_diff
}

/**
 * @brief Check two float's are approximately less than one another with default precision
 *
 * @param lhs left hand side of operand
 * @param rhs right hand side of operand
 * @return Whether lhs < rhs, with default precision
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
[[nodiscard]]
constexpr auto
almost_less_than(const T lhs, const T rhs) -> bool
    requires(std::is_same_v<T, float>)
{
    return almost_less_than(lhs, rhs, 9);
}

/**
 * @brief Check two double's are approximately less than one another with default precision
 *
 * @param lhs left hand side of operand
 * @param rhs right hand side of operand
 * @return Whether lhs < rhs, with default precision
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
[[nodiscard]]
constexpr auto
almost_less_than(const T lhs, const T rhs) -> bool
    requires(std::is_same_v<T, double>)
{
    return almost_less_than(lhs, rhs, 17);
}

/**
 * @brief constexpr sqrt wrapper
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
sqrt(const T x) -> T
{
    if (std::is_constant_evaluated()) {
        if (0 <= x && x < std::numeric_limits<float>().infinity()) {
            return gcem::sqrt(x);
        } else {
            throw std::domain_error("asciirast::math::sqrt() : input is not a positive real");
        }
    } else {
        return std::sqrt(x);
    }
}

/**
 * @brief constexpr sin wrapper
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
sin(const T x) -> T
{
    if (std::is_constant_evaluated()) {
        if (-std::numeric_limits<float>().infinity() < x && x < +std::numeric_limits<float>().infinity()) {
            return gcem::sin(x);
        } else {
            throw std::domain_error("asciirast::math::sin() : input is not a real");
        }
    } else {
        return std::sin(x);
    }
}

/**
 * @brief constexpr cos wrapper
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
cos(const T x) -> T
{
    if (std::is_constant_evaluated()) {
        if (-std::numeric_limits<float>().infinity() < x && x < +std::numeric_limits<float>().infinity()) {
            return gcem::cos(x);
        } else {
            throw std::domain_error("asciirast::math::cos() : input is not a real");
        }
    } else {
        return std::cos(x);
    }
}

/**
 * @brief constexpr tan wrapper
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
tan(const T x) -> T
{
    if (std::is_constant_evaluated()) {
        if (-std::numeric_limits<float>().infinity() < x && x < +std::numeric_limits<float>().infinity()) {
            return gcem::tan(x);
        } else {
            throw std::domain_error("asciirast::math::tan() : input is not a real");
        }
    } else {
        return std::tan(x);
    }
}

/**
 * @brief constexpr asin wrapper
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
asin(const T x) -> T
{
    if (std::is_constant_evaluated()) {
        if (0 <= x && x <= 1) {
            return gcem::asin(x);
        } else {
            throw std::domain_error("asciirast::math::asin() : input is not between 0 and 1");
        }
    } else {
        return std::asin(x);
    }
}

/**
 * @brief constexpr acos wrapper
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
acos(const T x) -> T
{
    if (std::is_constant_evaluated()) {
        if (0 <= x && x <= 1) {
            return gcem::acos(x);
        } else {
            throw std::domain_error("asciirast::math::acos() : input is not between 0 and 1");
        }
    } else {
        return std::acos(x);
    }
}

/**
 * @brief constexpr atan wrapper
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
atan(const T x) -> T
{
    if (std::is_constant_evaluated()) {
        if (-std::numeric_limits<float>().infinity() < x && x < +std::numeric_limits<float>().infinity()) {
            return gcem::atan(x);
        } else {
            throw std::domain_error("asciirast::math::atan() : input is not a real");
        }
    } else {
        return std::atan(x);
    }
}

/**
 * @brief constexpr atan2 wrapper
 */
template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
atan2(const T x, const T y) -> T
{
    if (std::is_constant_evaluated()) {
        if (-std::numeric_limits<float>().infinity() < x && x < +std::numeric_limits<float>().infinity() &&
            -std::numeric_limits<float>().infinity() < y && y < +std::numeric_limits<float>().infinity()) {
            return gcem::atan2(x, y);
        } else {
            throw std::domain_error("asciirast::math::atan2() : inputs are not reals");
        }
    } else {
        return std::atan2(x, y);
    }
}

} // namespace asciirast::math
