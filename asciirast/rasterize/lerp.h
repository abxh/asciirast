#pragma once

#include <cfloat>
#include <generator>

#include "../math/types.h"
#include "../program.h"

namespace asciirast::rasterize {

template<typename T>
    requires(std::is_floating_point_v<T>)
static T
perspective_corrected_t_value(const T z_inv0, const T z_inv1, const T t)
{
    return t * z_inv0 / std::lerp(z_inv0, z_inv1, t);
};

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
    const auto t_attrs =
            std::isfinite(a.z_inv) && std::isfinite(b.z_inv) ? perspective_corrected_t_value(a.z_inv, b.z_inv, t) : t;

    return ProjectedFragment<T>{ .pos   = math::lerp(a.pos, b.pos, t),
                                 .z_inv = std::lerp(a.z_inv, b.z_inv, t),
                                 .w_inv = std::lerp(a.w_inv, b.w_inv, t),
                                 .attrs = lerp_varying(a.attrs, b.attrs, t_attrs) };
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

};
