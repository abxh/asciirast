#pragma once

#include "../math/types.h"
#include <list>

namespace asciirast::rasterize {

/*
static bool
point_in_screen(const math::Vec2& p)
{
    const bool x_in_bounds = -1 <= p.x && p.x <= +1;
    const bool y_in_bounds = -1 <= p.y && p.y <= +1;

    return x_in_bounds && y_in_bounds;
}
*/

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
    if (p.w < 0) [[unlikely]] {
        return false;
    }

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

/*
static auto
line_in_screen(const math::Vec2& p0, const math::Vec2& p1) -> std::optional<std::tuple<math::F, math::F>>
{
    return line_in_bounds(p0, p1, math::Vec2{ -1, -1 }, math::Vec2{ +1, +1 });
}
*/

static auto
line_in_unit_square(const math::Vec2& p0, const math::Vec2& p1) -> std::optional<std::tuple<math::F, math::F>>
{
    return line_in_bounds(p0, p1, math::Vec2{ +0, +0 }, math::Vec2{ +1, +1 });
}

static auto
line_in_frustum(const math::Vec4& p0, const math::Vec4& p1) -> std::optional<std::tuple<math::F, math::F>>
{
    if (p0.w < 0 && p1.w < 0) [[unlikely]] {
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
// https://youtu.be/HXSuNxpCzdM?feature=shared&t=2155

using Vec2_Triplet = std::array<math::Vec2, 3>;
using Vec4_Triplet = std::array<math::Vec4, 3>;
using Attr_Triplet = std::array<math::Vec4, 3>;

namespace detail {

static inline auto
count_num_triangle_vertices_inside(const BorderType border,
                                   const Vec2_Triplet& v,
                                   const math::Vec2& min,
                                   const math::Vec2& max) -> std::tuple<unsigned, std::array<bool, 3>>
{
    const auto [v0, v1, v2] = v;

    std::array<bool, 3> inside = { false, false, false };

    // clang-format off
    switch (border) {
    case BorderType::LEFT:
        inside = { min.x <= v0.x,
                   min.x <= v1.x,
                   min.x <= v2.x };
        break;
    case BorderType::RIGHT:
        inside = { v0.x <= max.x,
                   v1.x <= max.x,
                   v2.x <= max.x };
        break;
    case BorderType::BOTTOM:
        inside = { min.y <= v0.y,
                   min.y <= v1.y,
                   min.y <= v2.y };
        break;
    case BorderType::TOP:
        inside = { v0.y <= max.y,
                   v1.y <= max.y,
                   v2.y <= max.y };
        break;
    default:
        break;
    }
    // clang-format on

    return { inside[0] + inside[1] + inside[2], inside };
}

static inline auto
count_num_triangle_vertices_inside(const BorderType border, const Vec4_Triplet& v)
        -> std::tuple<unsigned, std::array<bool, 3>>
{
    const auto [v0, v1, v2] = v;

    std::array<bool, 3> inside = { false, false, false };

    // clang-format off
    switch (border) {
    case BorderType::LEFT:
        inside = { -v0.w <= v0.x,
                   -v1.w <= v1.x,
                   -v2.w <= v2.x };
        break;
    case BorderType::RIGHT:
        inside = { v0.x <= +v0.w,
                   v1.x <= +v1.w,
                   v2.x <= +v2.w };
        break;
    case BorderType::BOTTOM:
        inside = { -v0.w <= v0.y,
                   -v1.w <= v1.y,
                   -v2.w <= v2.y };
        break;
    case BorderType::TOP:
        inside = { v0.y <= +v0.w,
                   v1.y <= +v1.w,
                   v2.y <= +v2.w };
        break;
    case BorderType::NEAR:
        inside = { -v0.w <= v0.z,
                   -v1.w <= v1.z,
                   -v2.w <= v2.z };
        break;
    case BorderType::FAR:
        inside = { v0.z <= +v0.w,
                   v1.z <= +v1.w,
                   v2.z <= +v2.w };
        break;
    default:
        break;
    }
    // clang-format on

    return { inside[0] + inside[1] + inside[2], inside };
}

template<bool is_clockwise_winding_order>
static inline auto
get_ordered_triangle_verticies_1_inside(const std::array<bool, 3>& inside) -> std::array<unsigned, 3>
{
    // first point is inside. others are outside.

    if constexpr (is_clockwise_winding_order) {
        // order:
        // ... -> 0 -> 1 -> 2 -> ...

        if (inside[0]) {
            return { 0, 1, 2 };
        } else if (inside[1]) {
            return { 1, 2, 0 };
        } else if (inside[2]) {
            return { 2, 0, 1 };
        }
    } else {
        // order:
        // ... <- 0 <- 1 <- 2 <- ...

        if (inside[0]) {
            return { 0, 2, 1 };
        } else if (inside[1]) {
            return { 1, 0, 2 };
        } else if (inside[2]) {
            return { 2, 1, 0 };
        }
    }
    return { 3, 3, 3 };
}

template<bool is_CW_winding_order>
static inline auto
get_ordered_triangle_verticies_2_inside(const std::array<bool, 3>& inside) -> std::array<unsigned, 3>
{
    // first two points are inside. last one is outside.

    if constexpr (is_CW_winding_order) {
        // order:
        // ... -> 0 -> 1 -> 2 -> ...

        if (inside[0] && inside[1]) {
            return { 0, 1, 2 };
        } else if (inside[0] && inside[2]) {
            return { 2, 0, 1 };
        } else if (inside[1] && inside[2]) {
            return { 1, 2, 0 };
        }
    } else {
        // order:
        // ... <- 0 <- 1 <- 2 <- ...

        if (inside[0] && inside[1]) {
            return { 1, 0, 2 };
        } else if (inside[0] && inside[2]) {
            return { 0, 2, 1 };
        } else if (inside[1] && inside[2]) {
            return { 2, 1, 0 };
        }
    }
    return { 3, 3, 3 };
}

};

template<bool is_CW_winding_order>
static bool
triangle_in_frustum(std::list<Vec4_Triplet>& vecs_queue, std::list<Attr_Triplet>& attr_queue)
{
    assert(vecs_queue.size() > 0U);
    assert(vecs_queue.size() == attr_queue.size());

    if (const auto [v0, v1, v2] = *vecs_queue.begin(); v0.w < 0 && v1.w < 0 && v2.w < 0) [[unlikely]] {
        return false;
    }

    for (auto border = BorderType::BEGIN; border < BorderType::END; border = detail::next_border_type(border)) {
        auto it_vecs = vecs_queue.begin();
        auto it_attr = attr_queue.begin();

        while (it_vecs != vecs_queue.end()) {
            auto [count, inside] = detail::count_num_triangle_vertices_inside(border, *it_vecs);
            switch (count) {
            case 0:
                it_vecs = vecs_queue.erase(it_vecs);
                it_attr = attr_queue.erase(it_attr);
                continue;
            case 1: {
                const auto vecs_triplet = *it_vecs;
                const auto attr_triplet = *it_attr;
                const auto [i0, i1, i2] = detail::get_ordered_triangle_verticies_1_inside<is_CW_winding_order>(inside);
                const auto [p0, p1, p2] = { vecs_triplet[i0], vecs_triplet[i1], vecs_triplet[i2] };
                const auto [a0, a1, a2] = { attr_triplet[i0], attr_triplet[i1], attr_triplet[i2] };

                const math::Vec3 min = { -p0.w, -p0.w, -p0.w };
                const math::Vec3 max = { +p0.w, +p0.w, +p0.w };

                math::F t0a = 0.f;
                math::F t0b = 0.f;
                math::F t1a = 1.f;
                math::F t2b = 1.f;

                const bool b01 = line_in_bounds(p0, p1, border, min, max, t0a, t1a);
                const bool b02 = line_in_bounds(p0, p2, border, min, max, t0b, t2b);

                assert(b01);
                assert(t0a == 0.f);
                assert(t1a != 1.f);

                assert(b02);
                assert(t0b == 0.f);
                assert(t2b != 1.f);

                [[assume(b01 == true)]];
                [[assume(t0a == 0.f)]];
                [[assume(t1a != 1.f)]];

                [[assume(b02 == true)]];
                [[assume(t0b == 0.f)]];
                [[assume(t2b != 1.f)]];

                *it_vecs = Vec4_Triplet{ p0, math::lerp(p0, p1, t1a), math::lerp(p0, p2, t2b) };
                *it_attr = Attr_Triplet{ a0, lerp_varying(a0, a1, t1a), lerp_varying(a0, a2, t2b) };
            } break;
            case 2: {
                const auto vecs_triplet = *it_vecs;
                const auto attr_triplet = *it_attr;
                const auto [i0, i1, i2] = detail::get_ordered_triangle_verticies_2_inside<is_CW_winding_order>(inside);
                const auto [p0, p1, p2] = { vecs_triplet[i0], vecs_triplet[i1], vecs_triplet[i2] };
                const auto [a0, a1, a2] = { attr_triplet[i0], attr_triplet[i1], attr_triplet[i2] };

                const math::Vec3 min0 = { -p0.w, -p0.w, -p0.w };
                const math::Vec3 max0 = { +p0.w, +p0.w, +p0.w };

                const math::Vec3 min1 = { -p1.w, -p1.w, -p1.w };
                const math::Vec3 max1 = { +p1.w, +p1.w, +p1.w };

                math::F t0a = 0.f;
                math::F t1b = 0.f;
                math::F t2a = 1.f;
                math::F t2b = 1.f;

                const bool b2a = line_in_bounds(p0, p2, border, min0, max0, t0a, t2a);
                const bool b2b = line_in_bounds(p1, p2, border, min1, max1, t1b, t2b);

                assert(b2a);
                assert(t0a == 0.f);
                assert(t2a != 1.f);

                assert(b2b);
                assert(t1b == 0.f);
                assert(t2b != 1.f);

                [[assume(b2a == true)]];
                [[assume(t0a == 0.f)]];
                [[assume(t2a != 1.f)]];

                [[assume(b2b == true)]];
                [[assume(t1b == 0.f)]];
                [[assume(t2b != 1.f)]];

                const auto p2a = math::lerp(p0, p2, t2a);
                const auto p2b = math::lerp(p1, p2, t2b);

                const auto a2a = lerp_varying(a0, a2, t2a);
                const auto a2b = lerp_varying(a1, a2, t2b);

                vecs_queue.insert(it_vecs, Vec4_Triplet{ p0, p1, p2a });
                attr_queue.insert(it_attr, Vec4_Triplet{ p0, p1, a2a });

                if constexpr (is_CW_winding_order) {
                    *it_vecs = Vec4_Triplet{ p2, p2b, p2a };
                    *it_attr = Attr_Triplet{ a2, a2b, a2a };
                } else {
                    *it_vecs = Vec4_Triplet{ p2, p2a, p2b };
                    *it_attr = Attr_Triplet{ a2, p2a, p2b };
                }
            } break;
            case 3:
            default:
                break;
            }
            ++it_vecs;
            ++it_attr;
        }
    }
    return vecs_queue.size() == 0;
}

// template<bool is_CW_winding_order>
// static bool
// triangle_in_borders(std::list<Vec2_Triplet>& vecs_queue,
//                     std::list<math::F>& z_inv_queue,
//                     std::list<Attr_Triplet>& attr_queue,
//                     const math::Vec2& min,
//                     const math::Vec2& max)
// {
//     return false;
// }

} // namespace asciirast::rasterize
