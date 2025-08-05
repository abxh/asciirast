/**
 * @file has_minus_operator.h
 * @brief Has minus operator concept
 *
 * @todo Remove this, when the bug to have variadic concepts is fixed.
 */

#pragma once

#include <concepts>

/// @cond DO_NOT_DOCUMENT
namespace detail {

template<typename T>
concept has_minus_operator = requires(T t) {
    { t - t } -> std::same_as<T>;
};

};
/// @endcond
