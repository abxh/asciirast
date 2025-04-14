#pragma once

#include <cassert>
#include <cfloat>

#include "../math/types.h"
#include "../program.h"

namespace asciirast::rasterize {

// perspective corrected interpolation:
// https://www.youtube.com/watch?v=1Dv2-cLAJXw (ChilliTomatoNoodle)
// https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
// https://en.wikipedia.org/wiki/Homogeneous_coordinates#Introduction

template<VaryingInterface Varying>
static auto
lerp_varying_perspective_corrected(const Varying& a,
                                   const Varying& b,
                                   const math::Float t,
                                   const math::Float Z_inv0,
                                   const math::Float Z_inv1,
                                   const math::Float acc_Z_inv) -> Varying
{
    if constexpr (std::is_same_v<Varying, EmptyVarying>) {
        return EmptyVarying();
    } else {
        // acc_Z_inv := lerp(Z_inv0, Z_inv1, t)

        const auto w0 = (1 - t) * Z_inv0;
        const auto w1 = t * Z_inv1;

        const auto l = a * w0;
        const auto r = b * w1;

        return (l + r) * (1 / acc_Z_inv);
    }
}

/**
 * @brief Linear interpolation of fragments
 */
template<VaryingInterface T>
static auto
lerp(const Fragment<T>& a, const Fragment<T>& b, const math::Float t) -> Fragment<T>
{
    return Fragment<T>{ .pos = math::lerp(a.pos, b.pos, t), .attrs = lerp_varying(a.attrs, b.attrs, t) };
}

/**
 * @brief Linear interpolation of projected fragments
 */
template<VaryingInterface T>
static auto
lerp(const ProjectedFragment<T>& a, const ProjectedFragment<T>& b, const math::Float t) -> ProjectedFragment<T>
{
    if (t == math::Float{ 0 }) {
        return a;
    } else if (t == math::Float{ 1 }) {
        return b;
    }
    const auto Z_inv_t = std::lerp(a.Z_inv, b.Z_inv, t);

    return ProjectedFragment<T>{
        .pos = math::lerp(a.pos, b.pos, t),
        .depth = lerp_varying_perspective_corrected(a.depth, b.depth, t, a.Z_inv, b.Z_inv, Z_inv_t),
        .Z_inv = Z_inv_t,
        .attrs = lerp_varying_perspective_corrected(a.attrs, b.attrs, t, a.Z_inv, b.Z_inv, Z_inv_t)
    };
}

/**
 * @brief Interpolation of vectors with barycentric coordinates of
 *        triangles
 */
[[maybe_unused]]
static auto
barycentric(const math::Vec3& v, const math::Vec3& weights) -> math::Float
{
    return math::dot(v, weights);
}

/**
 * @brief Interpolation of fragments with barycentric coordinates of
 *        triangles
 */
template<VaryingInterface Varying>
static auto
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
 * @brief Interpolation of fragments with barycentric coordinates of
 *        triangles
 */
template<VaryingInterface Varying>
static auto
barycentric_perspective_corrected(const std::array<Varying, 3>& attrs,
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
 * @brief Interpolation of fragments with barycentric coordinates of
 *        triangles
 */
[[maybe_unused]]
static auto
barycentric_perspective_corrected(const math::Vec3& v,
                                  const math::Vec3& weights,
                                  const math::Vec3& Z_inv,
                                  const math::Float& acc_Z_inv) -> math::Float
{
    // acc_Z_inv := barycentric(weights, Z_inv)

    return dot(v, weights * Z_inv) * (1 / acc_Z_inv);
}

}; // namespace asciirast::rasterize
