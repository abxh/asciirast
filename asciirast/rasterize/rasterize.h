#pragma once

#include <cassert>
#include <cfloat>

#include "../math/types.h"
#include "../renderer_options.h"
#include "./interpolate.h"

namespace asciirast::rasterize {

template<VaryingInterface Varying, typename Plot>
    requires(std::is_invocable_v<Plot, const std::array<ProjectedFragment<Varying>, 2>&, const std::array<bool, 2>&>)
[[maybe_unused]]
static void
rasterize_line(const ProjectedFragment<Varying>& proj0,
               const ProjectedFragment<Varying>& proj1,
               const Plot plot,
               const LineEndsInclusion bias_option)
{
    // Modified DDA Line algorithm:
    // https://www.redblobgames.com/grids/line-drawing/#more
    // https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)

    const auto [v0, depth0, Z_inv0, attrs0] = proj0;
    const auto [v1, depth1, Z_inv1, attrs1] = proj1;

    const auto delta = v1 - v0;
    const auto size = math::abs(delta);
    const auto len = std::max<math::Float>(size.x, size.y);
    const auto len_uint = static_cast<std::size_t>(len);
    if (len_uint == 0) {
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
            .pos = math::trunc(acc_v_),
            .depth = lerp_varying_perspective_corrected(depth0, depth1, acc_t_, Z_inv0, Z_inv1, acc_Z_inv_),
            .Z_inv = acc_Z_inv_,
            .attrs = lerp_varying_perspective_corrected(attrs0, attrs1, acc_t_, Z_inv0, Z_inv1, acc_Z_inv_)
        };
    };

    const auto bias0 =
            !(bias_option == LineEndsInclusion::IncludeStart || bias_option == LineEndsInclusion::IncludeBoth);
    const auto bias1 = !(bias_option == LineEndsInclusion::IncludeEnd || bias_option == LineEndsInclusion::IncludeBoth);

    std::array<ProjectedFragment<Varying>, 2> rfrag;
    rfrag[0] = func(acc_t, acc_v, acc_Z_inv);

    // process 1 fragment at a time, but pass both the current and the one ahead:
    for (std::size_t i = bias0; i <= len_uint - bias1; i++) {
        acc_t += inc_t;
        acc_v += inc_v;
        acc_Z_inv += inc_Z_inv;

        rfrag[(i + 1) % 2] = func(acc_t, acc_v, acc_Z_inv);

        plot({ rfrag[(i + 0) % 2], rfrag[(i + 1) % 2] }, { true, false });
    }
}

[[maybe_unused]]
static auto
is_top_left_edge_of_triangle(const math::Vec2& src, const math::Vec2& dest) -> bool
{
    const auto edge = src.vector_to(dest);

    const bool points_right = math::almost_less_than<math::Float>(0, edge.x); // 0 < x
    const bool points_up = math::almost_less_than<math::Float>(0, edge.y);    // 0 < y

    const bool is_top_edge = math::almost_equal<math::Float>(0, edge.y) && points_right;
    const bool is_left_edge = points_up;

    return is_top_edge || is_left_edge;
}

template<VaryingInterface Varying, typename Plot>
    requires(std::is_invocable_v<Plot, const std::array<ProjectedFragment<Varying>, 4>&, const std::array<bool, 4>&>)
