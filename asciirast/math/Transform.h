/**
 * @file Transform.h
 * @brief Class for stacking primitive transformations on top of each
 * other
 */

#pragma once

#include <cassert>

#include "./Mat.h"
#include "./Rot.h"

namespace asciirast::math {

/**
 * @brief Transformation abstraction
 *
 * @tparam N             Number of dimensions being operated on.
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the matrix is in column major
 */
template<std::size_t N, typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Transform;

/**
 * @brief 2D transformation abstraction
 *
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the matrix is in column major
 */
template<typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Transform<2, T, is_col_major>
{
    using Vec2 = Vec<2, T>;
    using Vec3 = Vec<3, T>;
    using Mat2 = Mat<2, 2, T, is_col_major>;
    using Mat3 = Mat<3, 3, T, is_col_major>;
    using Rot2 = Rot<2, T, is_col_major>;

    Mat3 m_mat;     ///< underlying matrix
    Mat3 m_mat_inv; ///< underlying inverse matrix

public:
    /**
     * Create a new transform object
     */
    Transform()
            : m_mat{ Mat3::identity() }
            , m_mat_inv{ Mat3::identity() } {};

    /**
     * Get underlying matrix
     */
    const Mat3& mat() const { return m_mat; }

    /**
     * Get underlying inverse matrix
     */
    const Mat3& mat_inv() const { return m_mat_inv; }

    /**
     * Apply transformation to a 2D Vector
     */
    Vec2 apply(const Vec2& v) const { return Vec2{ m_mat * Vec3{ v, 1 } }; }

    /**
     * Invert the transformation applied to a 2D Vector
     */
    Vec2 invert(const Vec2& v) const { return Vec2{ m_mat_inv * Vec3{ v, 1 } }; }

    /**
     * Get reversed transformation object
     */
    Transform reversed() const { return Transform().stack(this->m_mat_inv, this->m_mat); }

    /**
     * Stack a new transformation matrix and it's inverse on top
     */
    Transform& stack(const Mat3& mat, const Mat3& inv_mat)
    {
        m_mat     = mat * m_mat;
        m_mat_inv = m_mat_inv * inv_mat;
        return *this;
    }

    /**
     * Stack another Transform2D on top of this
     */
    Transform& stack(const Transform& that) { return this->stack(that.m_mat, that.m_mat_inv); }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + delta_x, y + delta_y)
     */
    Transform& translate(const T delta_x, const T delta_y)
    {
        const Vec3 vr = { +delta_x, +delta_y, 1 };
        const Vec3 vi = { -delta_x, -delta_y, 1 };

        const auto mr = Mat3{ Mat2::identity(), vr };
        const auto mi = Mat3{ Mat2::identity(), vi };

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x + delta.x, y + delta.y)
     */
    Transform& translate(const Vec2& delta) { return this->translate(delta.x, delta.y); }

