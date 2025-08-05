/**
 * @file has_minus_operator.h
 * @brief Has minus operator concept
 *
 * @todo Remove this, when the bug to have variadic concepts is fixed.
 *
 * See:
 * @link https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105644
 */

#pragma once

#include <concepts>

namespace asciirast {

/// @cond DO_NOT_DOCUMENT
namespace detail {

template<typename T>
concept has_minus_operator = requires(T t) {
    { t - t } -> std::same_as<T>;
};

};
/// @endcond

}
