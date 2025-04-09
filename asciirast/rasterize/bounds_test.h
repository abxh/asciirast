#pragma once

#include <cassert>
#include <deque>
#include <utility>

#include "../constants.h"
#include "../math/types.h"
#include "../program.h"
#include "./interpolate.h"

namespace asciirast::rasterize {

static auto
point_in_screen(const math::Vec2& p) -> bool
{
    const auto min = SCREEN_BOUNDS.min_get();
    const auto max = SCREEN_BOUNDS.max_get();

    const bool x_in_bounds = min.x <= p.x && p.x <= max.x;
    const bool y_in_bounds = min.y <= p.y && p.y <= max.y;

    return x_in_bounds && y_in_bounds;
}

static bool
point_in_frustum(const math::Vec4& p)
{
    if (p.w < 0) {
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
    END = COUNT,
    END2D = TOP + 1U,
};

namespace detail {

static inline BorderType
next_border_type(BorderType b)
{
    return static_cast<BorderType>(static_cast<std::size_t>(b) + 1);
}

} // namespace detail

// Liang-Barsky clipping algorithm:
// https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm
// https://github.com/Larry57/WinForms3D/blob/master/WinForms3D/Clipping/LiangBarskyClippingHomogeneous.cs

static inline bool
line_in_bounds(const T q, const T p, T& t0, T& t1)
{
    // q: delta from border to vector tail
    // p: delta from vector tail to vector head. sign flipped to face
    // border

    // Check if line is parallel to the clipping boundary
    if (math::almost_equal<T>(p, 0)) {
        if (q < 0.f) {
            return false; // Line is outside and parallel, so completely
                          // discarded
        }
        return true; // Line is inside and parallel, so kept as is
    }
    const float t = q / p; // t for the intersection point of the line
                           // and the window edge (possibly projected)

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
line_in_screen(const math::Vec2& p0, const math::Vec2& p1) -> std::optional<std::tuple<math::F, math::F>>
{
    const math::Vec2 min = SCREEN_BOUNDS.min_get();
    const math::Vec2 max = SCREEN_BOUNDS.max_get();

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
// https://youtu.be/HXSuNxpCzdM?feature=shared&t=2155

using Vec4Triplet = std::array<math::Vec4, 3>;

template<VaryingInterface VaryingType>
using AttrsTriplet = std::array<VaryingType, 3>;

namespace detail {

static inline auto
count_num_triangle_vertices_inside(const BorderType border,
                                   const Vec4Triplet& v,
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
count_num_triangle_vertices_inside(const BorderType border, const Vec4Triplet& v)
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

template<unsigned count>
static inline auto
get_ordered_triangle_verticies(const std::array<bool, 3>& inside) -> std::array<unsigned, 3>
{
    // order:
    // ... -> 0 -> 1 -> 2 -> ...

    if constexpr (count == 1) {
        // first point is inside. others are outside.
        if (inside[0]) {
            return { 0, 1, 2 };
        } else if (inside[1]) {
            return { 1, 2, 0 };
        } else if (inside[2]) {
            return { 2, 0, 1 };
        }
    } else if constexpr (count == 2) {
        // first two points are inside. last one is outside.
        if (inside[0] && inside[1]) {
            return { 0, 1, 2 };
        } else if (inside[0] && inside[2]) {
            return { 2, 0, 1 };
        } else if (inside[1] && inside[2]) {
            return { 1, 2, 0 };
        }
    }
    assert(false);
    return { 0, 0, 0 };
}

}; // namespace detail

template<VaryingInterface Varying, typename Vec4TripletAllocatorType, typename AttrAllocatorType>
static bool
triangle_in_screen(std::deque<Vec4Triplet, Vec4TripletAllocatorType>& vec_queue,
                   std::deque<AttrsTriplet<Varying>, AttrAllocatorType>& attrs_queue)
{
    const math::Vec2 min = SCREEN_BOUNDS.min_get();
    const math::Vec2 max = SCREEN_BOUNDS.max_get();

    assert(vec_queue.size() > 0);
    assert(vec_queue.size() == attrs_queue.size());

    for (auto border = BorderType::BEGIN; border < BorderType::END2D; border = detail::next_border_type(border)) {
        auto it_vec = vec_queue.begin();
        auto it_attr = attrs_queue.begin();

        while (it_vec != vec_queue.end()) {
            const auto [count, inside] = detail::count_num_triangle_vertices_inside(border, *it_vec, min, max);
            switch (count) {
            case 0:
                it_vec = vec_queue.erase(it_vec);
                it_attr = attrs_queue.erase(it_attr);
                continue;
            case 1: {
                const auto vec_triplet = *it_vec;
                const auto attrs_triplet = *it_attr;
                const auto [i0, i1, i2] = detail::get_ordered_triangle_verticies<1>(inside);
                const auto [p0, p1, p2] = Vec4Triplet{ vec_triplet[i0], vec_triplet[i1], vec_triplet[i2] };
                const auto [a0, a1, a2] = AttrsTriplet{ attrs_triplet[i0], attrs_triplet[i1], attrs_triplet[i2] };

                math::F t0a = 0.f;
                math::F t0b = 0.f;
                math::F t01 = 1.f;
                math::F t02 = 1.f;

                const bool b01 = line_in_bounds(p0.xy, p1.xy, border, min, max, t0a, t01);
                const bool b02 = line_in_bounds(p0.xy, p2.xy, border, min, max, t0b, t02);

                assert(b01);
                assert(t0a == 0.f);
                assert(t01 != 1.f);

                assert(b02);
                assert(t0b == 0.f);
                assert(t02 != 1.f);

                [[assume(b01 == true)]];
                [[assume(t0a == 0.f)]];
                [[assume(t01 != 1.f)]];

                [[assume(b02 == true)]];
                [[assume(t0b == 0.f)]];
                [[assume(t02 != 1.f)]];

                if (std::isfinite(p0.z) && std::isfinite(p1.z) && std::isfinite(p2.z)) {
                    const auto p01 = math::lerp(p0, p1, t01);
                    const auto p02 = math::lerp(p0, p2, t02);

                    *it_vec = Vec4Triplet{ p0, p01, p02 };
                    *it_attr = AttrsTriplet{ a0,
                                             lerp_varying_perspective_corrected(a0, a1, t01, p0.z, p1.z, p01.z),
                                             lerp_varying_perspective_corrected(a0, a2, t02, p0.z, p2.z, p02.z) };
                } else {
                    *it_vec = Vec4Triplet{ p0, math::lerp(p0, p1, t01), math::lerp(p0, p2, t02) };
                    *it_attr = AttrsTriplet{ a0, lerp_varying(a0, a1, t01), lerp_varying(a0, a2, t02) };
                }
            } break;
            case 2: {
                const auto vec_triplet = *it_vec;
                const auto attrs_triplet = *it_attr;
                const auto [i0, i1, i2] = detail::get_ordered_triangle_verticies<2>(inside);
                const auto [p0, p1, p2] = Vec4Triplet{ vec_triplet[i0], vec_triplet[i1], vec_triplet[i2] };
                const auto [a0, a1, a2] = AttrsTriplet{ attrs_triplet[i0], attrs_triplet[i1], attrs_triplet[i2] };

                math::F t0 = 0.f;
                math::F t1 = 0.f;
                math::F t02 = 1.f;
                math::F t12 = 1.f;

                const bool b02 = line_in_bounds(p0.xy, p2.xy, border, min, max, t0, t02);
                const bool b12 = line_in_bounds(p1.xy, p2.xy, border, min, max, t1, t12);

                assert(b02);
                assert(t0 == 0.f);
                assert(t02 != 1.f);

                assert(b12);
                assert(t1 == 0.f);
                assert(t12 != 1.f);

                [[assume(b02 == true)]];
                [[assume(t0 == 0.f)]];
                [[assume(t02 != 1.f)]];

                [[assume(b12 == true)]];
                [[assume(t1 == 0.f)]];
                [[assume(t12 != 1.f)]];

                const auto p02 = math::lerp(p0, p2, t02);
                const auto p12 = math::lerp(p1, p2, t12);

                Varying a02, a12;
                if (std::isfinite(p0.z) && std::isfinite(p1.z) && std::isfinite(p2.z)) {
                    a02 = lerp_varying_perspective_corrected(a0, a2, t02, p0.z, p2.z, p02.z);
                    a12 = lerp_varying_perspective_corrected(a1, a2, t12, p1.z, p2.z, p12.z);
                } else {
                    a02 = lerp_varying(a0, a2, t02);
                    a12 = lerp_varying(a1, a2, t12);
                }

                *it_vec = Vec4Triplet{ p0, p1, p02 };
                *it_attr = AttrsTriplet{ a0, a1, a02 };

                vec_queue.insert(it_vec, Vec4Triplet{ p1, p12, p02 });
                attrs_queue.insert(it_attr, AttrsTriplet{ a1, a12, a02 });
            } break;
            case 3:
                break;
            default:
                assert(false);
                std::unreachable();
                break;
            }
            ++it_vec;
            ++it_attr;
        }
    }
    return vec_queue.size() != 0;
}

template<VaryingInterface Varying, typename Vec4TripletAllocatorType, typename AttrAllocatorType>
static bool
triangle_in_frustum(std::deque<Vec4Triplet, Vec4TripletAllocatorType>& vec_queue,
                    std::deque<AttrsTriplet<Varying>, AttrAllocatorType>& attrs_queue)
{
    assert(vec_queue.size() > 0);
    assert(vec_queue.size() == attrs_queue.size());

    if (const auto [v0, v1, v2] = *vec_queue.begin(); v0.w < 0 && v1.w < 0 && v2.w < 0) {
        return false;
    }

    for (auto border = BorderType::BEGIN; border < BorderType::END; border = detail::next_border_type(border)) {
        auto it_vec = vec_queue.begin();
        auto it_attr = attrs_queue.begin();

        while (it_vec != vec_queue.end()) {
            const auto [count, inside] = detail::count_num_triangle_vertices_inside(border, *it_vec);
            switch (count) {
            case 0:
                it_vec = vec_queue.erase(it_vec);
                it_attr = attrs_queue.erase(it_attr);
                continue;
            case 1: {
                const auto vec_triplet = *it_vec;
                const auto attrs_triplet = *it_attr;
                const auto [i0, i1, i2] = detail::get_ordered_triangle_verticies<1>(inside);
                const auto [p0, p1, p2] = Vec4Triplet{ vec_triplet[i0], vec_triplet[i1], vec_triplet[i2] };
                const auto [a0, a1, a2] = AttrsTriplet{ attrs_triplet[i0], attrs_triplet[i1], attrs_triplet[i2] };

                const math::Vec3 min = { -p0.w, -p0.w, -p0.w };
                const math::Vec3 max = { +p0.w, +p0.w, +p0.w };

                math::F t0a = 0.f;
                math::F t0b = 0.f;
                math::F t01 = 1.f;
                math::F t02 = 1.f;

                const bool b01 = line_in_bounds(p0, p1, border, min, max, t0a, t01);
                const bool b02 = line_in_bounds(p0, p2, border, min, max, t0b, t02);

                assert(b01);
                assert(t0a == 0.f);
                assert(t01 != 1.f);

                assert(b02);
                assert(t0b == 0.f);
                assert(t02 != 1.f);

                [[assume(b01 == true)]];
                [[assume(t0a == 0.f)]];
                [[assume(t01 != 1.f)]];

                [[assume(b02 == true)]];
                [[assume(t0b == 0.f)]];
                [[assume(t02 != 1.f)]];

                *it_vec = Vec4Triplet{ p0, math::lerp(p0, p1, t01), math::lerp(p0, p2, t02) };
                *it_attr = AttrsTriplet{ a0, lerp_varying(a0, a1, t01), lerp_varying(a0, a2, t02) };
            } break;
            case 2: {
                const auto vec_triplet = *it_vec;
                const auto attrs_triplet = *it_attr;
                const auto [i0, i1, i2] = detail::get_ordered_triangle_verticies<2>(inside);
                const auto [p0, p1, p2] = Vec4Triplet{ vec_triplet[i0], vec_triplet[i1], vec_triplet[i2] };
                const auto [a0, a1, a2] = AttrsTriplet{ attrs_triplet[i0], attrs_triplet[i1], attrs_triplet[i2] };

                const math::Vec3 min0 = { -p0.w, -p0.w, -p0.w };
                const math::Vec3 max0 = { +p0.w, +p0.w, +p0.w };

                const math::Vec3 min1 = { -p1.w, -p1.w, -p1.w };
                const math::Vec3 max1 = { +p1.w, +p1.w, +p1.w };

                math::F t0 = 0.f;
                math::F t1 = 0.f;
                math::F t02 = 1.f;
                math::F t12 = 1.f;

                const bool b02 = line_in_bounds(p0, p2, border, min0, max0, t0, t02);
                const bool b12 = line_in_bounds(p1, p2, border, min1, max1, t1, t12);

                assert(b02);
                assert(t0 == 0.f);
                assert(t02 != 1.f);

                assert(b12);
                assert(t1 == 0.f);
                assert(t12 != 1.f);

                [[assume(b02 == true)]];
                [[assume(t0 == 0.f)]];
                [[assume(t02 != 1.f)]];

                [[assume(b12 == true)]];
                [[assume(t1 == 0.f)]];
                [[assume(t12 != 1.f)]];

                const auto p02 = math::lerp(p0, p2, t02);
                const auto p12 = math::lerp(p1, p2, t12);

                const auto a02 = lerp_varying(a0, a2, t02);
                const auto a12 = lerp_varying(a1, a2, t12);

                *it_vec = Vec4Triplet{ p0, p1, p02 };
                *it_attr = AttrsTriplet{ a0, a1, a02 };

                vec_queue.insert(it_vec, Vec4Triplet{ p1, p12, p02 });
                attrs_queue.insert(it_attr, AttrsTriplet{ a1, a12, a02 });
            } break;
            case 3:
                break;
            default:
                assert(false);
                std::unreachable();
                break;
            }
            ++it_vec;
            ++it_attr;
        }
    }
    return vec_queue.size() != 0;
}

} // namespace asciirast::rasterize
