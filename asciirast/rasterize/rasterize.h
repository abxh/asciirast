#pragma once

#include <cassert>
#include <cfloat>

#include "../math/types.h"
#include "../program.h"
#include "./interpolate.h"

namespace asciirast::rasterize {

template<VaryingInterface Varying, typename Plot, typename TestAndSetDepth>
    requires(std::is_invocable_v<Plot, ProjectedFragment<Varying>> &&
             std::is_invocable_r_v<bool, TestAndSetDepth, math::Vec2, math::Float>)
[[maybe_unused]]
static void
rasterize_line(const ProjectedFragment<Varying>& proj0,
               const ProjectedFragment<Varying>& proj1,
               const Plot plot,
               const TestAndSetDepth test_and_set_depth)
{
    // Modified DDA Line algorithm:
    // https://www.redblobgames.com/grids/line-drawing/#more
    // https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)

    const auto [v0, depth0, Z_inv0, attrs0] = proj0;
    const auto [v1, depth1, Z_inv1, attrs1] = proj1;

    const auto delta = v1 - v0;
    const auto size = math::abs(delta);
    const auto len = std::max<math::Float>(size.x, size.y);
    if (len == 0) {
        return;
    }
    const auto len_inv = 1 / len;

    const auto inc_t = len_inv;
    const auto inc_v = (v1 - v0) * len_inv;
    const auto inc_Z_inv = (Z_inv1 - Z_inv0) * len_inv;

    auto acc_t = math::Float{ 0 };
    auto acc_v = v0;
    auto acc_Z_inv = Z_inv0;

    const auto func = [&depth0, &depth1, &Z_inv0, &Z_inv1, &attrs0, &attrs1](
                              const math::Float& acc_t_,
                              const math::Vec2& acc_v_,
                              const math::Float& acc_Z_inv_) -> ProjectedFragment<Varying> {
        return ProjectedFragment<Varying>{
            .pos = math::floor(acc_v_),
            .depth = lerp_varying_perspective_corrected(depth0, depth1, acc_t_, Z_inv0, Z_inv1, acc_Z_inv_),
            .Z_inv = acc_Z_inv_,
            .attrs = lerp_varying_perspective_corrected(attrs0, attrs1, acc_t_, Z_inv0, Z_inv1, acc_Z_inv_)
        };
    };

    for (std::size_t i = 0; i < static_cast<std::size_t>(len); i++) {
        const ProjectedFragment<Varying> pfrag = func(acc_t, acc_v, acc_Z_inv);

        if (test_and_set_depth(pfrag.pos, pfrag.depth)) {
            plot(pfrag);
        }

        acc_t += inc_t;
        acc_v += inc_v;
        acc_Z_inv += inc_Z_inv;
    }
}

[[maybe_unused]]
static auto
is_top_left_edge_of_triangle(const math::Vec2& src, const math::Vec2& dest) -> bool
{
    const auto edge = src.vector_to(dest);

    // note: (y > 0) since y-axis points up

    const bool points_right = math::almost_less_than<math::Float>(0, edge.x); // 0 < x
    const bool points_up = math::almost_less_than<math::Float>(0, edge.y);    // 0 < y

    const bool is_top_edge = math::almost_equal<math::Float>(0, edge.y) && points_right;
    const bool is_left_edge = points_up;

    return is_top_edge || is_left_edge;
}

template<VaryingInterface Varying, typename Plot, typename TestAndSetDepth>
    requires(std::is_invocable_v<Plot, ProjectedFragment<Varying>> &&
             std::is_invocable_r_v<bool, TestAndSetDepth, math::Vec2, math::Float>)
[[maybe_unused]]
static void
rasterize_triangle(const ProjectedFragment<Varying>& proj0,
                   const ProjectedFragment<Varying>& proj1,
                   const ProjectedFragment<Varying>& proj2,
                   const Plot plot,
                   const TestAndSetDepth test_and_set_depth)
{
    // Modified algorithm which uses cross products
    // and bayesian coordinates for triangles:
    // - https://www.youtube.com/watch?v=k5wtuKWmV48

    // get the bounding box of the triangle
    const auto min = math::min(math::min(proj0.pos, proj1.pos), proj2.pos);
    const auto max = math::max(math::max(proj0.pos, proj1.pos), proj2.pos);

    const auto [v0, depth0, Z_inv0, attrs0] = proj0;
    const auto [v1, depth1, Z_inv1, attrs1] = proj1;
    const auto [v2, depth2, Z_inv2, attrs2] = proj2;

    const auto depth = math::Vec3{ depth0, depth1, depth2 };
    const auto Z_inv = math::Vec3{ Z_inv0, Z_inv1, Z_inv2 };
    const auto attrs = std::array{ attrs0, attrs1, attrs2 };

    /* optimised calculation of:
     *    p = {x, y}
     *   w0 = cross(v1v2, v1p) // opposite of v0 : v1v2
     *   w1 = cross(v2v0, v2p) // opposite of v1 : v2v0
     *   w2 = cross(v0v1, v0p) // opposite of v2 : v0v1
     *
     *            ,>v1
     *          .'    ´.
     *        .'        ´.
     *      .'     p      ´.
     *    .'                _|
     *   v0<-----------------v2
     *
     * note:
     *  cross(lhs, rhs) = lhs.x * rhs.y - rhs.x * lhs.y, for "2D" vectors
     *
     * cross product terms:
     *  cross(vivj, {1, 0} + min - vi) - cross(vivj, min - vi) = -vivj.y
     *  cross(vivj, {0, 1} + min - vi) - cross(vivj, min - vi) = +vivj.x
     */

    const math::Vec2 v1v2 = v1.vector_to(v2);
    const math::Vec2 v2v0 = v2.vector_to(v0);
    const math::Vec2 v0v1 = v0.vector_to(v1);

    // bias to exclude bottom right edge:
    const math::Float bias0 = is_top_left_edge_of_triangle(v1, v2) ? 0.f : -1.f;
    const math::Float bias1 = is_top_left_edge_of_triangle(v2, v0) ? 0.f : -1.f;
    const math::Float bias2 = is_top_left_edge_of_triangle(v0, v1) ? 0.f : -1.f;

    const math::Float triangle_area_2 = cross(v0.vector_to(v2), v0.vector_to(v1));
    if (triangle_area_2 == 0) {
        return;
    }

    math::Vec2 p{ min.x, min.y };

    math::Float w0_y_minx = cross(v1v2, v1.vector_to(p)) + bias0;
    math::Float w1_y_minx = cross(v2v0, v2.vector_to(p)) + bias1;
    math::Float w2_y_minx = cross(v0v1, v0.vector_to(p)) + bias2;

    // cross product terms:
    const math::Float delta_w0_x = -v1v2.y;
    const math::Float delta_w0_y = +v1v2.x;

    const math::Float delta_w1_x = -v2v0.y;
    const math::Float delta_w1_y = +v2v0.x;

    const math::Float delta_w2_x = -v0v1.y;
    const math::Float delta_w2_y = +v0v1.x;

    // bounding box as integer:
    const auto min_x_int = static_cast<std::size_t>(min.x);
    const auto min_y_int = static_cast<std::size_t>(min.y);
    const auto max_x_int = static_cast<std::size_t>(max.x);
    const auto max_y_int = static_cast<std::size_t>(max.y);

    const auto func = [&triangle_area_2, &Z_inv, &depth, &attrs](const math::Vec3& w,
                                                                 const math::Vec2& pos) -> ProjectedFragment<Varying> {
        const auto weights = w / triangle_area_2;
        const auto acc_Z_inv = barycentric(Z_inv, weights);
        const auto acc_depth = barycentric_perspective_corrected(depth, weights, Z_inv, acc_Z_inv);
        const auto acc_attrs = barycentric_perspective_corrected(attrs, weights, Z_inv, acc_Z_inv);

        return ProjectedFragment<Varying>{ .pos = pos, .depth = acc_depth, .Z_inv = acc_Z_inv, .attrs = acc_attrs };
    };

    for (std::size_t y = min_y_int; y <= max_y_int; y++) {
        math::Float w0 = w0_y_minx;
        math::Float w1 = w1_y_minx;
        math::Float w2 = w2_y_minx;

        p.x = min.x;

        for (std::size_t x = min_x_int; x <= max_x_int; x++) {
            const auto w = math::Vec3{ w0, w1, w2 };
            const bool in_triangle = w.x >= 0 && w.y >= 0 && w.z >= 0;

            if (in_triangle) {
                const auto pfrag = func(w, p);
                if (test_and_set_depth(pfrag.pos, pfrag.depth)) {
                    plot(pfrag);
                }
            }
            w0 += delta_w0_x;
            w1 += delta_w1_x;
            w2 += delta_w2_x;
            p.x += 1;
        }
        w0_y_minx += delta_w0_y;
        w1_y_minx += delta_w1_y;
        w2_y_minx += delta_w2_y;
        p.y += 1;
    }
}

} // namespace asciirast::rasterize
