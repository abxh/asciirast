/**
 * @file GeneratorInterface.hpp
 * @brief Generator interface concept
 */

#pragma once

#include <concepts>
#include <generator>

namespace asciirast {

/// @cond DO_NOT_DOCUMENT
namespace detail {

template<typename G, typename T>
concept GeneratorInterface =
    requires { typename G::promise_type; } &&
    std::same_as<typename std::remove_cvref_t<decltype(std::declval<G>().begin())>::value_type, T> &&
    std::sentinel_for<decltype(std::declval<G>().end()), decltype(std::declval<G>().begin())>;

static_assert(GeneratorInterface<std::generator<int>, int>);

}; // namespace detail
/// @endcond

}; // namespace asciirast
