/**
 * @file rasterize.h
 * @brief Functions for rasterizing lines and triangles
 */

#pragma once

#include <cfloat>

#include "../fragment.h"
#include "../math/types.h"
#include "../renderer_options.h"

namespace asciirast::renderer {

template<RendererOptions Options, VaryingInterface Varying, typename Plot>
static constexpr bool rasterize_line_plot_func_conds =
    std::is_invocable_v<Plot, const std::array<ProjectedFragment<Varying>, 2>&, const std::array<bool, 2>&> ||
    std::is_invocable_v<Plot, const ProjectedFragment<Varying>&>;

template<RendererOptions Options, VaryingInterface Varying, typename Plot>
    requires(rasterize_line_plot_func_conds<Options, Varying, Plot>)
[[maybe_unused]] static void
rasterize_line(const ProjectedFragment<Varying>& proj0, const ProjectedFragment<Varying>& proj1, const Plot&& plot)
{
    // Modified DDA Line algorithm:
    // https://www.redblobgames.com/grids/line-drawing/#more
    // https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)

    const auto [v0, depth0, Z_inv0, attrs0] = proj0;
    const auto [v1, depth1, Z_inv1, attrs1] = proj1;

    const auto delta = v1 - v0;
    const auto size = abs(delta);
    const auto len = std::max<math::Float>(size.x, size.y);
    const auto len_uint = static_cast<std::size_t>(len);
    if (len_uint == 0) {
        return;
    }
    const auto len_inv = 1 / len;

    const auto inc_t = len_inv;
    const auto inc_v = (v1 - v0) * len_inv;
    const auto inc_depth = (depth1 - depth0) * len_inv;
    const auto inc_Z_inv = (Z_inv1 - Z_inv0) * len_inv;

    const auto func = [&](const math::Float& acc_t, //
                          const math::Vec2& acc_v,
                          const math::Float& acc_depth,
                          const math::Float& acc_Z_inv) {
        static constexpr auto Option = Options.attr_interpolation;

        return ProjectedFragment<Varying>{
            .pos = trunc(acc_v),
            .depth = acc_depth,
            .Z_inv = acc_Z_inv,
            .attrs = lerp_projected_varying_conditionally<Option>(attrs0, attrs1, acc_t, Z_inv0, Z_inv1, acc_Z_inv),
        };
    };

    const auto bias0 = !(Options.line_ends_inclusion == LineEndsInclusion::IncludeStart ||
                         Options.line_ends_inclusion == LineEndsInclusion::IncludeBoth);
    const auto bias1 = !(Options.line_ends_inclusion == LineEndsInclusion::IncludeEnd ||
                         Options.line_ends_inclusion == LineEndsInclusion::IncludeBoth);

    auto acc_t = math::Float{ 0 };
    auto acc_v = v0;
    auto acc_depth = depth0;
    auto acc_Z_inv = Z_inv0;

    if (bias0) {
        acc_t += inc_t;
        acc_v += inc_v;
        acc_depth += inc_depth;
        acc_Z_inv += inc_Z_inv;
    }

    if constexpr (std::is_invocable_v<Plot, const ProjectedFragment<Varying>&>) {
        for (std::size_t i = bias0; i <= len_uint - bias1; i++) {
            plot(func(acc_t, acc_v, acc_depth, acc_Z_inv));

            acc_t += inc_t;
            acc_v += inc_v;
            acc_depth += inc_depth;
            acc_Z_inv += inc_Z_inv;
        }
    } else {
        std::array<ProjectedFragment<Varying>, 2> rfrag;
        rfrag[0] = func(acc_t, acc_v, acc_depth, acc_Z_inv);

        // process 1 fragment at a time, but pass both the current and the
        // one ahead:
        for (std::size_t i = bias0; i <= len_uint - bias1; i++) {
            acc_t += inc_t;
            acc_v += inc_v;
            acc_depth += inc_depth;
            acc_Z_inv += inc_Z_inv;

            rfrag[(i + 1) % 2] = func(acc_t, acc_v, acc_depth, acc_Z_inv);

            plot({ rfrag[(i + 0) % 2], rfrag[(i + 1) % 2] }, { true, false });
        }
    }
}

/**
 * @brief Interpolation of vectors with barycentric coordinates of
 *        triangles
 */
[[maybe_unused]] static auto
barycentric(const math::Vec3& v, const math::Vec3& weights) -> math::Float
{
    return dot(v, weights);
}

/**
 * @brief Interpolation of varying with barycentric coordinates of
 *        triangles
 */
template<VaryingInterface Varying>
[[maybe_unused]] static auto
barycentric(const std::array<Varying, 3>& attrs, const math::Vec3& weights) -> Varying
{
    if constexpr (std::is_same_v<Varying, EmptyVarying>) {
        return EmptyVarying();
    } else {
        const auto aw0 = attrs[0] * weights[0];
        const auto aw1 = attrs[1] * weights[1];
        const auto aw2 = attrs[2] * weights[2];

        return aw0 + aw1 + aw2;
    }
}

/**
 * @brief Interpolation of varying with barycentric coordinates of
 *        triangles
 */
template<VaryingInterface Varying>
[[maybe_unused]] static auto
barycentric_projected(const std::array<Varying, 3>& attrs,
                      const math::Vec3& weights,
                      const math::Vec3& Z_inv,
                      const math::Float& acc_Z_inv) -> Varying
{
    if constexpr (std::is_same_v<Varying, EmptyVarying>) {
        return EmptyVarying();
    } else {
        // acc_Z_inv := barycentric(weights, Z_inv)

        const auto w = weights * Z_inv;

        const auto aw0 = attrs[0] * w[0];
        const auto aw1 = attrs[1] * w[1];
        const auto aw2 = attrs[2] * w[2];

        return (aw0 + aw1 + aw2) * (1 / acc_Z_inv);
    }
}

/**
 * @brief Interpolation of varying with barycentric coordinates of
 *        triangles depending on option
 */
template<AttrInterpolation Option, VaryingInterface Varying>
[[maybe_unused]] static auto
barycentric_projected_conditionally(const std::array<Varying, 3>& attrs,
                                    [[maybe_unused]] const math::Vec3& weights,
                                    [[maybe_unused]] const math::Vec3& Z_inv,
                                    [[maybe_unused]] const math::Float& acc_Z_inv) -> Varying
{
    if constexpr (Option == AttrInterpolation::Smooth) {
        return barycentric_projected(attrs, weights, Z_inv, acc_Z_inv);
    } else if constexpr (Option == AttrInterpolation::NoPerspective) {
        return barycentric(attrs, weights);
    } else {
        return attrs[0];
    }
}

[[maybe_unused]] static auto
is_top_left_edge_of_triangle(const math::Vec2& src, const math::Vec2& dest) -> bool
{
    const auto edge = src.vector_to(dest);

    const bool points_right = math::almost_less_than<math::Float>(0, edge.x); // 0 < x
    const bool points_up = math::almost_less_than<math::Float>(0, edge.y);    // 0 < y

    const bool is_top_edge = math::almost_equal<math::Float>(0, edge.y) && points_right;
    const bool is_left_edge = points_up;

    return is_top_edge || is_left_edge;
}

template<RendererOptions Options, VaryingInterface Varying, typename Plot>
    requires(std::is_invocable_v<Plot, const std::array<ProjectedFragment<Varying>, 4>&, const std::array<bool, 4>&> ||
             std::is_invocable_v<Plot, const ProjectedFragment<Varying>&>)
[[maybe_unused]] static void
rasterize_triangle(const ProjectedFragment<Varying>& proj0,
                   const ProjectedFragment<Varying>& proj1,
                   const ProjectedFragment<Varying>& proj2,
                   const Plot&& plot)
{
    // Modified algorithm which uses cross products and bayesian
    // coordinates for triangles:
    // - https://www.youtube.com/watch?v=k5wtuKWmV48

    // get the bounding box of the triangle
    const auto min_ = min(min(proj0.pos, proj1.pos), proj2.pos);
    const auto max_ = max(max(proj0.pos, proj1.pos), proj2.pos);

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
     *   w0 = cross(v1v2, v1p) // opposite of v0 is v1v2
     *   w1 = cross(v2v0, v2p) // opposite of v1 is v2v0
     *   w2 = cross(v0v1, v0p) // opposite of v2 is v0v1
     *
     *            ,.v2_
     *          .'   |´.
     *        .'        ´.
     *      .'     p      ´.
     *    |_                `
     *   v0----------------->v1
     *
     * note:
     *  cross(lhs, rhs) = lhs.x * rhs.y - rhs.x * lhs.y, for "2D" vectors
     *
     * cross product terms:
     *  cross(vivj, {1, 0} + min - vi) - cross(vivj, min - vi) = -vivj.y
     *  cross(vivj, {0, 1} + min - vi) - cross(vivj, min - vi) = +vivj.x
     */

    // bias to exclude either top-left or bottom-right edge
    static constexpr bool top_left_bias = Options.triangle_fill_bias == TriangleFillBias::TopLeft;
    static constexpr bool bottom_right_bias = Options.triangle_fill_bias == TriangleFillBias::BottomRight;

    const math::Float bias0 =
        is_top_left_edge_of_triangle(v1, v2) ? (top_left_bias ? 0.f : -1.f) : (bottom_right_bias ? 0.f : -1.f);
    const math::Float bias1 =
        is_top_left_edge_of_triangle(v2, v0) ? (top_left_bias ? 0.f : -1.f) : (bottom_right_bias ? 0.f : -1.f);
    const math::Float bias2 =
        is_top_left_edge_of_triangle(v0, v1) ? (top_left_bias ? 0.f : -1.f) : (bottom_right_bias ? 0.f : -1.f);

    const math::Float triangle_area_2 = cross(v0.vector_to(v1), v0.vector_to(v2));
    ASCIIRAST_ASSERT(triangle_area_2 > 0, "non-negative triangle area");

    const math::Vec2 v1v2 = v1.vector_to(v2);
    const math::Vec2 v2v0 = v2.vector_to(v0);
    const math::Vec2 v0v1 = v0.vector_to(v1);

    const math::Vec2 offset = { 0.5f, 0.5f };
    math::Vec2 p = min_ + offset;
    math::Vec3 w_y_minx = {
        cross(v1v2, v1.vector_to(p)) + bias0,
        cross(v2v0, v2.vector_to(p)) + bias1,
        cross(v0v1, v0.vector_to(p)) + bias2,
    };

    // cross product terms:
    const math::Vec3 delta_w_x = { -v1v2.y, -v2v0.y, -v0v1.y };
    const math::Vec3 delta_w_y = { +v1v2.x, +v2v0.x, +v0v1.x };

    // bounding box as integer:
    const auto x_diff = static_cast<std::size_t>(max_.x) - static_cast<std::size_t>(min_.x);
    const auto y_diff = static_cast<std::size_t>(max_.y) - static_cast<std::size_t>(min_.y);

    const auto func = [&](const math::Vec3& w, const math::Vec2& pos) {
        static constexpr auto Option = Options.attr_interpolation;

        const auto weights = w / triangle_area_2;
        const auto acc_depth = barycentric(depth, weights);
        const auto acc_Z_inv = barycentric(Z_inv, weights);
        const auto acc_attrs = barycentric_projected_conditionally<Option>(attrs, weights, Z_inv, acc_Z_inv);

        return ProjectedFragment<Varying>{ .pos = pos, .depth = acc_depth, .Z_inv = acc_Z_inv, .attrs = acc_attrs };
    };

    if constexpr (std::is_invocable_v<Plot, const ProjectedFragment<Varying>&>) {
        for (std::size_t y = 0; y <= y_diff; y++) {
            auto w = w_y_minx;
            p.x = min_.x + offset.x;

            for (std::size_t x = 0; x <= x_diff; x++) {
                if (const bool in_triangle = w.x >= 0 && w.y >= 0 && w.z >= 0; in_triangle) {
                    plot(func(w, p));
                }
                w += delta_w_x;
                p.x += 1;
            }
            w_y_minx += delta_w_y;
            p.y += 1;
        }
    } else {
        for (std::size_t y = 0; y <= y_diff / 2 + y_diff % 2; y++) {
            auto w = w_y_minx;
            p.x = min_.x + offset.x;

            for (std::size_t x = 0; x <= x_diff / 2 + x_diff % 2; x++) {
                const auto w00 = w;
                const auto w01 = w + delta_w_x;
                const auto w10 = w + delta_w_y;
                const auto w11 = w + delta_w_y + delta_w_x;

                const auto in_triangle00 = w00.x >= 0 && w00.y >= 0 && w00.z >= 0;
                const auto in_triangle01 = w01.x >= 0 && w01.y >= 0 && w01.z >= 0;
                const auto in_triangle10 = w10.x >= 0 && w10.y >= 0 && w10.z >= 0;
                const auto in_triangle11 = w11.x >= 0 && w11.y >= 0 && w11.z >= 0;

                if (in_triangle00 || in_triangle01 || in_triangle10 || in_triangle11) {
                    plot({ func(w00, p + math::Vec2{ 0, 0 }), //
                           func(w01, p + math::Vec2{ 1, 0 }),
                           func(w10, p + math::Vec2{ 0, 1 }),
                           func(w11, p + math::Vec2{ 1, 1 }) },
                         { in_triangle00, in_triangle01, in_triangle10, in_triangle11 });
                }

                w += 2 * delta_w_x;
                p.x += 2;
            }

            w_y_minx += 2 * delta_w_y;
            p.y += 2;
        }
    }
}

} // namespace asciirast::renderer
