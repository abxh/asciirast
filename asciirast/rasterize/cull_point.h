#pragma once

#include "./../math.h"

namespace asciirast::rasterize {

static bool
cull_point(const math::Vec2& p)
{
    const bool x_in_bounds = -1 <= p.x && p.x <= +1;
    const bool y_in_bounds = -1 <= p.y && p.y <= +1;

    return !(x_in_bounds && y_in_bounds);
}

static bool
cull_point(const math::Vec4& p)
{
    const bool x_in_bounds = -p.w <= p.x && p.x <= +p.w;
    const bool y_in_bounds = -p.w <= p.y && p.y <= +p.w;
    const bool z_in_bounds = -p.w <= p.z && p.z <= +p.w;

    return !(x_in_bounds && y_in_bounds && z_in_bounds);
}

}
