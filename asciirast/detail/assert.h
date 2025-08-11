/**
 * @file assert.h
 * @brief custom assert macro definition
 */

#pragma once

#ifndef NDEBUG

#ifndef ASCIIRAST_ASSERT

#include "../../external/libassert/include/libassert/assert.hpp"
#define ASCIIRAST_ASSERT(...) DEBUG_ASSERT(__VA_ARGS__)

#endif

// #include <cassert>
// #define ASCIIRAST_ASSERT(arg, ...) assert(arg)

#else

#ifndef ASCIIRAST_ASSERT
#define ASCIIRAST_ASSERT(...)
#endif

#endif
