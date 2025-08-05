/**
 * @file program_token.h
 * @brief Special program tokens
 */

#pragma once

#include <ranges>

#include "./detail/StaticPoolGenerator.h"

namespace asciirast {

/**
 * @brief Program tokens to be emitted to do special procedures
 */
enum class ProgramToken
{
    Discard,
    Keep,
    Syncronize,
};

/**
 * @brief Program token generator to be used
 */
using ProgramTokenGenerator = detail::StaticPoolGenerator<ProgramToken, 512, 4>;

static_assert(std::ranges::input_range<ProgramTokenGenerator>);

}
