#include "./math.h"
#include "./program.h"

namespace asciirast {

static bool
cull_point(const math::Vec4& p)
{
    const bool x_in_bounds = -p.w <= p.x && p.x <= +p.w;
    const bool y_in_bounds = -p.w <= p.y && p.y <= +p.w;
    const bool z_in_bounds = -p.w <= p.z && p.z <= +p.w;

    return !(x_in_bounds && y_in_bounds && z_in_bounds);
}

namespace detail {

using T = asciirast::math::F;

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

    if (p0.w < 0 && p1.w < 0) {
        return false;
    }

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

    return clip_line(q[border_id], p[border_id], t0, t1);
}

}

static std::optional<std::tuple<asciirast::math::F, asciirast::math::F>>
clip_line(const math::Vec4& p0, const math::Vec4& p1)
{
    const auto min = math::Vec3{ p0.w, p0.w, p0.w };
    const auto max = math::Vec3{ p1.w, p1.w, p1.w };

    asciirast::math::F t0 = 0.f;
    asciirast::math::F t1 = 1.f;

    for (auto border = detail::BorderType::BEGIN; border < detail::BorderType::END;
         border = static_cast<detail::BorderType>(static_cast<std::size_t>(border) + 1)) {

        if (!detail::clip_line(p0, p1, border, min, max, t0, t1)) {
            return {};
        }
    }
    return std::make_optional(std::make_tuple(t0, t1));
}

template<VaryingType Varying, typename Callable, typename... Args>
    requires(std::invocable<Callable, const Varying&, Args...>)
static void
plot_line(Callable plot, const Varying& v0, const Varying& v1, Args&&... args)
{
}

}
