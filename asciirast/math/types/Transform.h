/**
 * @file Transform.h
 * @brief Class for stacking primitive transformations on top of each other
 */

#pragma once

#include <cassert>

#include "Mat.h"

namespace asciirast::math {

/**
 * @brief Transformation abstraction
 *
 * @tparam N                    Number of dimensions being operated on.
 * @tparam T                    Type of elements
 * @tparam mat_is_column_major  Whether the matrix is in column major
 */
template<std::size_t N, typename T, bool mat_is_column_major>
    requires(std::is_floating_point_v<T>)
class Transform;

/**
 * @brief 2D transformation abstraction
 *
 * @tparam T                    Type of elements
 * @tparam mat_is_column_major  Whether the matrix is in column major
 */
template<typename T, bool mat_is_column_major>
    requires(std::is_floating_point_v<T>)
class Transform<2, T, mat_is_column_major>
{
public:
    using Vec2 = Vec<2, T>;
    using Vec3 = Vec<3, T>;
    using Mat3 = Mat<3, 3, T, mat_is_column_major>;

    Mat3 m_mat;     ///< underlying matrix
    Mat3 m_mat_inv; ///< underlying inverse matrix

    /**
     * Create a new transform object
     */
    Transform()
            : m_mat{ Mat3::identity() }
            , m_mat_inv{ Mat3::identity() }
    {
    }

    /**
     * Apply transformation to a 2D Vector
     */
    Vec2 apply(const Vec2& v) const { return Vec2{ m_mat * Vec3{ v, 1 } }; }

    /**
     * Invert the transformation applied to a 2D Vector
     */
    Vec2 invert(const Vec2& v) const
    {
        return Vec2{ m_mat_inv * Vec3{ v, 1 } };
    }

    /**
     * Stack a new transformation matrix and it's inverse on top
     */
    Transform& stack(const Mat3& mat, const Mat3& inv_mat)
    {
        m_mat = mat * m_mat;
        m_mat_inv = m_mat_inv * inv_mat;
        return *this;
    }

    /**
     * Stack another Transform2D on top of this
     */
    Transform& stack(const Transform& that)
    {
        return this->stack(that.m_mat, that.m_mat_inv);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + delta_x, y + delta_y)
     */
    Transform& translate(const T delta_x, const T delta_y)
    {
        const auto vr = Vec3{ delta_x, delta_y, 1 };
        const auto vi = Vec3{ -delta_x, -delta_y, 1 };

        const auto mr = Mat3::identity().column_set(2, vr);
        const auto mi = Mat3::identity().column_set(2, vi);

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + delta.x, y + delta.y)
     */
    Transform& translate(const Vec2& delta)
    {
        return this->translate(delta.x, delta.y);
    }

