#pragma once

#include <cfloat>
#include <generator>

#include "../math/types.h"
#include "../program.h"
#include "./lerp.h"

namespace asciirast::rasterize {

static const inline auto default_depth = std::numeric_limits<math::F>::infinity();

template<VaryingInterface Varying>
static auto
rasterize_line(const ProjectedFragment<Varying>& proj0, const ProjectedFragment<Varying>& proj1)
        -> std::generator<std::tuple<math::Vec2, math::F, math::F, Varying>>
{
    // DDA Line algorithm:
    // - https://www.redblobgames.com/grids/line-drawing/#more
    // - https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)

    const auto [v0, z_inv0, w_inv0, attrs0] = proj0;
    const auto [v1, z_inv1, w_inv1, attrs1] = proj1;

    const auto delta   = v1 - v0;
    const auto size    = math::abs(delta);
    const auto len     = std::max<math::F>(size.x, size.y);
    const auto len_inv = 1 / len; // division by zero let through

    const auto inc_t     = len_inv;
    const auto inc_v     = (v1 - v0) * len_inv;
    const auto inc_z_inv = (z_inv1 - z_inv0) * len_inv;
    const auto inc_w_inv = (w_inv1 - w_inv0) * len_inv;
    const auto inc_attrs = (attrs1 + attrs0 * -1) * len_inv;

    auto acc_t     = math::F{ 0 };
    auto acc_v     = v0;
    auto acc_z_inv = z_inv0;
    auto acc_w_inv = w_inv0;
    auto acc_attrs = attrs0;

    if (std::isfinite(z_inv0) && std::isfinite(z_inv1)) {
        for (std::size_t i = 0; i < static_cast<std::size_t>(len); i++) {
            co_yield std::make_tuple(
                    math::floor(acc_v),
                    acc_z_inv,
                    acc_w_inv,
                    lerp_varying_perspective_corrected(proj0.attrs, proj1.attrs, z_inv0, z_inv1, acc_z_inv));

            acc_t += inc_t;
            acc_v += inc_v;
            acc_z_inv += inc_z_inv;
            acc_w_inv += inc_w_inv;
        }
    } else {
        for (std::size_t i = 0; i < static_cast<std::size_t>(len); i++) {
            co_yield std::make_tuple(math::floor(acc_v), default_depth, acc_w_inv, acc_attrs);

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

    const bool points_right = math::almost_less_than<math::F>(0, edge.x);
    const bool points_up    = math::almost_less_than<math::F>(0, edge.y); // since y-axis points up

    const bool is_top_edge  = math::almost_equal<math::F>(0, edge.y) && points_right;
    const bool is_left_edge = points_up;

    return is_top_edge || is_left_edge;
}

template<VaryingInterface Varying, bool is_CW_winding_order>
static auto
rasterize_triangle(const ProjectedFragment<Varying>& proj0,
                   const ProjectedFragment<Varying>& proj1,
                   const ProjectedFragment<Varying>& proj2)
        -> std::generator<std::tuple<math::Vec2, math::F, math::F, Varying>>
{
    // Algorithm using bayesian coordinates:
    // - https://www.youtube.com/watch?v=k5wtuKWmV48
    // - https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf

    // get the bounding box of the triangle
    const auto min = math::min(math::min(proj0.pos, proj1.pos), proj2.pos);
    const auto max = math::max(math::max(proj0.pos, proj1.pos), proj2.pos);

    const auto [v0, z_inv0, w_inv0, attrs0] = proj0;
    const auto [v1, z_inv1, w_inv1, attrs1] = proj1;
    const auto [v2, z_inv2, w_inv2, attrs2] = proj2;

    const math::F triangle_area_2_inv = 1.f / std::abs(math::cross(v0.vector_to(v1), v0.vector_to(v2)));

    math::Vec2 p{ min.x, min.y };
    math::F w0_y_minx, w1_y_minx, w2_y_minx;
    math::F delta_w0_y, delta_w0_x;
    math::F delta_w1_y, delta_w1_x;
    math::F delta_w2_y, delta_w2_x;

    if constexpr (is_CW_winding_order) {
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
        const math::F bias0 = is_top_left_edge_of_triangle(v1, v2) ? 0 : -1;
        const math::F bias1 = is_top_left_edge_of_triangle(v2, v0) ? 0 : -1;
        const math::F bias2 = is_top_left_edge_of_triangle(v0, v1) ? 0 : -1;

        w0_y_minx = cross(v1v2, v1.vector_to(p)) + bias0;
        w1_y_minx = cross(v2v0, v2.vector_to(p)) + bias1;
        w2_y_minx = cross(v0v1, v0.vector_to(p)) + bias2;

        // cross product terms:
        delta_w0_x = -v1v2.y;
        delta_w0_y = +v1v2.x;

        delta_w1_x = -v2v0.y;
        delta_w1_y = +v2v0.x;

        delta_w2_x = -v0v1.y;
        delta_w2_y = +v0v1.x;
    } else {
        /* optimised calculation of:
         *   w0 = cross(v2v1, v2p) // opposite of v0 : v2v1
         *   w1 = cross(v0v1, v0p) // opposite of v1 : v0v1
         *   w2 = cross(v1v0, v1p) // opposite of v2 : v1v0
         *
         *            ,v1|-
         *          .'    ´.
         *        .'        ´.
         *      .'    p       ´.
         *    |_                '
         *   v0----------------->v2
         *
         * note:
         *  cross(lhs, rhs) = lhs.x * rhs.y - rhs.x * lhs.y, for "2D" vectors
         *
         * cross product terms:
         *  cross(vivj, {1, 0} + min - vi) - cross(vivj, min - vi) = -vivj.y
         *  cross(vivj, {0, 1} + min - vi) - cross(vivj, min - vi) = +vivj.x
         */

        const math::Vec2 v2v1 = v2.vector_to(v1);
        const math::Vec2 v0v2 = v0.vector_to(v2);
        const math::Vec2 v1v0 = v1.vector_to(v0);

        // bias to exclude bottom right edge:
        const math::F bias0 = is_top_left_edge_of_triangle(v2, v1) ? 0 : -1;
        const math::F bias1 = is_top_left_edge_of_triangle(v0, v2) ? 0 : -1;
        const math::F bias2 = is_top_left_edge_of_triangle(v1, v0) ? 0 : -1;

        w0_y_minx = cross(v2v1, v2.vector_to(p)) + bias0;
        w1_y_minx = cross(v0v2, v0.vector_to(p)) + bias1;
        w2_y_minx = cross(v1v0, v1.vector_to(p)) + bias2;

        delta_w0_x = -v2v1.y;
        delta_w0_y = +v2v1.x;

        delta_w1_x = -v0v2.y;
        delta_w1_y = +v0v2.x;

        delta_w2_x = -v1v0.y;
        delta_w2_y = +v1v0.x;
    }

    if (std::isfinite(z_inv0) && std::isfinite(z_inv1)) {
        for (std::size_t y = static_cast<std::size_t>(min.y); y <= static_cast<std::size_t>(max.y); y++) {
            math::F w0 = w0_y_minx;
            math::F w1 = w1_y_minx;
            math::F w2 = w2_y_minx;
            p.x        = min.x;
            for (std::size_t x = static_cast<std::size_t>(min.x); x <= static_cast<std::size_t>(max.x); x++) {
                const bool is_inside_triangle = w0 >= 0 && w1 >= 0 && w2 >= 0;
                if (is_inside_triangle) {
                    const auto alpha = w0 * triangle_area_2_inv;
                    const auto beta  = w1 * triangle_area_2_inv;
                    const auto gamma = w2 * triangle_area_2_inv;

                    const auto z_inv = z_inv0 * alpha + z_inv1 * beta + z_inv2 * gamma;
                    const auto w_inv = w_inv0 * alpha + w_inv1 * beta + w_inv1 * gamma;
                    const auto attrs = attrs0 * z_inv0 * alpha + attrs1 * z_inv1 * beta + attrs2 * z_inv2 * gamma;

                    co_yield std::make_tuple(p, z_inv, w_inv, attrs * (1 / z_inv));
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
        for (std::size_t y = static_cast<std::size_t>(min.y); y <= static_cast<std::size_t>(max.y); y++) {
            math::F w0 = w0_y_minx;
            math::F w1 = w1_y_minx;
            math::F w2 = w2_y_minx;

            p.x = min.x;

            for (std::size_t x = static_cast<std::size_t>(min.x); x <= static_cast<std::size_t>(max.x); x++) {
                const bool is_inside_triangle = w0 >= 0 && w1 >= 0 && w2 >= 0;
                if (is_inside_triangle) {
                    const auto alpha = w0 * triangle_area_2_inv;
                    const auto beta  = w1 * triangle_area_2_inv;
                    const auto gamma = w2 * triangle_area_2_inv;

                    const auto w_inv = w_inv0 * alpha + w_inv1 * beta + w_inv1 * gamma;
                    const auto attrs = attrs0 * alpha + attrs1 * beta + attrs2 * gamma;

                    co_yield std::make_tuple(p, default_depth, w_inv, attrs);
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
