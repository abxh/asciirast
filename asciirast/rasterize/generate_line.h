// Bresenham's Line Algorithm
// - https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
// - https://www.youtube.com/watch?v=CceepU1vIKo

#pragma once

#include <cassert>
#include <generator>

#include "../math.h"

namespace asciirast::rasterize {

static std::generator<math::Vec2>
generate_horizontal_line(const math::Vec2& v0, const math::Vec2& size)
{
    assert(size.x >= 0);
    assert(size.y == 0);

    math::Vec2 v = v0;

    for (math::I i = 0; i < static_cast<math::I>(size.x); i++) {
        co_yield v;

        v.x += 1;
    }
}

static std::generator<math::Vec2>
generate_vertical_line(const math::Vec2& v0, const math::Vec2& size)
{
    assert(size.x == 0);
    assert(size.y >= 0);

    math::Vec2 v = v0;

    for (math::I i = 0; i < static_cast<math::I>(size.y); i++) {
        co_yield v;

        v.y += 1;
    }
}

static std::generator<math::Vec2>
generate_steep_line(const math::Vec2& v0, const math::Vec2& v1, const math::Vec2& size)
{
    const auto delta = v1 - v0;
    const auto dir = delta.x < 0 ? -1 : 1;

    assert(delta.y >= 0);
    assert(size.y >= size.x);

    math::Vec2 v = v0;
    math::F D = size.y - 2 * size.x; // "Decision" variable choosing based on next midpoint

    for (math::I i = 0; i < static_cast<math::I>(size.y); i++) {
        co_yield v;

        if (D < 0) {
            v.x += dir;
            D += 2 * size.y;
        }
        D -= 2 * size.x;
        v.y += 1;
    }
}

static std::generator<math::Vec2>
generate_shallow_line(const math::Vec2& v0, const math::Vec2& v1, const math::Vec2& size)
{
    const auto delta = v1 - v0;
    const auto dir = delta.y < 0 ? -1 : 1;

    assert(delta.x >= 0);
    assert(size.x >= size.y);

    math::Vec2 v = v0;
    math::F D = size.x - 2 * size.y; // "Decision" variable choosing based on next midpoint

    for (math::I i = 0; i < static_cast<math::I>(size.x); i++) {
        co_yield v;

        if (D < 0) {
            v.y += dir;
            D += 2 * size.x;
        }
        D -= 2 * size.y;
        v.x += 1;
    }
}

static std::generator<math::Vec2>
generate_line(const math::Vec2& delta, const math::Vec2& size, const math::Vec2& v0, const math::Vec2& v1)
{
    // to be passed by reference by callee:
    // - delta: v1 - v0
    // - size: abs(v1 - v0)

    if (size.x >= size.y) {
        if (size.y == 0) {
            return (delta.x >= 0) ? generate_horizontal_line(v0, size) : generate_horizontal_line(v1, size);
        } else {
            return (delta.x >= 0) ? generate_shallow_line(v0, v1, size) : generate_shallow_line(v1, v0, size);
        }
    } else {
        if (size.x == 0) {
            return (delta.y >= 0) ? generate_vertical_line(v0, size) : generate_vertical_line(v1, size);
        } else {
            return (delta.y >= 0) ? generate_steep_line(v0, v1, size) : generate_steep_line(v1, v0, size);
        }
    }
}

}
