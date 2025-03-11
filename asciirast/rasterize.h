#include "./math.h"

namespace asciirast {

static bool
cull_point(const math::Vec4& p)
{
    const bool x_in_bounds = -p.w <= p.x && p.x <= +p.w;
    const bool y_in_bounds = -p.w <= p.y && p.y <= +p.w;
    const bool z_in_bounds = -p.w <= p.z && p.z <= +p.w;

    return !(x_in_bounds && y_in_bounds && z_in_bounds);
}

}