[[maybe_unused]]
static void
rasterize_triangle(const ProjectedFragment<Varying>& proj0,
                   const ProjectedFragment<Varying>& proj1,
                   const ProjectedFragment<Varying>& proj2,
                   const Plot plot,
                   const TriangleFillBias bias_option)
{
    // Modified algorithm which uses cross products and bayesian coordinates for triangles:
    // - https://www.youtube.com/watch?v=k5wtuKWmV48

    // get the bounding box of the triangle
    const auto min = math::min(math::min(proj0.pos, proj1.pos), proj2.pos);
    const auto max = math::max(math::max(proj0.pos, proj1.pos), proj2.pos);

    const auto [v0_, depth0, Z_inv0, attrs0] = proj0;
    const auto [v1_, depth1, Z_inv1, attrs1] = proj1;
    const auto [v2_, depth2, Z_inv2, attrs2] = proj2;

    const auto v0 = v0_ + math::Vec2{ 0.5f, 0.5f };
    const auto v1 = v1_ + math::Vec2{ 0.5f, 0.5f };
    const auto v2 = v2_ + math::Vec2{ 0.5f, 0.5f };

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

    // bias to exclude either top-left or bottom-right edge
    const math::Float bias0 = is_top_left_edge_of_triangle(v1, v2)
                                      ? (bias_option == TriangleFillBias::TopLeft ? 0.f : -1.f)
                                      : (bias_option == TriangleFillBias::BottomRight ? 0.f : -1.f);
    const math::Float bias1 = is_top_left_edge_of_triangle(v2, v0)
                                      ? (bias_option == TriangleFillBias::TopLeft ? 0.f : -1.f)
                                      : (bias_option == TriangleFillBias::BottomRight ? 0.f : -1.f);
    const math::Float bias2 = is_top_left_edge_of_triangle(v0, v1)
                                      ? (bias_option == TriangleFillBias::TopLeft ? 0.f : -1.f)
                                      : (bias_option == TriangleFillBias::BottomRight ? 0.f : -1.f);

    const math::Float triangle_area_2 = cross(v0.vector_to(v2), v0.vector_to(v1));
    if (std::trunc(triangle_area_2) == 0) {
        return;
    }

    math::Vec2 p = min + math::Vec2{ 0.5f, 0.5f };
    auto w_y_minx = math::Vec3{ cross(v1v2, v1.vector_to(p)) + bias0,
                                cross(v2v0, v2.vector_to(p)) + bias1,
                                cross(v0v1, v0.vector_to(p)) + bias2 };

    // cross product terms:
    const auto delta_w_x = math::Vec3{ -v1v2.y, -v2v0.y, -v0v1.y };
    const auto delta_w_y = math::Vec3{ +v1v2.x, +v2v0.x, +v0v1.x };

    // bounding box as integer:
    const auto x_diff = static_cast<std::size_t>(max.x) - static_cast<std::size_t>(min.x);
    const auto y_diff = static_cast<std::size_t>(max.y) - static_cast<std::size_t>(min.y);

    const auto func = [&triangle_area_2, &Z_inv, &depth, &attrs](const math::Vec3& w,
                                                                 const math::Vec2& pos) -> ProjectedFragment<Varying> {
        const auto weights = w / triangle_area_2;
        const auto acc_Z_inv = barycentric(Z_inv, weights);
        const auto acc_depth = barycentric_perspective_corrected(depth, weights, Z_inv, acc_Z_inv);
        const auto acc_attrs = barycentric_perspective_corrected(attrs, weights, Z_inv, acc_Z_inv);

        return ProjectedFragment<Varying>{ .pos = pos, .depth = acc_depth, .Z_inv = acc_Z_inv, .attrs = acc_attrs };
    };

    for (std::size_t y = 0; y <= y_diff / 2 + 1; y++) {
        auto w = w_y_minx;
        p.x = min.x;

        for (std::size_t x = 0; x <= x_diff / 2 + 1; x++) {
            const auto w00 = w;
            const auto w01 = w + delta_w_x;
            const auto w10 = w + delta_w_y;
            const auto w11 = w + delta_w_y + delta_w_x;

            const auto in_triangle00 = w00.x >= 0 && w00.y >= 0 && w00.z >= 0;
            const auto in_triangle01 = w01.x >= 0 && w01.y >= 0 && w01.z >= 0;
            const auto in_triangle10 = w10.x >= 0 && w10.y >= 0 && w10.z >= 0;
            const auto in_triangle11 = w11.x >= 0 && w11.y >= 0 && w11.z >= 0;

            plot({ func(w00, p + math::Vec2{ 0, 0 }), //
                   func(w01, p + math::Vec2{ 1, 0 }), // note: notation is y then x
                   func(w10, p + math::Vec2{ 0, 1 }),
                   func(w11, p + math::Vec2{ 1, 1 }) },
                 { in_triangle00, in_triangle01, in_triangle10, in_triangle11 });

            w += 2 * delta_w_x;
            p.x += 2;
        }

        w_y_minx += 2 * delta_w_y;
        p.y += 2;
    }
}

} // namespace asciirast::rasterize
