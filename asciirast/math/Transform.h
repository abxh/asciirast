/**
 * @file Transform.h
 * @brief Class for stacking primitive invertible transformations on top of each other
 */

#pragma once

#include <cassert>

#include "Complex.h"
#include "Mat.h"

namespace asciirast::math {

/**
 * @brief 2D transformation class
 */
template <typename T>
    requires(std::is_floating_point_v<T>)
class Transform2D {
public:
    Mat3x3<T> m_mat;      ///< underlying matrix
    Mat3x3<T> m_mat_inv;  ///< underlying inverse matrix

    /**
     * Create a new transform object
     */
    Transform2D()
            : m_mat{Mat3x3<T>::identity()}, m_mat_inv{Mat3x3<T>::identity()} {}

    /**
     * Apply transformation to a 2D Vector
     */
    Vec2<T> apply(const Vec2<T>& v) const {
        return Vec2<T>{m_mat * Vec3<T>{v, 1}};
    }

    /**
     * Invert the transformation applied to a 2D Vector
     */
    Vec2<T> invert(const Vec2<T>& v) const {
        return Vec2<T>{m_mat_inv * Vec3<T>{v, 1}};
    }

    /**
     * Stack a new transformation matrix and it's inverse on top
     */
    Transform2D stack(const Mat3x3<T>& mat, const Mat3x3<T>& inv_mat) {
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
    Transform2D translate(const T delta_x, const T delta_y) {
        const auto vr = Vec3<T>{delta_x, delta_y, 1};
        const auto vi = Vec3<T>{-delta_x, -delta_y, 1};

        const auto mr = Mat3x3<T>::identity().column_set(2, vr);
        const auto mi = Mat3x3<T>::identity().column_set(2, vi);

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + delta.x, y + delta.y)
     */
    Transform2D translate(const Vec2<T>& delta) {
        return this->translate(delta.x, delta.y);
    }

    /**
     * Stack the transformation which aligns the x-axis with v
     */
    Transform2D rotate(const Complex<T>& v, bool is_normalized = false) {
        const auto x_basis_c = is_normalized ? v : v.normalized();
        const auto y_basis_c = Complex<T>{0, 1} * x_basis_c;

        const auto x_basis = x_basis_c.m_vec;
        const auto y_basis = y_basis_c.m_vec;
        const auto z_basis = Vec3<T>{0, 0, 1};

        const Mat3x3<T> mr{x_basis, y_basis, z_basis};
        const Mat3x3<T> mi = mr.transposed();

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation which aligns the x-axis with v
     */
    Transform2D rotate(const Vec2<T>& v, bool is_normalized = false) {
        return rotate(Complex<T>{v}, is_normalized);
    }

    /**
     * Stack the transformation which performs a rotation of `theta` radians in
     * counterclockwise direction
     */
    Transform2D rotate_counterclockwise(const Angle<T> theta) {
        return this->rotate(Complex<T>::from_angle(theta), true);
    }

    /**
     * Stack the transformation which performs a rotation of `theta` radians in
     * clockwise direction
     */
    Transform2D rotate_clockwise(const Angle<T> theta) {
        return this->rotate(Complex<T>::from_angle(-theta), true);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (scale_x * x, scale_y * y) assuming scale_x * scale_y != 0
     */
    Transform2D scale(const T scale_x, const T scale_y) {
        assert(scale_x != 0);
        assert(scale_y != 0);

        Mat3x3<T> mr{}, mi{};

        mr(0, 0) = scale_x;
        mr(1, 1) = scale_y;
        mr(2, 2) = 1;

        mi(0, 0) = 1 / scale_x;
        mi(1, 1) = 1 / scale_y;
        mi(2, 2) = 1;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (scale.x * x, scale.y * y) assuming scale.x * scale.y != 0
     */
    Transform2D scale(const Vec2<T>& scale) {
        return this->scale(scale.x, scale.y);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + t * y, y)
     */
    Transform2D shearX(const T t) {
        const auto a = Vec3<T>{1, t, 0};
        const auto b = Vec3<T>{0, 1, 0};
        const auto c = Vec3<T>{0, 0, 1};
        const auto mr = Mat3x3<T>::from_rows(a, b, c);

        auto mi = Mat3x3<T>{mr};
        mi(0, 1) *= -1;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x, y + t * x)
     */
    Transform2D shearY(const T t) {
        const auto a = Vec3<T>{1, 0, 0};
        const auto b = Vec3<T>{t, 1, 0};
        const auto c = Vec3<T>{0, 0, 1};
        const auto mr = Mat3x3<T>::from_rows(a, b, c);

        auto mi = Mat3x3<T>{mr};
        mi(1, 0) *= -1;

        return this->stack(mr, mi);
    }
};

/**
 * @brief 3D transformation class
 */
template <typename T>
    requires(std::is_floating_point_v<T>)
class Transform3D {
private:
    Mat4x4<T> m_mat;      ///< underlying matrix
    Mat4x4<T> m_mat_inv;  ///< underlying inverse matrix

    /**
     * Create a new transform object
     */
    Transform3D()
            : m_mat{Mat4x4<T>::identity()}, m_mat_inv{Mat4x4<T>::identity()} {}

    /**
     * Apply transformation to a 3D Vector
     */
    Vec3<T> apply(const Vec3<T>& v) const {
        return Vec3<T>{m_mat * Vec4<T>{v, 1}};
    }

    /**
     * Invert the transformation applied to a 3D Vector
     */
    Vec3<T> invert(const Vec3<T>& v) const {
        return Vec3<T>{m_mat_inv * Vec4<T>{v, 1}};
    }

    /**
     * Stack a new transformation matrix and it's inverse on top
     */
    Transform3D stack(const Mat4x4<T>& mat, const Mat4x4<T>& inv_mat) {
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
    Transform3D translate(const T delta_x, const T delta_y, const T delta_z) {
        const auto v = Vec4<T>{delta_x, delta_y, delta_z, 1};
        const auto mr = Mat4x4<T>::identity().column_set(3, v);

        const auto vi = Vec4<T>{-delta_x, -delta_y, -delta_z, 1};
        const auto mi = Mat4x4<T>::identity().column_set(3, vi);

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + delta.x, y + delta.y, z + delta.z)
     */
    Transform3D translate(const Vec3<T>& delta) {
        return this->translate(delta.x, delta.y, delta.z);
    }

    // /**
    //  * Stack the transformation equivalent to:
    //  * (x', y', z') = [right | up | forward] * (x, y, z) with:
    //  * - right   = up_dir x forward
    //  * - up      = forward x right
    //  * following right-hand-rule.
    //  *
    //  * @note With OpenGL conventions, camera forward should be -FORWARD by
    //  * default.
    //  */
    // Transform3D rotate(const Vec3<T>& forward_,
    //                    const Vec3<T>& up_dir_,
    //                    bool is_normalized = false) {
    //     const auto up_dir = is_normalized ? up_dir_ : up_dir_.normalized();
    //     const auto forward = is_normalized ? forward_ :
    //     forward_.normalized();
    //
    //     const auto right = cross(up_dir, forward);
    //     const auto up = cross(forward, right);
    //
    //     const auto mr = Mat4x4<T>{Mat3x3<T>::from_rows(right, up, forward),
    //                               Vec4<T>{0, 0, 0, 1}};
    //     const auto mi = mr.transposed();
    //
    //     return this->stack(mr, mi);
    // }
    //
    // /**
    //  * Rotate by `angle_x` radians measured from RIGHT towards FORWARD
    //  */
    // Transform3D rotateX(const T angle_x) {
    //     const auto up_dir_y = std::cos(angle_x);
    //     const auto up_dir_z = std::sin(angle_x);
    //     const auto forward_y = -up_dir_z;
    //     const auto forward_z = up_dir_y;
    //
    //     return this->rotate(Vec3<T>{0, forward_y, forward_z},
    //                         Vec3<T>{0, up_dir_y, up_dir_z}, true);
    // }
    //
    // /**
    //  * Rotate by `angle_y` radians measured from UP towards FORWARD
    //  */
    // Transform3D rotateY(const T angle_y) {
    //     const auto up_dir = UP;
    //     const auto forward_x = std::cos(angle_y);
    //     const auto forward_z = std::sin(angle_y);
    //
    //     return this->rotate(Vec3<T>{forward_x, 0, forward_z}, up_dir,
    //     true);
    // }
    //
    // /**
    //  * Rotate by `angle_z` radians measured from RIGHT towards UP
    //  */
    // Transform3D rotateZ(const T angle_z) {
    //     const auto forward = FORWARD;
    //     const auto up_dir_x = std::cos(angle_z);
    //     const auto up_dir_y = std::sin(angle_z);
    //
    //     return this->rotate(forward, Vec3<T>{up_dir_x, up_dir_y, 0},
    //     true);
    // }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (scale_x * x, scale_y * y, scale_z * z) assuming
     * scale_x * scale_y * scale_z != 0
     */
    Transform3D scale(const T scale_x, const T scale_y, const T scale_z) {
        assert(scale_x != 0);
        assert(scale_y != 0);
        assert(scale_z != 0);

        Mat4x4<T> mr{}, mi{};

        mr(0, 0) = scale_x;
        mr(1, 1) = scale_y;
        mr(2, 2) = scale_z;
        mr(3, 3) = 1;

        mi(0, 0) = 1 / scale_x;
        mi(1, 1) = 1 / scale_y;
        mi(2, 2) = 1 / scale_z;
        mi(3, 3) = 1;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (scale.x * x, scale.y * y, scale.z * z) assuming
     * scale.x * scale.y * scale.z != 0
     */
    Transform3D scale(const Vec3<T>& scale) {
        return this->scale(scale.x, scale.y, scale.z);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + s * z, y + t * z, z)
     */
    Transform3D shearXY(const T s, const T t) {
        const auto a = Vec4<T>{1, 0, s, 0};
        const auto b = Vec4<T>{0, 1, t, 0};
        const auto c = Vec4<T>{0, 0, 1, 0};
        const auto d = Vec4<T>{0, 0, 0, 1};
        const auto mr = Mat4x4<T>::from_rows(a, b, c, d);

        auto mi = Mat4x4<T>{mr};
        mi(0, 2) *= -1;
        mi(1, 2) *= -1;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + s * y, y, z + t * y)
     */
    Transform3D shearXZ(const T s, const T t) {
        const auto a = Vec4<T>{1, s, 0, 0};
        const auto b = Vec4<T>{0, 1, 0, 0};
        const auto c = Vec4<T>{0, t, 1, 0};
        const auto d = Vec4<T>{0, 0, 0, 1};
        const auto mr = Mat4x4<T>::from_rows(a, b, c, d);

        auto mi = Mat4x4<T>{mr};
        mi(0, 1) *= -1;
        mi(2, 1) *= -1;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x, y + s * x, z + t * x)
     */
    Transform3D shearYZ(const T s, const T t) {
        const auto a = Vec4<T>{1, 0, 0, 0};
        const auto b = Vec4<T>{s, 1, 0, 0};
        const auto c = Vec4<T>{t, 0, 1, 0};
        const auto d = Vec4<T>{0, 0, 0, 1};
        const auto mr = Mat4x4<T>::from_rows(a, b, c, d);

        auto mi = Mat4x4<T>{mr};
        mi(1, 0) *= -1;
        mi(2, 0) *= -1;

        return this->stack(mr, mi);
    }
};

}  // namespace asciirast::math
