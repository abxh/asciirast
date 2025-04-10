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
 * @brief Default depth when z = 0
 *
 * This depth is given, when 1/z is non-finite.
 */
static constexpr auto DEFAULT_DEPTH = +std::numeric_limits<math::Float>::infinity();

}; // namespace asciirast
