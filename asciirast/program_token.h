/**
 * @file program_token.h
 * @brief Special program tokens
 */

#pragma once

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

};
