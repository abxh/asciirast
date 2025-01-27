/**
 * @file Transform.h
 * @brief Class for stacking primitive transformations on top of each other
 * @todo Write rotation matrices more idiomatically and check code
 *
 * Using OpenGL conventions for the coordinate systems. Just multiply
 * by the UP / RIGHT / FORWARD basis vectors to be coordinate system agnostic.
 */

#pragma once

#include <cassert>

#include "Mat.h"

namespace asciirast::math {

using Mat2x2f = Mat<2, 2, float, true>;  ///< 2x2 float column-major matrix
using Mat3x3f = Mat<3, 3, float, true>;  ///< 3x3 float column-major matrix
using Mat4x4f = Mat<4, 4, float, true>;  ///< 4x4 float column-major matrix

using Vec2f = Vec<2, float>;  ///< 2D float math vector
using Vec3f = Vec<3, float>;  ///< 3D float math vector
using Vec4f = Vec<4, float>;  ///< 4D float math vector

/**
 * @brief 2D transformation class
 */
class Transform2D {
public:
    static inline const Vec2f RIGHT = Vec2f{1.f, 0.f};  ///< screen right
    static inline const Vec2f UP = Vec2f{0.f, -1.f};    ///< screen up

public:
    Mat3x3f m_mat;      ///< underlying matrix
    Mat3x3f m_mat_inv;  ///< underlying inverse matrix

    /**
     * Create a new transform object
     */
    Transform2D()
            : m_mat{Mat3x3f::identity()}, m_mat_inv{Mat3x3f::identity()} {}

    /**
     * Apply transformation to a 2D Vector
     */
    Vec2f apply(const Vec2f& v) const { return Vec2f{m_mat * Vec3f{v, 1.f}}; }

    /**
     * Invert the transformation applied to a 2D Vector
     */
    Vec2f invert(const Vec2f& v) const {
        return Vec2f{m_mat_inv * Vec3f{v, 1.f}};
    }

    /**
     * Stack a new transformation matrix and it's inverse on top
     */
    Transform2D stack(const Mat3x3f& mat, const Mat3x3f& inv_mat) {
        m_mat = mat * m_mat;
        m_mat_inv = m_mat_inv * inv_mat;
        return *this;
    }

