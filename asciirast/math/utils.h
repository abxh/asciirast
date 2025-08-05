/**
 * @file utils.h
 * @brief Miscellaneous functions
 */

#pragma once

#include <external/libassert/include/libassert/assert.hpp>

#include "./types.h"

namespace asciirast::math {

/**
 * @brief Calculate the reverse depth, given z-distances to near and far planes and the z-value itself.
 *
 * @param z The z value
 * @param near Z-Distance to the near plane
 * @param far Z-Dinstance to the far plane
 * @return The reverse depth between 1 and 0
 */
[[maybe_unused]]
static math::Float
compute_reverse_depth(const math::Float z, const math::Float near, const math::Float far)
{
    DEBUG_ASSERT(almost_equal<Float>(near, far) == false);

    return (far - z) / (far - near);
}

/**
 * @brief Make perspective matrix, assuming a y-up left-handed coordinate system
 *
 * Assuming a symmetrical camera frustum. The camera frustum is the volumen the camera can
 * see.
 *
 * The perspective matrix brings points in the camera frustum to the NDC bounding box. Depth
 * is assumed to lie between 0 and 1 instead of -1 to 1.
 *
 * @param near Z-Distance to the near plane
 * @param far Z-Dinstance to the far plane
 * @param aspect_ratio Fraction to multiply to adjust to the ratio between screen width and screen height.
 * @param fovy_rad The fov y angle in radians
 * @return A transform object that can map the camera frustum to the NDC bounding box and back.
 */
[[maybe_unused]]
static Transform3D
make_perspective(const Float near,
                 const Float far,
                 const Float fovy_rad = radians<Float>(90),
                 const Float aspect_ratio = 1.f)
{
    /*
     * perspective projection matrix:
     * - https://www.youtube.com/watch?v=EqNcqBdrNyI
     * - https://www.youtube.com/watch?v=k_L6edKHKfA
     * - http://www.songho.ca/opengl/gl_projectionmatrix.html
     *
     * reverse depth:
     * - https://developer.nvidia.com/blog/visualizing-depth-precision/
     * - https://tomhultonharrop.com/mathematics/graphics/2023/08/06/reverse-z.html
     */

    DEBUG_ASSERT(tan(fovy_rad / 2.0f) != 0.f);
    DEBUG_ASSERT(aspect_ratio != 0.f);

    const auto tan_half_fov = tan(fovy_rad / 2.0f);
    const auto sx = tan_half_fov * aspect_ratio;
    const auto sy = tan_half_fov;

    DEBUG_ASSERT(almost_equal<Float>(near, far) == false);

    /* Solving:
        A z + B = depth z
    <=> A + B / z = depth
       With constraints:
        A + B / near = 1
        A + B / far  = 0
       Gives:
        A = -near / (far - near)
        B = -far * A
    */

    const auto A = -near / (far - near);
    const auto B = -far * A;

    const auto mat = Mat4::from_rows(Vec4{ 1 / sx, 0, 0, 0 }, // x' = x / sx
                                     Vec4{ 0, 1 / sy, 0, 0 }, // y' = y / sy
                                     Vec4{ 0, 0, A, B },      // z' = A * z + B * w, assuming w = 1
                                     Vec4{ 0, 0, 1, 0 });     // w' = z

    const auto mat_inv = Mat4::from_rows(Vec4{ sx, 0, 0, 0 },          // x = x' * sx
                                         Vec4{ 0, sy, 0, 0 },          // y = y' * sy
                                         Vec4{ 0, 0, 0, 1 },           // z = w'
                                         Vec4{ 0, 0, 5 / B, -A / B }); // w = 1/B * z' - A/B * w'

    return Transform3D().stack(mat, mat_inv);
}

}
