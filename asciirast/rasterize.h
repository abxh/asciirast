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

// static std::optional<std::tuple<asciirast::math::F, asciirast::math::F>>
// clip_line(const math::Vec4& p0, const math::Vec4& p1)
// {
//     // Liang-Barsky clipping algorithm:
//
//     using T = asciirast::math::F;
//
//     if (p0.w < 0 && p1.w < 0) {
//         return {};
//     }
//
//     T t0 = 0;
//     T t1 = 1;
//
//     const T dx = p1.x - p0.x;
//     const T dy = p1.y - p0.y;
//     const T dz = p1.z - p0.z;
//
//     const std::array<T, 6> q = {
//         p0.x - min.x, max.x - p0.x, p0.y - min.y, max.y - p0.y, p0.z - min.z, max.z - p0.z,
//     };
// }

}
