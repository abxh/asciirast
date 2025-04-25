/**
 * @file utils.h
 * @brief Miscellaneous functions operating on floats
 */

#pragma once

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <type_traits>

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

/// @cond DO_NOT_DOCUMENT
namespace detail {

// square root with newton-raphson
// -------------------------------

template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
sqrt_newton_raphson(const T x, const T curr, const T prev) -> T
{
    // constexpr sqrt:
    // https://stackoverflow.com/a/34134071

    return almost_equal(curr, prev) ? curr : sqrt_newton_raphson<T>(x, T{ 0.5 } * (curr + x / curr), curr);
}

template<typename T>
    requires(std::is_floating_point_v<T>)
constexpr auto
sqrt(const T x) -> T
{
    if (std::is_constant_evaluated()) {
        if (0 <= x && x < std::numeric_limits<T>::infinity()) {
            return detail::sqrt_newton_raphson<T>(x, x, 0);
        } else {
            throw std::domain_error("asciirast::math::details::sqrt() : sqrt value is not a positive real");
        }
    } else {
        return std::sqrt(x);
    }
}

};
/// @endcond

} // namespace asciirast::math
