#pragma once

#include "../math/types.h"

namespace asciirast::rasterize {

static bool
point_in_screen(const math::Vec2& p)
{
    const bool x_in_bounds = -1 <= p.x && p.x <= +1;
    const bool y_in_bounds = -1 <= p.y && p.y <= +1;

    return x_in_bounds && y_in_bounds;
}

static bool
point_in_unit_square(const math::Vec2& p)
{
    const bool x_in_bounds = +0 <= p.x && p.x <= +1;
    const bool y_in_bounds = +0 <= p.y && p.y <= +1;

    return x_in_bounds && y_in_bounds;
}

static bool
point_in_frustum(const math::Vec4& p)
{
    const bool x_in_bounds = -p.w <= p.x && p.x <= +p.w;
    const bool y_in_bounds = -p.w <= p.y && p.y <= +p.w;
    const bool z_in_bounds = -p.w <= p.z && p.z <= +p.w;

    return x_in_bounds && y_in_bounds && z_in_bounds;
}

using T = math::F;

enum class BorderType
{
    LEFT = 0U,
    RIGHT,
    BOTTOM,
    TOP,
    NEAR,
    FAR,
    COUNT,
    BEGIN = 0U,
    END   = COUNT,
    END2D = TOP + 1U,
};

namespace detail {

static inline BorderType
next_border_type(BorderType b)
{
    return static_cast<BorderType>(static_cast<std::size_t>(b) + 1);
}

}

// Liang-Barsky clipping algorithm:
// - https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm
// - https://github.com/Larry57/WinForms3D/blob/master/WinForms3D/Clipping/LiangBarskyClippingHomogeneous.cs

static inline bool
line_in_bounds(const T q, const T p, T& t0, T& t1)
{
    // q: delta from border to vector tail
    // p: delta from vector tail to vector head. sign flipped to face border

    if (math::almost_equal<T>(p, 0)) { // Check if line is parallel to the clipping boundary
        if (q < 0.f) {
            return false; // Line is outside and parallel, so completely discarded
        }
        return true; // Line is inside and parallel, so kept as is
    }
    const float t = q / p; // t for the intersection point of the line and the
                           // window edge (possibly projected)

    if (p < 0.f) { // the line proceeds outside to inside the clip window
        if (t1 < t) {
            return false;
        }
        t0 = std::max(t, t0);
    } else { // the line proceeds inside to outside the clip window
        if (t < t0) {
            return false;
        }
        t1 = std::min(t, t1);
    }
    return true;
}

static inline bool
line_in_bounds(const math::Vec2& p0,
               const math::Vec2& p1,
               const BorderType border,
               const math::Vec2& min,
               const math::Vec2& max,
               T& t0,
               T& t1)
{
    const std::size_t border_id = static_cast<std::size_t>(border);

    const math::Vec2 delta = p1 - p0;

    // clang-format off
    const std::array<T, 6> q = {
        p0.x - min.x, max.x - p0.x,
        p0.y - min.y, max.y - p0.y,
    };

    const std::array<T, 6> p = {
        -delta.x, delta.x,
        -delta.y, delta.y,
    };
    // clang-format on

    return line_in_bounds(q[border_id], p[border_id], t0, t1);
}

static inline bool
line_in_bounds(const math::Vec4& p0,
               const math::Vec4& p1,
               const BorderType border,
               const math::Vec3& min,
               const math::Vec3& max,
               T& t0,
               T& t1)
{
    const std::size_t border_id = static_cast<std::size_t>(border);

    const math::Vec4 delta = p1 - p0;

    const std::array<T, 6> q = {
        p0.x - min.x, max.x - p0.x, //
        p0.y - min.y, max.y - p0.y, //
        p0.z - min.z, max.z - p0.z, //
    };

    const std::array<T, 6> p = {
        -delta.x, delta.x, //
        -delta.y, delta.y, //
        -delta.z, delta.z, //
    };

    return line_in_bounds(q[border_id], -delta.w + p[border_id], t0, t1);
}

static auto
line_in_bounds(const math::Vec2& p0, const math::Vec2& p1, const math::Vec2& min, const math::Vec2& max)
        -> std::optional<std::tuple<math::F, math::F>>
{
    math::F t0 = 0;
    math::F t1 = 1;

    for (auto border = BorderType::BEGIN; border < BorderType::END2D; border = detail::next_border_type(border)) {
        if (!line_in_bounds(p0, p1, border, min, max, t0, t1)) {
            return {};
        }
    }
    return std::make_optional(std::make_tuple(t0, t1));
}

static auto
line_in_screen(const math::Vec2& p0, const math::Vec2& p1) -> std::optional<std::tuple<math::F, math::F>>
{
    return line_in_bounds(p0, p1, math::Vec2{ -1, -1 }, math::Vec2{ +1, +1 });
}

static auto
line_in_unit_square(const math::Vec2& p0, const math::Vec2& p1) -> std::optional<std::tuple<math::F, math::F>>
{
    return line_in_bounds(p0, p1, math::Vec2{ +0, +0 }, math::Vec2{ +1, +1 });
}

static auto
line_in_frustum(const math::Vec4& p0, const math::Vec4& p1) -> std::optional<std::tuple<math::F, math::F>>
{
    if (p0.w < 0 && p1.w < 0) {
        return {};
    }

    const math::Vec3 min = { -p0.w, -p0.w, -p0.w };
    const math::Vec3 max = { +p0.w, +p0.w, +p0.w };

    math::F t0 = 0;
    math::F t1 = 1;

    for (auto border = BorderType::BEGIN; border < BorderType::END; border = detail::next_border_type(border)) {
        if (!line_in_bounds(p0, p1, border, min, max, t0, t1)) {
            return {};
        }
    }
    return std::make_optional(std::make_tuple(t0, t1));
}

// Triangle clipping algorithm based on:
// https://youtu.be/HXSuNxpCzdM?feature=shared&t=2151

using Vec4_Triplet = std::tuple<math::Vec4, math::Vec4, math::Vec4>;

namespace detail {

static inline const auto infty = std::numeric_limits<math::F>::infinity();

static inline auto
count_num_triangle_vertices_inside(const BorderType border, const Vec4_Triplet& v)
        -> std::tuple<unsigned, std::array<bool, 3>>
{
    math::Vec3 min0 = { -infty, -infty, -infty };
    math::Vec3 min1 = min0, min2 = min0;

    math::Vec3 max0 = { +infty, +infty, -+infty };
    math::Vec3 max1 = min0, max2 = max0;

    const auto [v0, v1, v2] = v;

    switch (border) {
    case BorderType::LEFT:
        min0.x = -v0.w;
        min1.x = -v1.w;
        min2.x = -v2.w;
        break;
    case BorderType::BOTTOM:
        max0.x = +v0.w;
        max1.x = +v1.w;
        max2.x = +v2.w;
        break;
    case BorderType::NEAR:
        min0.y = -v0.w;
        min1.y = -v1.w;
        min2.y = -v2.w;
        break;
    case BorderType::RIGHT:
        max0.y = +v0.w;
        max1.y = +v1.w;
        max2.y = +v2.w;
        break;
    case BorderType::TOP:
        min0.z = -v0.w;
        min1.z = -v1.w;
        min2.z = -v2.w;
        break;
    case BorderType::FAR:
        max0.z = +v0.w;
        max1.z = +v1.w;
        max2.z = +v2.w;
        break;
    default:
        break;
    }

    // clang-format off
    std::array<bool, 3> inside = {
        (min0 <= v0 && v0 <= max0),
        (min1 <= v1 && v1 <= max1),
        (min2 <= v2 && v2 <= max2)
    };
    // clang-format on

    return { inside[0] + inside[1] + inside[2], inside };
}

};

static auto
triangle_in_frustum(const math::Vec4& p0, const math::Vec4& p1, const math::Vec4& p2)
        -> std::optional<std::tuple<math::F, math::F>>
{
}

} // namespace asciirast::rasterize
