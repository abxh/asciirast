/*
    Copyright (C) 2025 abxh

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

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
