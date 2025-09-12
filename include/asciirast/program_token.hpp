/**
 * @file program_token.hpp
 * @brief Special program tokens
 */

#pragma once

#include <ranges>

#include "./detail/StaticPoolGenerator.hpp"

namespace asciirast {

/**
 * @brief Fragment (program) tokens to be emitted to do special procedures
 */
enum class FragmentToken
{
    Discard,
    Keep,
    Syncronize,
};

/**
 * @brief Geometry (program) tokens to be emitted to do special procedures
 */
enum class GeometryToken
{
    Discard,
    Emit,
    EndPrimitive,
};

/**
 * @brief Fragment (program) token generator to be used
 */
using FragmentTokenGenerator = detail::StaticPoolGenerator<FragmentToken, 512, 4>;

static_assert(std::ranges::input_range<FragmentTokenGenerator>);

/**
 * @brief Geometry (program) token generator to be used
 */
using GeometryTokenGenerator = detail::StaticPoolGenerator<GeometryToken, 1024, 1>;

static_assert(std::ranges::input_range<GeometryTokenGenerator>);

} // namespace asciirast