    /**
     * Stack another Transform2D on top of this
     */
    Transform2D stack(const Transform2D& that) {
        return this->stack(that.m_mat, that.m_mat_inv);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + delta_x, y + delta_y)
     */
    Transform2D translate(const float delta_x, const float delta_y) {
        const auto v = Vec3f{delta_x, delta_y, 1.f};
        const auto mr = Mat3x3f::identity().column_set(2, v);

        const auto vi = Vec3f{-delta_x, -delta_y, 1.f};
        const auto mi = Mat3x3f::identity().column_set(2, vi);

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + delta.x, y + delta.y)
     */
    Transform2D translate(const Vec2f& delta) {
        return this->translate(delta.x, delta.y);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = [right | up] * (x, y) with up = (-right.y, right.x)
     */
    Transform2D rotate(const Vec2f& right_, bool is_normalized = false) {
        const auto right = is_normalized ? right_ : right_.normalized();

        const auto up =
                Vec2f{-right.y, right.x};  // counterclockwise 90 degrees
        const auto mr = Mat3x3f::from_rows(right, up, Vec3f{0.f, 0.f, 1.f});
        const auto mi = mr.transposed();

        return this->stack(mr, mi);
    }

    /**
     * Rotate by `angle_x` radians in clockwise direction
     */
    Transform2D rotate_clockwise(const float angle_x) {
        const auto x = std::cos(angle_x);
        const auto y = std::sin(angle_x);

        return this->rotate(Vec2f{x, y}, true);
    }

    /**
     * Rotate by `angle_x` radians in counter-clockwise direction
     */
    Transform2D rotate_counterclockwise(const float angle_x) {
        return this->rotate_clockwise(-angle_x);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (scale_x * x, scale_y * y) assuming scale_x * scale_y != 0
     */
    Transform2D scale(const float scale_x, const float scale_y) {
        assert(scale_x != 0.f);
        assert(scale_y != 0.f);

        Mat3x3f mr{}, mi{};

        mr(0, 0) = scale_x;
        mr(1, 1) = scale_y;
        mr(2, 2) = 1.f;

        mi(0, 0) = 1.f / scale_x;
        mi(1, 1) = 1.f / scale_y;
        mi(2, 2) = 1.f;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (scale.x * x, scale.y * y) assuming scale.x * scale.y != 0
     */
    Transform2D scale(const Vec2f& scale) {
        return this->scale(scale.x, scale.y);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + sh_x * y, y)
     */
    Transform2D shearX(const float sh_x) {
        const auto a = Vec3f{1.0f, sh_x, 0.0f};
        const auto b = Vec3f{0.0f, 1.0f, 0.0f};
        const auto c = Vec3f{0.0f, 0.0f, 1.0f};
        const auto mr = Mat3x3f::from_rows(a, b, c);

        auto mi = Mat3x3f{mr};
        mi(0, 1) = -sh_x;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x, y + sh_y * x)
     */
    Transform2D shearY(const float sh_y) {
        const auto a = Vec3f{1.0f, 0.f, 0.f};
        const auto b = Vec3f{sh_y, 1.f, 0.f};
        const auto c = Vec3f{0.0f, 0.f, 1.f};
        const auto mr = Mat3x3f::from_rows(a, b, c);

        auto mi = Mat3x3f{mr};
        mi(1, 0) = -sh_y;

        return this->stack(mr, mi);
    }
};

/**
 * @brief 3D transformation class
 */
class Transform3D {
public:
    static inline const Vec3f RIGHT = Vec3f{1.f, 0.f, 0.f};  ///< world right
    static inline const Vec3f UP = Vec3f{0.f, 1.f, 0.f};     ///< world up
    static inline const Vec3f FORWARD =
            Vec3f{0.f, 0.f, -1.f};  ///< world forward

private:
    Mat4x4f m_mat;      ///< underlying matrix
    Mat4x4f m_mat_inv;  ///< underlying inverse matrix

    /**
     * Create a new transform object
     */
    Transform3D()
            : m_mat{Mat4x4f::identity()}, m_mat_inv{Mat4x4f::identity()} {}

    /**
     * Apply transformation to a 3D Vector
     */
    Vec3f apply(const Vec3f& v) const { return Vec3f{m_mat * Vec4f{v, 1.f}}; }

    /**
     * Invert the transformation applied to a 3D Vector
     */
    Vec3f invert(const Vec3f& v) const {
        return Vec3f{m_mat_inv * Vec4f{v, 1.f}};
    }

    /**
     * Stack a new transformation matrix and it's inverse on top
     */
    Transform3D stack(const Mat4x4f& mat, const Mat4x4f& inv_mat) {
        m_mat = mat * m_mat;
        m_mat_inv = m_mat_inv * inv_mat;
        return *this;
    }

    /**
     * Stack another Transform3D on top of this
     */
    Transform3D stack(const Transform3D& that) {
        return this->stack(that.m_mat, that.m_mat_inv);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + delta_x, y + delta_y, z + delta_z)
     */
    Transform3D translate(const float delta_x,
                          const float delta_y,
                          const float delta_z) {
        const auto v = Vec4f{delta_x, delta_y, delta_z, 1.f};
        const auto mr = Mat4x4f::identity().column_set(3, v);

        const auto vi = Vec4f{-delta_x, -delta_y, -delta_z, 1.f};
        const auto mi = Mat4x4f::identity().column_set(3, vi);

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + delta.x, y + delta.y, z + delta.z)
     */
    Transform3D translate(const Vec3f& delta) {
        return this->translate(delta.x, delta.y, delta.z);
    }

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
    Transform3D rotate(const Vec3f& forward_,
                       const Vec3f& up_dir_,
                       bool is_normalized = false) {
        const auto up_dir = is_normalized ? up_dir_ : up_dir_.normalized();
        const auto forward = is_normalized ? forward_ : forward_.normalized();

        const auto right = cross(up_dir, forward);
        const auto up = cross(forward, right);

        const auto mr = Mat4x4f{Mat3x3f::from_rows(right, up, forward),
                                Vec4f{0.f, 0.f, 0.f, 1.f}};
        const auto mi = mr.transposed();

        return this->stack(mr, mi);
    }

    /**
     * Rotate by `angle_x` radians measured from RIGHT towards FORWARD
     */
    Transform3D rotateX(const float angle_x) {
        const auto up_dir_y = std::cos(angle_x);
        const auto up_dir_z = std::sin(angle_x);
        const auto forward_y = -up_dir_z;
        const auto forward_z = up_dir_y;

        return this->rotate(Vec3f{0.f, forward_y, forward_z},
                            Vec3f{0.f, up_dir_y, up_dir_z}, true);
    }

    /**
     * Rotate by `angle_y` radians measured from UP towards FORWARD
     */
    Transform3D rotateY(const float angle_y) {
        const auto up_dir = UP;
        const auto forward_x = std::cos(angle_y);
        const auto forward_z = std::sin(angle_y);

        return this->rotate(Vec3f{forward_x, 0.f, forward_z}, up_dir, true);
    }

    /**
     * Rotate by `angle_z` radians measured from RIGHT towards UP
     */
    Transform3D rotateZ(const float angle_z) {
        const auto forward = FORWARD;
        const auto up_dir_x = std::cos(angle_z);
        const auto up_dir_y = std::sin(angle_z);

        return this->rotate(forward, Vec3f{up_dir_x, up_dir_y, 0.f}, true);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (scale_x * x, scale_y * y, scale_z * z) assuming
     * scale_x * scale_y * scale_z != 0
     */
    Transform3D scale(const float scale_x,
                      const float scale_y,
                      const float scale_z) {
        assert(scale_x != 0.f);
        assert(scale_y != 0.f);
        assert(scale_z != 0.f);

        Mat4x4f mr{}, mi{};

        mr(0, 0) = scale_x;
        mr(1, 1) = scale_y;
        mr(2, 2) = scale_z;
        mr(3, 3) = 1.f;

        mi(0, 0) = 1.f / scale_x;
        mi(1, 1) = 1.f / scale_y;
        mi(2, 2) = 1.f / scale_z;
        mi(3, 3) = 1.f;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (scale.x * x, scale.y * y, scale.z * z) assuming
     * scale.x * scale.y * scale.z != 0
     */
    Transform3D scale(const Vec3f& scale) {
        return this->scale(scale.x, scale.y, scale.z);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + sh_x * z, y + sh_y * z, z)
     */
    Transform3D shearXY(const float sh_x, const float sh_y) {
        auto a = Vec4f{1.f, 0.f, sh_x, 0.f};
        auto b = Vec4f{0.f, 1.f, sh_y, 0.f};
        auto c = Vec4f{0.f, 0.f, 1.0f, 0.f};
        auto d = Vec4f{0.f, 0.f, 0.0f, 1.f};

        auto mr = Mat4x4f::from_rows(a, b, c, d);

        auto mi = Mat4x4f{mr};
        mi(0, 2) = -sh_x;
        mi(1, 2) = -sh_y;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + sh_x * y, y, z + sh_z * y)
     */
    Transform3D shearXZ(const float sh_x, const float sh_z) {
        auto a = Vec4f{1.f, sh_x, 0.f, 0.f};
        auto b = Vec4f{0.f, 1.0f, 0.f, 0.f};
        auto c = Vec4f{0.f, sh_z, 1.f, 0.f};
        auto d = Vec4f{0.f, 0.0f, 0.f, 1.f};

        auto mr = Mat4x4f::from_rows(a, b, c, d);

        auto mi = Mat4x4f{mr};
        mi(0, 1) = -sh_x;
        mi(2, 1) = -sh_z;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x, y + sh_y * x, z + sh_z * x)
     */
    Transform3D shearYZ(const float sh_y, const float sh_z) {
        auto a = Vec4f{1.0f, 0.f, 0.f, 0.f};
        auto b = Vec4f{sh_y, 1.f, 0.f, 0.f};
        auto c = Vec4f{sh_z, 0.f, 1.f, 0.f};
        auto d = Vec4f{0.0f, 0.f, 0.f, 1.f};

        auto mr = Mat4x4f::from_rows(a, b, c, d);

        auto mi = Mat4x4f{mr};
        mi(1, 0) = -sh_y;
        mi(2, 0) = -sh_z;

        return this->stack(mr, mi);
    }
};

}  // namespace asciirast::math
