/**
 * @file misc.hpp
 * @brief Miscellaneous functions and constants
 *
 * reverse depth:
 * - https://developer.nvidia.com/blog/visualizing-depth-precision/
 * -
 * https://tomhultonharrop.com/mathematics/graphics/2023/08/06/reverse-z.html
 */

#pragma once

#include "./fragment.hpp"
#include "./math/types.hpp"

namespace asciirast {

static constexpr math::Vec3 RIGHT = { 1, 0, 0 };   ///< "right" axis
static constexpr math::Vec3 UP = { 0, 1, 0 };      ///< "up" axis
static constexpr math::Vec3 FORWARD = { 0, 0, 1 }; ///< "forward" axis

/**
 * @brief Normalized Device Coordinates boundary
 *
 * This the bounding box, vertices are to be mapped to, before vertices
 * outside of it are clipped / culled.
 *
 * @note The z coordinate is used for depth. z-near should be mapped to
 * 1, and z-far to 0, and all other z-values are linearly interpolated
 * between them.
 */
static inline constexpr auto NDC_BOUNDS = math::AABB3D::from_min_max({ -1, -1, 0 }, { +1, +1, +1 });

/**
 * @brief Screen boundary
 *
 * This is bounding box used for screen clipping, in case the given
 * viewport maps points outside of it.
 *
 * It is meant to be used to easily compose Transforms from the screen
 * coordinates to the desired framebuffer coordinates.
 */
static inline constexpr auto SCREEN_BOUNDS = math::AABB2D::from_min_max({ -1, -1 }, { +1, +1 });

/**
 * @brief Calculate the linear reverse depth, given z-distances to near
 * and far planes and the z-value itself.
 *
 * @param z The z value
 * @param near Z-Distance to the near plane
 * @param far Z-Distance to the far plane
 * @return The reverse depth between 1 (meaning z = near) and 0 (meaning
 * z = far)
 */
[[maybe_unused]] static math::Float
compute_reverse_depth_linear(const math::Float z, const math::Float near, const math::Float far)
{
    ASCIIRAST_ASSERT(math::almost_equal<math::Float>(near, far) == false, "near is not equal to far", near, far);

    /* Solving:
        depth(z) is linear
       With constraints:
        depth(near) = 1
        depth(far)  = 0
       Gives:
        depth(z) = 1 - (z - near) / (far - near) <=>
                 = (far - z) / (far - near)
     */

    return (far - z) / (far - near);
}

/**
 * @brief Calculate the hyperbolic reverse depth, given z-distances to
 * near and far planes and the z-value itself.
 *
 * This quantity is to be divided by z.
 *
 * @param z The z value
 * @param near Z-Distance to the near plane
 * @param far Z-Distance to the far plane
 * @return The reverse depth between 1 (meaning z = near) and 0 (meaning
 * z = far)
 */
[[maybe_unused]] static math::Float
compute_reverse_depth_hyperbolic(const math::Float z, const math::Float near, const math::Float far)
{
    ASCIIRAST_ASSERT(math::almost_equal<math::Float>(near, far) == false, "near is not equal to far", near, far);

    /* Solving:
        A z + B     = depth z
        A   + B / z = depth   <=>
       With constraints:
        A + B / near = 1
        A + B / far  = 0
       Gives:
        A = -near / (far - near)
        B = -far * A
    */

    const auto A = -near / (far - near);
    const auto B = -far * A;

    return A * z + B;
}

/**
 * @brief Make orthographic matrix
 *
 * @param near Z-Distance to the near plane
 * @param far Z-Distance to the far plane
 * @param min_ Minimum xy-coordinate of camera bounding box
 * @param max_ Maximum xy-coordinate of camera bounding box
 * @return A transform object that can map the camera bounding box to
 * the NDC bounding box and back.
 */
[[maybe_unused]] static math::Transform3D
make_orthographic(const math::Float near,
                  const math::Float far,
                  const math::Vec2 min_ = { -1, -1 },
                  const math::Vec2 max_ = { +1, +1 })
{
    return math::AABB3D::from_min_max({ min_, near }, { max_, far }) //
        .to_transform()
        .inversed()
        .stack(NDC_BOUNDS.to_transform())
        .reflectZ()
        .translate(0, 0, 1);
}

/**
 * @brief Make perspective matrix
 *
 * Assuming a symmetrical camera frustum. The camera frustum is the
 * volumen the camera can see.
 *
 * The perspective matrix brings points in the camera frustum to the NDC
 * bounding box. Depth is assumed to lie between 0 and 1 instead of -1
 * to 1.
 *
 * @param near Z-Distance to the near plane
 * @param far Z-Distance to the far plane
 * @param aspect_ratio Fraction to multiply to adjust to the ratio
 * between screen width and screen height.
 * @param fovy_rad The fov y angle in radians
 * @return A transform object that can map the camera frustum to the NDC
 * bounding box and back.
 */
[[maybe_unused]] static math::Transform3D
make_perspective(const math::Float near,
                 const math::Float far,
                 const math::Float fovy_rad = math::radians<math::Float>(90),
                 const math::Float aspect_ratio = 1.f)
{
    /*
     * perspective projection matrix:
     * - https://www.youtube.com/watch?v=EqNcqBdrNyI
     * - https://www.youtube.com/watch?v=k_L6edKHKfA
     * - http://www.songho.ca/opengl/gl_projectionmatrix.html
     */

    ASCIIRAST_ASSERT(tan(fovy_rad / 2.0f) != 0.f, "tangent to half fov angle is not 0", fovy_rad);
    ASCIIRAST_ASSERT(aspect_ratio != 0.f, "aspect_ratio not 0");

    const auto tan_half_fov = tan(fovy_rad / 2.0f);
    const auto sx = tan_half_fov * aspect_ratio;
    const auto sy = tan_half_fov;

    ASCIIRAST_ASSERT(math::almost_equal<math::Float>(near, far) == false, "near is not equal to far", near, far);

    const auto A = -near / (far - near);
    const auto B = -far * A;

    const auto mat = math::Mat4::from_rows(math::Vec4{ 1 / sx, 0, 0, 0 }, // x' = x / sx
                                           math::Vec4{ 0, 1 / sy, 0, 0 }, // y' = y / sy
                                           math::Vec4{ 0, 0, A, B },      // z' = A * z + B * w, assuming w = 1
                                           math::Vec4{ 0, 0, 1, 0 });     // w' = z

    const auto mat_inv = math::Mat4::from_rows(math::Vec4{ sx, 0, 0, 0 },          // x = x' * sx
                                               math::Vec4{ 0, sy, 0, 0 },          // y = y' * sy
                                               math::Vec4{ 0, 0, 0, 1 },           // z = w'
                                               math::Vec4{ 0, 0, 1 / B, -A / B }); // w = 1/B * z' - A/B * w'

    return math::Transform3D().stack(mat, mat_inv);
}

/**
 * @brief Triangulate fragment quad and get indicies of triangles
 */
template<WindingOrder winding_order, VaryingInterface Varying>
[[maybe_unused]] static std::array<std::size_t, 6>
triangulate_fragment_quad(const std::array<Fragment<Varying>, 4>& fragments)
{
    const auto& [f0, f1, f2, f3] = fragments;
    const auto v0 = f0.pos.xyz.to_vec();
    const auto v1 = f1.pos.xyz.to_vec();
    const auto v2 = f2.pos.xyz.to_vec();
    const auto v3 = f3.pos.xyz.to_vec();

    ASCIIRAST_ASSERT(math::almost_equal(dot(cross(v0.vector_to(v1), v0.vector_to(v2)), v0.vector_to(v3)), 0.f),
                     "all given points lie on the same plane",
                     v0,
                     v1,
                     v2,
                     v3);

    // pick the smaller of the two diagonals:
    const auto v02 = v0.vector_to(v2);
    const auto v13 = v1.vector_to(v3);

    if (dot(v02, v02) < dot(v13, v13)) {
        /*
            0 --- 1
            |  \  |
            3 --- 2
        */
        if constexpr (winding_order == WindingOrder::Clockwise) {
            return { 0, 1, 2, 2, 3, 0 };
        } else {
            return { 1, 0, 2, 2, 0, 3 };
        }
    } else {
        /*
            0 --- 1
            |  /  |
            3 --- 2
        */
        if constexpr (winding_order == WindingOrder::Clockwise) {
            return { 0, 1, 3, 3, 1, 2 };
        } else {
            return { 0, 3, 1, 1, 3, 2 };
        }
    }
}

} // namespace asciirast
