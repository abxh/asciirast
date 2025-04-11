#pragma once

#include "./math/types.h"

namespace asciirast {

/**
 * @brief Screen boundary
 *
 * Verticies outside of this boundary are not shown.
 */
static constexpr auto SCREEN_BOUNDS = math::AABB2D::from_min_max({ -1, -1 }, { +1, +1 });

/**
 * @brief Depth for the farthest object
 */
static constexpr math::Float MIN_DEPTH = 0;

/**
 * @brief Depth for the nearest object
 *
 * Default depth when z = 0. This depth is given, when 1/z is non-finite.
 */
static constexpr math::Float MAX_DEPTH = 1;

/**
 * @brief Default depth for depth buffer
 */
static constexpr math::Float DEFAULT_DEPTH = 2;

static_assert(DEFAULT_DEPTH > MAX_DEPTH, "first pixel is always accepted");

}; // namespace asciirast
