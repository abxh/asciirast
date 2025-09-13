/**
 * @file assert.hpp
 * @brief custom assert macro definition
 */

#pragma once

// #ifndef NDEBUG

#ifndef ASCIIRAST_ASSERT

// #include <libassert/assert.hpp>
// #define ASCIIRAST_ASSERT(...) DEBUG_ASSERT(__VA_ARGS__)

#include <cassert>
#define ASCIIRAST_ASSERT(arg, msg, ...) assert((arg) && (msg))

#endif

// #else
//
// #ifndef ASCIIRAST_ASSERT
// #define ASCIIRAST_ASSERT(...)
// #endif
//
// #endif
