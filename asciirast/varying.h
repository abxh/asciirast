/**
 * @file varying_mixin.h
 * @brief Definition of Varying Mixin
 */

#pragma once

#include "./math/types.h"

#include <type_traits>

// on perspective corrected interpolation:
// https://www.youtube.com/watch?v=1Dv2-cLAJXw (ChilliTomatoNoodle)
// https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf

namespace asciirast {

/**
 * @brief Empty varying type
 */
struct EmptyVarying
{};

/**
 * @brief Concept to follow the varying interface
 *
 * Varying are the interpolated attributes of verticies.
 */
template<typename T>
concept VaryingInterface = std::same_as<T, EmptyVarying> || requires(const T x) {
    { x + x } -> std::same_as<T>;
    { x * math::Float{ -1 } } -> std::same_as<T>;
    requires std::semiregular<T>;
};

/**
 * @brief Linear interpolation of varying types
 */
template<VaryingInterface Varying>
static Varying
lerp_varying(const Varying& a, const Varying& b, const math::Float t)
{
    if constexpr (std::is_same_v<Varying, EmptyVarying>) {
        return EmptyVarying();
    } else {
        return a * (1 - t) + b * t;
    }
}

/**
 * @brief Linear interpolation of varying perspective corrected
 */
template<VaryingInterface Varying>
[[maybe_unused]]
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
        if (t == 0) {
            return a;
        } else if (t == 1) {
            return b;
        }
        // acc_Z_inv := lerp(Z_inv0, Z_inv1, t)

        const auto w0 = (1 - t) * Z_inv0;
        const auto w1 = t * Z_inv1;

        const auto l = a * w0;
        const auto r = b * w1;

        return (l + r) * (1 / acc_Z_inv);
    }
}

/**
 * @brief Interpolation of vectors with barycentric coordinates of
 *        triangles
 */
[[maybe_unused]]
static auto
barycentric(const math::Vec3& v, const math::Vec3& weights) -> math::Float
{
    return dot(v, weights);
}

/**
 * @brief Interpolation of fragments with barycentric coordinates of
 *        triangles
 */
template<VaryingInterface Varying>
[[maybe_unused]]
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
[[maybe_unused]]
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

};
