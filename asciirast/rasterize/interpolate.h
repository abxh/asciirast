/*
    Copyright (C) 2025 abxh

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

#pragma once

#include <cassert>
#include <cfloat>

#include "../math/types.h"
#include "../program.h"

namespace asciirast::rasterize {

// perspective corrected lerp:
// https://andrewkchan.dev/posts/perspective-interpolation.html
// https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf

// note: not sure how to interpolate w, but guessing it's similar to
//       interpolating z.

template<VaryingInterface Varying>
static Varying
lerp_varying_perspective_corrected(const Varying& a,
                                   const Varying& b,
                                   const math::Float t,
                                   const math::Float z_inv0,
                                   const math::Float z_inv1,
                                   const math::Float z_inv_interpolated)
{
    assert(std::isfinite(z_inv_interpolated));
    assert(std::isfinite(z_inv_interpolated));

    const auto l = a * z_inv0 * (1 - t);
    const auto r = b * z_inv1 * t;

    return (l + r) * (1 / z_inv_interpolated);
}

/**
 * @brief Linear interpolation of fragments
 */
template<VaryingInterface T>
static Fragment<T>
lerp(const Fragment<T>& a, const Fragment<T>& b, const math::Float t)
{
    return Fragment<T>{ .pos = math::lerp(a.pos, b.pos, t), .attrs = lerp_varying(a.attrs, b.attrs, t) };
}

/**
 * @brief Linear interpolation of projected fragments
 */
template<VaryingInterface T>
static ProjectedFragment<T>
lerp(const ProjectedFragment<T>& a, const ProjectedFragment<T>& b, const math::Float t)
{
    if (t == math::Float{ 0 }) {
        return a;
    } else if (t == math::Float{ 1 }) {
        return b;
    }

    if (std::isfinite(a.z_inv) && std::isfinite(b.z_inv)) {
        const auto z_inv_interpolated = std::lerp(a.z_inv, b.z_inv, t);

        return ProjectedFragment<T>{ .pos = math::lerp(a.pos, b.pos, t),
                                     .z_inv = z_inv_interpolated,
                                     .w_inv = std::lerp(a.w_inv, b.w_inv, t),
                                     .attrs = lerp_varying_perspective_corrected(
                                             a.attrs, b.attrs, t, a.z_inv, b.z_inv, z_inv_interpolated) };
    } else {
        return ProjectedFragment<T>{ .pos = math::lerp(a.pos, b.pos, t),
                                     .z_inv = std::lerp(a.z_inv, b.z_inv, t),
                                     .w_inv = std::lerp(a.w_inv, b.w_inv, t),
                                     .attrs = lerp_varying(a.attrs, b.attrs, t) };
    }
}

/**
 * @brief Interpolation of vectors with barycentric coordinates of
 *        triangles
 */
static math::Float
barycentric(const math::Vec3& v, const math::Vec3& weights)
{
    return math::dot(v, weights);
}

/**
 * @brief Interpolation of fragments with barycentric coordinates of
 *        triangles
 */
template<VaryingInterface Varying>
static Varying
barycentric(const std::array<Varying, 3>& attrs, const math::Vec3& weights)
{
    const auto aw0 = attrs[0] * weights[0];
    const auto aw1 = attrs[1] * weights[1];
    const auto aw2 = attrs[2] * weights[2];

    return aw0 + aw1 + aw2;
}

/**
 * @brief Interpolation of fragments with barycentric coordinates of
 *        triangles
 */
template<VaryingInterface Varying>
static Varying
barycentric_perspective_corrected(const std::array<Varying, 3>& attrs,
                                  const math::Vec3& weights,
                                  const math::Vec3& z_inv,
                                  const math::Float& z_inv_interpolated)
{
    assert(std::isfinite(z_inv_interpolated));

    const auto wzi = weights * z_inv;
    const auto aw0 = attrs[0] * wzi[0];
    const auto aw1 = attrs[1] * wzi[1];
    const auto aw2 = attrs[2] * wzi[2];

    return (aw0 + aw1 + aw2) * (1 / z_inv_interpolated);
}

}; // namespace asciirast::rasterize
