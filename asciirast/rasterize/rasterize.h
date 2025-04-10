#pragma once

#include <cassert>
#include <cfloat>

#include "../constants.h"
#include "../math/types.h"
#include "../program.h"
#include "./interpolate.h"

namespace asciirast::rasterize {

template<VaryingInterface Varying, typename Callable>
    requires(std::is_invocable_v<Callable, math::Vec2, math::Float, math::Float, Varying>)
static void
rasterize_line(const ProjectedFragment<Varying>& proj0, const ProjectedFragment<Varying>& proj1, const Callable plot)
{
    // DDA Line algorithm:
    // https://www.redblobgames.com/grids/line-drawing/#more
    // https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)

    const auto [v0, z_inv0, w_inv0, attrs0] = proj0;
    const auto [v1, z_inv1, w_inv1, attrs1] = proj1;

    const auto delta = v1 - v0;
    const auto size = math::abs(delta);
    const auto len = std::max<math::Float>(size.x, size.y);
    const auto len_inv = 1 / len; // division by zero let through

    const auto inc_t = len_inv;
    const auto inc_v = (v1 - v0) * len_inv;
    const auto inc_z_inv = (z_inv1 - z_inv0) * len_inv;
    const auto inc_w_inv = (w_inv1 - w_inv0) * len_inv;
    const auto inc_attrs = (attrs1 + attrs0 * -1) * len_inv;

    auto acc_t = math::Float{ 0 };
    auto acc_v = v0;
    auto acc_z_inv = z_inv0;
    auto acc_w_inv = w_inv0;
    auto acc_attrs = attrs0;

    if (std::isfinite(z_inv0) && std::isfinite(z_inv1)) {
        for (std::size_t i = 0; i < static_cast<std::size_t>(len); i++) {
            const auto attrs =
                    lerp_varying_perspective_corrected(proj0.attrs, proj1.attrs, acc_t, z_inv0, z_inv1, acc_z_inv);

            plot(math::floor(acc_v), acc_z_inv, acc_w_inv, attrs);

            acc_t += inc_t;
            acc_v += inc_v;
            acc_z_inv += inc_z_inv;
            acc_w_inv += inc_w_inv;
        }
    } else {
        for (std::size_t i = 0; i < static_cast<std::size_t>(len); i++) {
            plot(math::floor(acc_v), DEFAULT_DEPTH, acc_w_inv, acc_attrs);

            acc_v += inc_v;
            acc_w_inv += inc_w_inv;
            acc_attrs = std::move(acc_attrs + inc_attrs);
        }
    }
}

static inline bool
is_top_left_edge_of_triangle(const math::Vec2& src, const math::Vec2& dest)
{
    const auto edge = src.vector_to(dest);

    // note: (y > 0) since y-axis points up

    const bool points_right = math::almost_less_than<math::Float>(0, edge.x);
    const bool points_up = math::almost_less_than<math::Float>(0, edge.y);

    const bool is_top_edge = math::almost_equal<math::Float>(0, edge.y) && points_right;
    const bool is_left_edge = points_up;

    return is_top_edge || is_left_edge;
}

template<VaryingInterface Varying, typename Callable>
    requires(std::is_invocable_v<Callable, math::Vec2, math::Float, math::Float, Varying>)
static void
rasterize_triangle(const ProjectedFragment<Varying>& proj0,
                   const ProjectedFragment<Varying>& proj1,
                   const ProjectedFragment<Varying>& proj2,
                   const Callable plot)
{
    // Algorithm using cross products and bayesian coordinates for
    // triangles:
    // - https://www.youtube.com/watch?v=k5wtuKWmV48

    // get the bounding box of the triangle
    const auto min = math::min(math::min(proj0.pos, proj1.pos), proj2.pos);
    const auto max = math::max(math::max(proj0.pos, proj1.pos), proj2.pos);

    const auto [v0, z_inv0, w_inv0, attrs0] = proj0;
    const auto [v1, z_inv1, w_inv1, attrs1] = proj1;
    const auto [v2, z_inv2, w_inv2, attrs2] = proj2;

    const auto attrs = std::array{ attrs0, attrs1, attrs2 };
    const auto z_inv = math::Vec3{ z_inv0, z_inv1, z_inv2 };
    const auto w_inv = math::Vec3{ w_inv0, w_inv1, w_inv2 };

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

    const math::Float triangle_area_2 = cross(v0.vector_to(v1), v0.vector_to(v2));

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

    if (std::isfinite(z_inv0) && std::isfinite(z_inv1)) {
        for (std::size_t y = min_y_int; y <= max_y_int; y++) {
            math::Float w0 = w0_y_minx;
            math::Float w1 = w1_y_minx;
            math::Float w2 = w2_y_minx;

            p.x = min.x;

            for (std::size_t x = min_x_int; x <= max_x_int; x++) {
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    const auto weights = math::Vec3{ w0, w1, w2 } / triangle_area_2;
                    const auto acc_z_inv = barycentric(z_inv, weights);
                    const auto acc_w_inv = barycentric(w_inv, weights);
                    const auto acc_attrs = barycentric_perspective_corrected(attrs, weights, z_inv, acc_z_inv);

                    plot(p, acc_z_inv, acc_w_inv, acc_attrs);
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
    } else {
        for (std::size_t y = min_y_int; y <= max_y_int; y++) {
            math::Float w0 = w0_y_minx;
            math::Float w1 = w1_y_minx;
            math::Float w2 = w2_y_minx;

            p.x = min.x;

            for (std::size_t x = min_x_int; x <= max_x_int; x++) {
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    const auto weights = math::Vec3{ w0, w1, w2 } / triangle_area_2;
                    const auto acc_w_inv = barycentric(w_inv, weights);
                    const auto acc_attrs = barycentric(attrs, weights);

                    plot(p, DEFAULT_DEPTH, acc_w_inv, acc_attrs);
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
}

} // namespace asciirast::rasterize
