/**
 * @file assert.h
 * @brief assert macro definition
 */

#pragma once

#ifndef NDEBUG
#include <external/libassert/include/libassert/assert.hpp>
#define ASCIIRAST_ASSERT(...) DEBUG_ASSERT(__VA_ARGS__)
#else
#include <cassert>
#define ASCIIRAST_ASSERT(...) assert(...)
#endif
