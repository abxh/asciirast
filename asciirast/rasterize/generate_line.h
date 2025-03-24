// DDA Line algorithm:
// - https://www.redblobgames.com/grids/line-drawing/#more
// -
// https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)

#pragma once

#include <cassert>
#include <generator>

#include "../math.h"

namespace asciirast::rasterize {

static std::generator<math::Vec2>
generate_line(const math::F& len, const math::F& len_inv, const math::Vec2& v0, const math::Vec2& v1)
{
    auto acc = v0;
    auto inc = (v1 - v0) * len_inv;

    for (math::I i = 0; i < static_cast<math::I>(len); i++) {
        co_yield math::floor(acc);
        acc += inc;
    }
}

} // namespace asciirast::rasterize
