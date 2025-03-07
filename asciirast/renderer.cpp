
#include "renderer.h"
#include <cassert>

namespace asciirast {

bool
Renderer::cull_point(const math::Vec2 p) const
{
    const bool x_in_bounds = -1.f <= p.x && p.x <= 1.f;
    const bool y_in_bounds = -1.f <= p.y && p.y <= 1.f;

    return !(x_in_bounds && y_in_bounds);
}

};
