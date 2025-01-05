/**
 * @file Transform.h
 * @brief Class for stacking primitive transformations on top of each other
 *
 * Using OpenGL conventions for the coordinate systems. Just multiply
 * by the UP / RIGHT / FORWARD basis vectors to be coordinate system agnostic.
 */

#pragma once

#include <cassert>

#include "Mat.h"

namespace asciirast::math {

using Mat2x2f = Mat<2, 2, float>;
using Mat3x3f = Mat<3, 3, float>;
using Mat4x4f = Mat<4, 4, float>;

using Vec2f = Vec<2, float>;
using Vec3f = Vec<3, float>;
using Vec4f = Vec<4, float>;

/**
 * @brief 2D transformation class
 */
class Transform2D {
public:
    static inline const Vec2f RIGHT = Vec2f{1.f, 0.f};
    static inline const Vec2f UP = Vec2f{0.f, -1.f};

public:
    Mat3x3f m_mat;      ///< underlying matrix
    Mat3x3f m_mat_inv;  ///< underlying inverse matrix

    /**
     * Create a new transform object
     */
    Transform2D();

    /**
     * Apply transformation to a 2D Vector
     */
    Vec2f apply(const Vec2f& v) const;

    /**
     * Invert the transformation applied to a 2D Vector
     */
    Vec2f invert(const Vec2f& v) const;

    /**
     * Stack a new transformation matrix and it's inverse on top
     */
    Transform2D stack(const Mat3x3f& mat, const Mat3x3f& inv_mat);

    /**
     * Stack another Transform2D on top of this
     */
    Transform2D stack(const Transform2D& that);

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + delta_x, y + delta_y)
     */
    Transform2D translate(float delta_x, float delta_y);

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + delta.x, y + delta.y)
     */
    Transform2D translate(const Vec2f& delta);

    /**
     * Stack the transformation equivalent to:
     * (x', y') = [right | up] * (x, y) with up = (-right.y, right.x)
     */
    Transform2D rotate(const Vec2f& right, bool is_normalized = false);

    /**
     * Rotate by `angle_x` radians in clockwise direction
     */
    Transform2D rotate_clockwise(float angle_x);

    /**
     * Rotate by `angle_x` radians in counter-clockwise direction
     */
    Transform2D rotate_counterclockwise(float angle_x);

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (scale_x * x, scale_y * y) assuming scale_x * scale_y != 0
     */
    Transform2D scale(float scale_x, float scale_y);

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (scale.x * x, scale.y * y) assuming scale.x * scale.y != 0
     */
    Transform2D scale(const Vec2f& scale);

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + sh_x * y, y)
     */
    Transform2D shearX(float sh_x);

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x, y + sh_y * x)
     */
    Transform2D shearY(float sh_y);
};

/**
 * @brief 3D transformation class
 */
class Transform3D {
public:
    static inline const Vec3f RIGHT = Vec3f{1.f, 0.f, 0.f};
    static inline const Vec3f UP = Vec3f{0.f, 1.f, 0.f};
    static inline const Vec3f FORWARD = Vec3f{0.f, 0.f, -1.f};

private:
    Mat4x4f m_mat;      ///< underlying matrix
    Mat4x4f m_mat_inv;  ///< underlying inverse matrix

    /**
     * Create a new transform object
     */
    Transform3D();

    /**
     * Apply transformation to a 3D Vector
     */
    Vec3f apply(const Vec3f& v) const;

    /**
     * Invert the transformation applied to a 3D Vector
     */
    Vec3f invert(const Vec3f& v) const;

    /**
     * Stack a new transformation matrix and it's inverse on top
     */
    Transform3D stack(const Mat4x4f& mat, const Mat4x4f& inv_mat);

    /**
     * Stack another Transform3D on top of this
     */
    Transform3D stack(const Transform3D& that);

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + delta_x, y + delta_y, z + delta_z)
     */
    Transform3D translate(float delta_x, float delta_y, float delta_z);

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + delta.x, y + delta.y, z + delta.z)
     */
    Transform3D translate(const Vec3f& delta);

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = [right | up | forward] * (x, y, z) with:
     * - right   = up_dir x forward
     * - up      = forward x right
     * following right-hand-rule.
     *
     * @note With OpenGL conventions, camera forward should be -FORWARD by
     * default.
     */
    Transform3D rotate(const Vec3f& forward,
                       const Vec3f& up_dir,
                       bool is_normalized = false);

    /**
     * Rotate by `angle_x` radians measured from RIGHT towards FORWARD
     */
    Transform3D rotateX(float angle_x);

    /**
     * Rotate by `angle_y` radians measured from UP towards FORWARD
     */
    Transform3D rotateY(float angle_y);

    /**
     * Rotate by `angle_z` radians measured from RIGHT towards UP
     */
    Transform3D rotateZ(float angle_z);

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (scale_x * x, scale_y * y, scale_z * z) assuming
     * scale_x * scale_y * scale_z != 0
     */
    Transform3D scale(float scale_x, float scale_y, float scale_z);

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (scale.x * x, scale.y * y, scale.z * z) assuming
     * scale.x * scale.y * scale.z != 0
     */
    Transform3D scale(const Vec3f& scale);

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + sh_x * z, y + sh_y * z, z)
     */
    Transform3D shearXY(float sh_x, float sh_y);

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + sh_x * y, y, z + sh_z * y)
     */
    Transform3D shearXZ(float sh_x, float sh_z);

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x, y + sh_y * x, z + sh_z * x)
     */
    Transform3D shearYZ(float sh_y, float sh_z);
};

}  // namespace asciirast::math
