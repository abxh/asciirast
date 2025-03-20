
#pragma once

#include "./math.h"
#include "./program.h"
#include <cassert>

namespace asciirast::detail {

static bool
cull_point(const math::Vec4& p)
{
    const bool x_in_bounds = -p.w <= p.x && p.x <= +p.w;
    const bool y_in_bounds = -p.w <= p.y && p.y <= +p.w;
    const bool z_in_bounds = -p.w <= p.z && p.z <= +p.w;

    return !(x_in_bounds && y_in_bounds && z_in_bounds);
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
};

static inline bool
clip_line(const T q, const T p, T& t0, T& t1)
{
    // Liang-Barsky clipping algorithm:
    // - https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm

    // q: delta from border to vector tail
    // p: delta from vector tail to vector head. sign flipped to face border

    if (math::almost_equal<T>(p, 0)) { // Check if line is parallel to the clipping boundary
        if (q < 0.f) {
            return false; // Line is outside and parallel, so completely discarded
        }
        return true; // Line is inside and parallel, so kept as is
    }
    const float t = q / p; // t for the intersection point of the line and the window edge (possibly projected)

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
clip_line(const math::Vec4& p0,
          const math::Vec4& p1,
          const BorderType border,
          const math::Vec3& min,
          const math::Vec3& max,
          T& t0,
          T& t1)
{
    // Liang-Barsky clipping algorithm:
    // - https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm
    // - https://github.com/Larry57/WinForms3D/blob/master/WinForms3D/Clipping/LiangBarskyClippingHomogeneous.cs

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

    return clip_line(q[border_id], -delta.w + p[border_id], t0, t1);
}

static std::optional<std::tuple<math::F, math::F>>
clip_line(const math::Vec4& p0, const math::Vec4& p1)
{
    if (p0.w < 0 && p1.w < 0) {
        return {};
    }

    const auto min = math::Vec3{ -p0.w, -p0.w, -p0.w };
    const auto max = math::Vec3{ +p0.w, +p0.w, +p0.w };

    auto t0 = math::F{ 0 };
    auto t1 = math::F{ 1 };

    for (auto border = BorderType::BEGIN; border < BorderType::END;
         border = static_cast<BorderType>(static_cast<std::size_t>(border) + 1)) {

        if (!clip_line(p0, p1, border, min, max, t0, t1)) {
            return {};
        }
    }
    return std::make_optional(std::make_tuple(t0, t1));
}

template<ProjectedFragmentType ProjectedFragment, typename Callable, typename... Args>
    requires(std::invocable<Callable, const ProjectedFragment &&, Args...>)
static void
plot_line(Callable plot, const ProjectedFragment& frag0, const ProjectedFragment& frag1, Args&&... args)
{
    // line drawing based on linear interpolation:
    // https://www.redblobgames.com/grids/line-drawing/#more

    const auto p_delta = frag1.pos - frag0.pos;
    const auto max_len = static_cast<math::I>(std::max(std::abs(p_delta.x), std::abs(p_delta.y)));

    if (max_len == 0U) {
        return;
    }

    auto attr_t_func = [&](math::F t) -> math::F {
        // perspective-corrected / hyperbolic interpolation
        return t * frag0.depth / ((1 - t) * frag0.depth + t * frag1.depth);
    };

    const auto t_step = math::F{ 1 } / max_len;
    const auto p_step = t_step * p_delta;
    const auto d_step = t_step * (frag1.depth - frag0.depth);

    auto t_curr = math::F{ 0 };
    auto p_curr = frag0.pos;
    auto d_curr = frag0.depth;

    for (math::I i = 0; i <= max_len; i++) {
        plot(ProjectedFragment{ .pos = p_curr,
                                .depth = d_curr,
                                .attrs = lerp(frag0.attrs, frag1.attrs, attr_t_func(t_curr)) },
             std::forward<Args>(args)...);

        t_curr += t_step;
        p_curr += p_step;
        d_curr += d_step;
    }
}

}
