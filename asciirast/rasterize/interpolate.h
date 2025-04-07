#pragma once

#include <cassert>
#include <cfloat>

#include "../math/types.h"
#include "../program.h"

namespace asciirast::rasterize {

static const inline auto max_depth = std::numeric_limits<math::F>::infinity(); ///@< depth when z = 0

// perspective corrected lerp:
// https://andrewkchan.dev/posts/perspective-interpolation.html
// https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf

// note: not sure how to lerp w, but guessing it's similar to lerping z.

template<VaryingInterface Varying>
static Varying
lerp_varying_perspective_corrected(const Varying& a,
                                   const Varying& b,
                                   const math::F t,
                                   const math::F z_inv0,
                                   const math::F z_inv1)
{
    assert(std::isfinite(z_inv0));
    assert(std::isfinite(z_inv1));

    const auto z_inv_lerped = std::lerp(z_inv0, z_inv1, t);

    const auto l = a * z_inv0 * (1 - t);
    const auto r = b * z_inv1 * t;

    return (l + r) / z_inv_lerped;
}

template<VaryingInterface Varying>
static Varying
lerp_varying_perspective_corrected(const Varying& a,
                                   const Varying& b,
                                   const math::F t,
                                   const math::F z_inv0,
                                   const math::F z_inv1,
                                   const math::F z_inv_lerped)
{
    assert(std::isfinite(z_inv_lerped));
    assert(std::isfinite(z_inv_lerped));

    const auto l = a * z_inv0 * (1 - t);
    const auto r = b * z_inv1 * t;

    return (l + r) / z_inv_lerped;
}

/**
 * @brief Linear interpolation of fragments
 */
template<VaryingInterface T>
static Fragment<T>
lerp(const Fragment<T>& a, const Fragment<T>& b, const math::F t)
{
    return Fragment<T>{ .pos = math::lerp(a.pos, b.pos, t), .attrs = lerp_varying(a.attrs, b.attrs, t) };
}

/**
 * @brief Linear interpolation of projected fragments
 */
template<VaryingInterface T>
static ProjectedFragment<T>
lerp(const ProjectedFragment<T>& a, const ProjectedFragment<T>& b, const math::F t)
{
    if (t == math::F{ 0 }) {
        return a;
    } else if (t == math::F{ 1 }) {
        return b;
    }

    if (std::isfinite(a.z_inv) && std::isfinite(b.z_inv)) {
        return ProjectedFragment<T>{ .pos   = math::lerp(a.pos, b.pos, t),
                                     .z_inv = std::lerp(a.z_inv, b.z_inv, t),
                                     .w_inv = std::lerp(a.w_inv, b.w_inv, t),
                                     .attrs = lerp_varying_perspective_corrected(
                                             a.attrs, b.attrs, t, a.z_inv, b.z_inv) };
    } else {
        return ProjectedFragment<T>{ .pos   = math::lerp(a.pos, b.pos, t),
                                     .z_inv = std::lerp(a.z_inv, b.z_inv, t),
                                     .w_inv = std::lerp(a.w_inv, b.w_inv, t),
                                     .attrs = lerp_varying(a.attrs, b.attrs, t) };
    }
}

/**
 * @brief Project a Fragment to converted it to ProjectFragment
 */
template<VaryingInterface T>
static ProjectedFragment<T>
project(const Fragment<T>& frag)
{
    const auto w_inv = 1 / frag.pos.w;
    const auto vec   = frag.pos.xyz * w_inv;

    return ProjectedFragment<T>{ .pos = vec.xy, .z_inv = 1 / vec.z, .w_inv = w_inv, .attrs = frag.attrs };
}

/**
 * @brief Interpolation of vectors with barycentric coordinates of triangles
 */
static math::F
barycentric(const math::Vec3& v, const math::Vec3& weights)
{
    return math::dot(v, weights);
}

/**
 * @brief Interpolation of fragments with barycentric coordinates of triangles
 */
template<VaryingInterface Varying>
static Varying
barycentric(const Varying& a, const Varying& b, const Varying& c, const math::Vec3& weights)
{
    return a * weights[0] + b * weights[1] + c * weights[2];
}

/**
 * @brief Interpolation of fragments with barycentric coordinates of triangles
 */
template<VaryingInterface Varying>
static Varying
barycentric_perspective_corrected(const Varying& a,
                                  const Varying& b,
                                  const Varying& c,
                                  const math::Vec3& weights,
                                  const math::Vec3& z_inv,
                                  const math::Vec3& z_inv_interpolated)
{
    return (a * weights[0] * z_inv[0] + b * weights[1] * z_inv[1] + c * weights[2] * z_inv[2]) / z_inv_interpolated;
}

/**
 * @brief Interpolation of fragments with barycentric coordinates of triangles
 */
template<VaryingInterface Varying>
static Varying
barycentric_perspective_corrected(const Varying& a,
                                  const Varying& b,
                                  const Varying& c,
                                  const math::Vec3& weights,
                                  const math::Vec3& z_inv)
{
    const auto z_inv_interpolated = barycentric(z_inv, weights);

    return (a * weights[0] * z_inv[0] + b * weights[1] * z_inv[1] + c * weights[2] * z_inv[2]) / z_inv_interpolated;
}

}; // namespace asciirast::rasterize