    /**
     * Stack the transformation which aligns the x-axis with v
     */
    Transform& rotate(const Vec2& v, bool is_normalized = false)
    {
        const auto x_basis = is_normalized ? v : v.normalized();
        const auto y_basis = complex_prod(x_basis, Vec2{ 0, 1 });
        const auto z_basis = Vec3{ 0, 0, 1 };

        const Mat3 mr{ x_basis, y_basis, z_basis };
        const Mat3 mi = mr.transposed();

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation which performs a rotation of some radians in
     * counterclockwise direction
     */
    Transform& rotate_counterclockwise(const T radians)
    {
        return this->rotate(Vec2::rotor(radians), true);
    }

    /**
     * Stack the transformation which performs a rotation of some radians in
     * clockwise direction
     */
    Transform& rotate_clockwise(const T radians)
    {
        return this->rotate(Vec2::rotor(-radians), true);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (scale_x * x, scale_y * y) assuming scale_x * scale_y != 0
     */
    Transform& scale(const T scale_x, const T scale_y)
    {
        assert(scale_x != 0);
        assert(scale_y != 0);

        Mat3 mr{}, mi{};

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
    Transform& scale(const Vec2& scale)
    {
        return this->scale(scale.x, scale.y);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (-x, y)
     */
    Transform& reflectX() { return this->scale(-1, 1); }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x, -y)
     */
    Transform& reflectY() { return this->scale(1, -1); }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + t * y, y)
     */
    Transform& shearX(const T t)
    {
        const auto a = Vec3{ 1, t, 0 };
        const auto b = Vec3{ 0, 1, 0 };
        const auto c = Vec3{ 0, 0, 1 };
        const auto mr = Mat3::from_rows(a, b, c);

        auto mi = Mat3{ mr };
        mi(0, 1) *= -1;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x, y + t * x)
     */
    Transform& shearY(const T t)
    {
        const auto a = Vec3{ 1, 0, 0 };
        const auto b = Vec3{ t, 1, 0 };
        const auto c = Vec3{ 0, 0, 1 };
        const auto mr = Mat3::from_rows(a, b, c);

        auto mi = Mat3{ mr };
        mi(1, 0) *= -1;

        return this->stack(mr, mi);
    }
};

/**
 * @brief 3D transformation abstraction
 *
 * @tparam T                    Type of elements
 * @tparam mat_is_column_major  Whether the matrix is in column major
 */
template<typename T, bool mat_is_column_major>
    requires(std::is_floating_point_v<T>)
class Transform<3, T, mat_is_column_major>
{
private:
    using Vec3 = Vec<3, T>;
    using Vec4 = Vec<4, T>;
    using Mat4 = Mat<4, 4, T, mat_is_column_major>;

    Mat4 m_mat;     ///< underlying matrix
    Mat4 m_mat_inv; ///< underlying inverse matrix

    /**
     * Create a new transform object
     */
    Transform()
            : m_mat{ Mat4::identity() }
            , m_mat_inv{ Mat4::identity() }
    {
    }

    /**
     * Apply transformation to a 3D Vector
     */
    Vec3 apply(const Vec3& v) const { return Vec3{ m_mat * Vec4{ v, 1 } }; }

    /**
     * Invert the transformation applied to a 3D Vector
     */
    Vec3 invert(const Vec3& v) const
    {
        return Vec3{ m_mat_inv * Vec4{ v, 1 } };
    }

    /**
     * Stack a new transformation matrix and it's inverse on top
     */
    Transform& stack(const Mat4& mat, const Mat4& inv_mat)
    {
        m_mat = mat * m_mat;
        m_mat_inv = m_mat_inv * inv_mat;
        return *this;
    }

    /**
     * Stack another Transform on top of this
     */
    Transform& stack(const Transform& that)
    {
        return this->stack(that.m_mat, that.m_mat_inv);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + delta_x, y + delta_y, z + delta_z)
     */
    Transform& translate(const T delta_x, const T delta_y, const T delta_z)
    {
        const auto v = Vec4{ delta_x, delta_y, delta_z, 1 };
        const auto mr = Mat4::identity().column_set(3, v);

        const auto vi = Vec4{ -delta_x, -delta_y, -delta_z, 1 };
        const auto mi = Mat4::identity().column_set(3, vi);

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + delta.x, y + delta.y, z + delta.z)
     */
    Transform& translate(const Vec3& delta)
    {
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
    // Transform rotate(const vec3& forward_,
    //                    const vec3& up_dir_,
    //                    bool is_normalized = false) {
    //     const auto up_dir = is_normalized ? up_dir_ : up_dir_.normalized();
    //     const auto forward = is_normalized ? forward_ :
    //     forward_.normalized();
    //
    //     const auto right = cross(up_dir, forward);
    //     const auto up = cross(forward, right);
    //
    //     const auto mr = mat4{mat3::from_rows(right, up, forward),
    //                               vec4{0, 0, 0, 1}};
    //     const auto mi = mr.transposed();
    //
    //     return this->stack(mr, mi);
    // }
    //
    // /**
    //  * Rotate by `angle_x` radians measured from RIGHT towards FORWARD
    //  */
    // Transform rotateX(const T angle_x) {
    //     const auto up_dir_y = std::cos(angle_x);
    //     const auto up_dir_z = std::sin(angle_x);
    //     const auto forward_y = -up_dir_z;
    //     const auto forward_z = up_dir_y;
    //
    //     return this->rotate(vec3{0, forward_y, forward_z},
    //                         vec3{0, up_dir_y, up_dir_z}, true);
    // }
    //
    // /**
    //  * Rotate by `angle_y` radians measured from UP towards FORWARD
    //  */
    // Transform& rotateY(const T angle_y) {
    //     const auto up_dir = UP;
    //     const auto forward_x = std::cos(angle_y);
    //     const auto forward_z = std::sin(angle_y);
    //
    //     return this->rotate(vec3{forward_x, 0, forward_z}, up_dir,
    //     true);
    // }
    //
    // /**
    //  * Rotate by `angle_z` radians measured from RIGHT towards UP
    //  */
    // Transform& rotateZ(const T angle_z) {
    //     const auto forward = FORWARD;
    //     const auto up_dir_x = std::cos(angle_z);
    //     const auto up_dir_y = std::sin(angle_z);
    //
    //     return this->rotate(forward, vec3{up_dir_x, up_dir_y, 0},
    //     true);
    // }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (scale_x * x, scale_y * y, scale_z * z) assuming
     * scale_x * scale_y * scale_z != 0
     */
    Transform& scale(const T scale_x, const T scale_y, const T scale_z)
    {
        assert(scale_x != 0);
        assert(scale_y != 0);
        assert(scale_z != 0);

        Mat4 mr{}, mi{};

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
    Transform& scale(const Vec3& scale)
    {
        return this->scale(scale.x, scale.y, scale.z);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (-x, y, z)
     */
    Transform& reflectX() { return this->scale(-1, 1, 1); }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x, -y, z)
     */
    Transform& reflectY() { return this->scale(1, -1, 1); }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x, y, -z)
     */
    Transform& reflectZ() { return this->scale(1, 1, -1); }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + s * z, y + t * z, z)
     */
    Transform& shearXY(const T s, const T t)
    {
        const auto a = Vec4{ 1, 0, s, 0 };
        const auto b = Vec4{ 0, 1, t, 0 };
        const auto c = Vec4{ 0, 0, 1, 0 };
        const auto d = Vec4{ 0, 0, 0, 1 };
        const auto mr = Mat4::from_rows(a, b, c, d);

        auto mi = Mat4{ mr };
        mi(0, 2) *= -1;
        mi(1, 2) *= -1;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + s * y, y, z + t * y)
     */
    Transform& shearXZ(const T s, const T t)
    {
        const auto a = Vec4{ 1, s, 0, 0 };
        const auto b = Vec4{ 0, 1, 0, 0 };
        const auto c = Vec4{ 0, t, 1, 0 };
        const auto d = Vec4{ 0, 0, 0, 1 };
        const auto mr = Mat4::from_rows(a, b, c, d);

        auto mi = Mat4{ mr };
        mi(0, 1) *= -1;
        mi(2, 1) *= -1;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x, y + s * x, z + t * x)
     */
    Transform& shearYZ(const T s, const T t)
    {
        const auto a = Vec4{ 1, 0, 0, 0 };
        const auto b = Vec4{ s, 1, 0, 0 };
        const auto c = Vec4{ t, 0, 1, 0 };
        const auto d = Vec4{ 0, 0, 0, 1 };
        const auto mr = Mat4::from_rows(a, b, c, d);

        auto mi = Mat4{ mr };
        mi(1, 0) *= -1;
        mi(2, 0) *= -1;

        return this->stack(mr, mi);
    }
};

} // namespace asciirast::math