    /**
     * Stack a rotation transformation
     */
    Transform& rotate(const Rot2& rot)
    {
        const Mat3 mr = { rot.to_mat(), Vec3{ 0, 0, 1 } };
        const Mat3 mi = mr.transposed();

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (scale_x * x, scale_y * y)
     *
     * assuming scale_x * scale_y != 0
     */
    Transform& scale(const T scale_x, const T scale_y)
    {
        assert(scale_x != 0);
        [[assume(scale_x != 0)]];

        assert(scale_y != 0);
        [[assume(scale_y != 0)]];

        Mat3 mr{}, mi{};

        mr[0, 0] = scale_x;
        mr[1, 1] = scale_y;
        mr[2, 2] = 1;

        mi[0, 0] = 1 / scale_x;
        mi[1, 1] = 1 / scale_y;
        mi[2, 2] = 1;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (scale.x * x, scale.y * y)
     *
     * assuming scale.x * scale.y != 0
     */
    Transform& scale(const Vec2& scale) { return this->scale(scale.x, scale.y); }

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
        const Vec3 a  = { 1, +t, 0 };
        const Vec3 b  = { 0, 1., 0 };
        const Vec3 c  = { 0, 0., 1 };
        const auto mr = Mat3::from_rows(a, b, c);

        const Vec3 ai = { 1, -t, 0 };
        const Vec3 bi = { 0, 1, 0 };
        const Vec3 ci = { 0, 0, 1 };
        const auto mi = Mat3::from_rows(ai, bi, ci);

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y') = (x, y + t * x)
     */
    Transform& shearY(const T t)
    {
        const Vec3 a  = { 1., 0, 0 };
        const Vec3 b  = { +t, 1, 0 };
        const Vec3 c  = { 0., 0, 1 };
        const auto mr = Mat3::from_rows(a, b, c);

        const Vec3 ai = { 1., 0, 0 };
        const Vec3 bi = { -t, 1, 0 };
        const Vec3 ci = { 0., 0, 1 };
        const auto mi = Mat3::from_rows(ai, bi, ci);

        return this->stack(mr, mi);
    }
};

/**
 * @brief 3D transformation abstraction
 *
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the matrix is in column major
 */
template<typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Transform<3, T, is_col_major>
{
    using Vec3 = Vec<3, T>;
    using Vec4 = Vec<4, T>;
    using Mat3 = Mat<3, 3, T, is_col_major>;
    using Mat4 = Mat<4, 4, T, is_col_major>;
    using Rot3 = Rot<3, T, is_col_major>;

    Mat4 m_mat;     ///< underlying matrix
    Mat4 m_mat_inv; ///< underlying inverse matrix

public:
    /**
     * Create a new transform object
     */
    Transform()
            : m_mat{ Mat4::identity() }
            , m_mat_inv{ Mat4::identity() }
    {
    }

    /**
     * Get underlying matrix
     */
    const Mat4& mat() const { return m_mat; }

    /**
     * Get underlying inverse matrix
     */
    const Mat4& mat_inv() const { return m_mat_inv; }

    /**
     * Apply transformation to a 3D Vector
     */
    Vec3 apply(const Vec3& v) const { return Vec3{ m_mat * Vec4{ v, 1 } }; }

    /**
     * Invert the transformation applied to a 3D Vector
     */
    Vec3 invert(const Vec3& v) const { return Vec3{ m_mat_inv * Vec4{ v, 1 } }; }

    /**
     * Get reversed transformation object
     */
    Transform reversed() const { return Transform{}.stack(this->m_mat_inv, this->m_mat); }

    /**
     * Stack a new transformation matrix and it's inverse on top
     */
    Transform& stack(const Mat4& mat, const Mat4& inv_mat)
    {
        m_mat     = mat * m_mat;
        m_mat_inv = m_mat_inv * inv_mat;
        return *this;
    }

    /**
     * Stack another Transform on top of this
     */
    Transform& stack(const Transform& that) { return this->stack(that.m_mat, that.m_mat_inv); }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + delta_x, y + delta_y, z + delta_z)
     */
    Transform& translate(const T delta_x, const T delta_y, const T delta_z)
    {
        const Vec4 v  = { +delta_x, +delta_y, +delta_z, 1 };
        const auto mr = Mat4{ Mat3::identity(), v };

        const Vec4 vi = { -delta_x, -delta_y, -delta_z, 1 };
        const auto mi = Mat4{ Mat3::identity(), vi };

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + delta.x, y + delta.y, z + delta.z)
     */
    Transform& translate(const Vec3& delta) { return this->translate(delta.x, delta.y, delta.z); }

    /**
     * Stack a rotation transformation
     */
    Transform& rotate(const Rot3& rot)
    {
        const Mat4 mr = { rot.to_mat(), Vec4{ 0, 0, 0, 1 } };
        const Mat4 mi = mr.transposed();

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (scale_x * x, scale_y * y, scale_z * z),
     *
     * @note assuming scale_x * scale_y * scale_z != 0
     */
    Transform& scale(const T scale_x, const T scale_y, const T scale_z)
    {
        assert(scale_x != 0);
        [[assume(scale_x != 0)]];

        assert(scale_y != 0);
        [[assume(scale_y != 0)]];

        assert(scale_z != 0);
        [[assume(scale_z != 0)]];

        Mat4 mr{}, mi{};

        mr[0, 0] = scale_x;
        mr[1, 1] = scale_y;
        mr[2, 2] = scale_z;
        mr[3, 3] = 1;

        mi[0, 0] = 1 / scale_x;
        mi[1, 1] = 1 / scale_y;
        mi[2, 2] = 1 / scale_z;
        mi[3, 3] = 1;

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (scale.x * x, scale.y * y, scale.z * z),
     *
     * @note assuming scale.x * scale.y * scale.z != 0
     */
    Transform& scale(const Vec3& scale) { return this->scale(scale.x, scale.y, scale.z); }

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
        const Vec4 a  = { 1, 0, +s, 0 };
        const Vec4 b  = { 0, 1, +t, 0 };
        const Vec4 c  = { 0, 0, 1., 0 };
        const Vec4 d  = { 0, 0, 0., 1 };
        const auto mr = Mat4::from_rows(a, b, c, d);

        const Vec4 ai = { 1, 0, -s, 0 };
        const Vec4 bi = { 0, 1, -t, 0 };
        const Vec4 ci = { 0, 0, 1., 0 };
        const Vec4 di = { 0, 0, 0., 1 };
        const auto mi = Mat4::from_rows(ai, bi, ci, di);

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x + s * y, y, z + t * y)
     */
    Transform& shearXZ(const T s, const T t)
    {
        const Vec4 a  = { 1, +s, 0, 0 };
        const Vec4 b  = { 0, 1., 0, 0 };
        const Vec4 c  = { 0, +t, 1, 0 };
        const Vec4 d  = { 0, 0., 0, 1 };
        const auto mr = Mat4::from_rows(a, b, c, d);

        const Vec4 ai = { 1, -s, 0, 0 };
        const Vec4 bi = { 0, 1., 0, 0 };
        const Vec4 ci = { 0, -t, 1, 0 };
        const Vec4 di = { 0, 0., 0, 1 };
        const auto mi = Mat4::from_rows(ai, bi, ci, di);

        return this->stack(mr, mi);
    }

    /**
     * Stack the transformation equivalent to:
     * (x', y', z') = (x, y + s * x, z + t * x)
     */
    Transform& shearYZ(const T s, const T t)
    {
        const Vec4 a  = { 1., 0, 0, 0 };
        const Vec4 b  = { +s, 1, 0, 0 };
        const Vec4 c  = { +t, 0, 1, 0 };
        const Vec4 d  = { 0., 0, 0, 1 };
        const auto mr = Mat4::from_rows(a, b, c, d);

        const Vec4 ai = { 1., 0, 0, 0 };
        const Vec4 bi = { -s, 1, 0, 0 };
        const Vec4 ci = { -t, 0, 1, 0 };
        const Vec4 di = { 0., 0, 0, 1 };
        const auto mi = Mat4::from_rows(ai, bi, ci, di);

        return this->stack(mr, mi);
    }
};

} // namespace asciirast::math
