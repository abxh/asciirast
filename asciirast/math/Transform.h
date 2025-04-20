/**
 * @file Transform.h
 * @brief Class for composing primitive transformation matricies and their inverses
 */

#pragma once

#include <cassert>

#include "./Mat.h"
#include "./Rot.h"

namespace asciirast::math {

/**
 * @brief Class for composing primitive 2D transformation matricies and their inverses
 *
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the matrix is in column major
 */
template<typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Transform2
{
    using Vec2 = Vec<2, T>;
    using Vec3 = Vec<3, T>;
    using Mat2 = Mat<2, 2, T, is_col_major>;
    using Mat3 = Mat<3, 3, T, is_col_major>;
    using Rot2D = Rot2<T, is_col_major>;

    Mat3 m_mat;     ///< underlying matrix
    Mat3 m_mat_inv; ///< underlying inverse matrix

    constexpr Transform2(const Mat3& mat, const Mat3& mat_inv)
            : m_mat{ mat }
            , m_mat_inv{ mat_inv } {};

protected:
    /**
     * @brief Stack a new transformation matrix and it's inverse on top of this
     *
     * @param mat The transformation matrix
     * @param inv_mat The inverse transformation matrix
     * @return This
     */
    constexpr Transform2& stack(const Mat3& mat, const Mat3& inv_mat)
    {
        m_mat = mat * m_mat;
        m_mat_inv = m_mat_inv * inv_mat;
        return *this;
    }

public:
    /**
     * @brief Construct a identity transform object that performs "nothing"
     */
    constexpr Transform2()
            : m_mat{ Mat3::identity() }
            , m_mat_inv{ Mat3::identity() } {};

    /**
     * @brief Get underlying matrix
     *
     * @return Const reference to the underlying matrix
     */
    constexpr const Mat3& mat() const { return m_mat; }

    /**
     * @brief Get underlying inverse matrix
     *
     * @return Const reference to the underlying inverse matrix
     */
    constexpr const Mat3& mat_inv() const { return m_mat_inv; }

    /**
     * @brief Apply the transformation "action" on a 2D vector
     *
     * @param v The vector at hand
     * @return A copy of the vector transformed
     */
    [[nodiscard]] constexpr Vec2 apply(const Vec2& v) const { return Vec2{ m_mat * Vec3{ v, 1 } }; }

    /**
     * @brief Apply the inverse transformation "action" on a 2D vector
     *
     * @param v The vector at hand
     * @return A copy of the vector transformed back
     */
    [[nodiscard]] constexpr Vec2 apply_inv(const Vec2& v) const { return Vec2{ m_mat_inv * Vec3{ v, 1 } }; }

    /**
     * @brief Get transformation object that performs the reverse transformation
     *
     * @return Copy of this that performs the reverse transfromation
     */
    [[nodiscard]] constexpr Transform2 reversed() const { return Transform2{ this->m_mat_inv, this->m_mat }; }

    /**
     * @brief Stack another Transform on top of this
     *
     * @param that The other Transform
     * @return This
     */
    constexpr Transform2& stack(const Transform2& that) { return this->stack(that.m_mat, that.m_mat_inv); }

    /**
     * @brief Stack (x', y') = (x + delta_x, y + delta_y)
     *
     * @param delta_x How much to translate in x-axis
     * @param delta_y How much to translate in y-axis
     * @return This
     */
    constexpr Transform2& translate(const T delta_x, const T delta_y)
    {
        const Vec3 vr = { +delta_x, +delta_y, 1 };
        const Vec3 vi = { -delta_x, -delta_y, 1 };

        const auto mr = Mat3{ Mat2::identity(), vr };
        const auto mi = Mat3{ Mat2::identity(), vi };

        return this->stack(mr, mi);
    }

    /**
     * @brief Stack (x', y') = (x + delta.x, y + delta.y)
     *
     * @param delta How much to translate as a Vec
     * @return This
     */
    constexpr Transform2& translate(const Vec2& delta) { return this->translate(delta.x, delta.y); }

    /**
     * @brief Stack a rotation transformation
     *
     * @param rot The rotation object
     * @return This
     */
    Transform2& rotate(const Rot2D& rot)
    {
        const Mat3 mr = { rot.to_mat(), Vec3{ 0, 0, 1 } };
        const Mat3 mi = mr.transposed();

        return this->stack(mr, mi);
    }

    /**
     * @brief Stack (x', y') = (scale_x * x, scale_y * y), assuming scale_x * scale_y != 0
     *
     * @param scale_x How much to scale x
     * @param scale_y How much to scale y
     * @return This
     */
    constexpr Transform2& scale(const T scale_x, const T scale_y)
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
     * @brief Stack (x', y') = (scale.x * x, scale.y * y), assuming scale.x * scale.y != 0
     *
     * @param scale How much to scale as a Vec
     * @return This
     */
    constexpr Transform2& scale(const Vec2& scale) { return this->scale(scale.x, scale.y); }

    /**
     * @brief Stack (x', y') = (-x, y)
     *
     * @return This
     */
    constexpr Transform2& reflectX() { return this->scale(-1, 1); }

    /**
     * @brief Stack (x', y') = (x, -y)
     *
     * @return This
     */
    constexpr Transform2& reflectY() { return this->scale(1, -1); }

    /**
     * @brief Stack (x', y') = (x + t * y, y)
     *
     * @param t How much y should influence x in the direction of x
     * @return This
     */
    constexpr Transform2& shearX(const T t)
    {
        const Vec3 ar = { 1, +t, 0 };
        const Vec3 br = { 0, 1., 0 };
        const Vec3 cr = { 0, 0., 1 };
        const auto mr = Mat3::from_rows(ar, br, cr);

        const Vec3 ai = { 1, -t, 0 };
        const Vec3 bi = { 0, 1., 0 };
        const Vec3 ci = { 0, 0., 1 };
        const auto mi = Mat3::from_rows(ai, bi, ci);

        return this->stack(mr, mi);
    }

    /**
     * @brief Stack (x', y') = (x, y + t * x)
     *
     * @param t How much x should influence y in the direction of y
     * @return This
     */
    constexpr Transform2& shearY(const T t)
    {
        const Vec3 ar = { 1., 0, 0 };
        const Vec3 br = { +t, 1, 0 };
        const Vec3 cr = { 0., 0, 1 };
        const auto mr = Mat3::from_rows(ar, br, cr);

        const Vec3 ai = { 1., 0, 0 };
        const Vec3 bi = { -t, 1, 0 };
        const Vec3 ci = { 0., 0, 1 };
        const auto mi = Mat3::from_rows(ai, bi, ci);

        return this->stack(mr, mi);
    }
};

/**
 * @brief Class for composing primitive 3D transformation matricies and their inverses
 *
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the matrix is in column major
 */
template<typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Transform3
{
    using Vec3 = Vec<3, T>;
    using Vec4 = Vec<4, T>;
    using Mat3 = Mat<3, 3, T, is_col_major>;
    using Mat4 = Mat<4, 4, T, is_col_major>;
    using Rot3D = Rot3<T, is_col_major>;

    Mat4 m_mat;     ///< underlying matrix
    Mat4 m_mat_inv; ///< underlying inverse matrix

    constexpr Transform3(const Mat4& mat, const Mat4& mat_inv)
            : m_mat{ mat }
            , m_mat_inv{ mat_inv } {};

protected:
    /**
     * @brief Stack a new transformation matrix and it's inverse on top of this
     *
     * @param mat The transformation matrix
     * @param inv_mat The inverse transformation matrix
     * @return This
     */
    constexpr Transform3& stack(const Mat4& mat, const Mat4& inv_mat)
    {
        m_mat = mat * m_mat;
        m_mat_inv = m_mat_inv * inv_mat;
        return *this;
    }

public:
    /**
     * @brief Construct a identity transform object that performs "nothing"
     */
    constexpr Transform3()
            : m_mat{ Mat4::identity() }
            , m_mat_inv{ Mat4::identity() } {};

    /**
     * @brief Get underlying transformation matrix
     *
     * @return Const reference to the underlying matrix
     */
    [[nodiscard]] constexpr const Mat4& mat() const { return m_mat; }

    /**
     * @brief Get underlying inverse transformation matrix
     *
     * @return Const reference to the underlying inverse matrix
     */
    [[nodiscard]] constexpr const Mat4& mat_inv() const { return m_mat_inv; }

    /**
     * @brief Apply the transformation "action" on a 3D vector
     *
     * @param v The vector at hand
     * @return A copy of the vector transformed
     */
    [[nodiscard]] constexpr Vec3 apply(const Vec3& v) const { return Vec3{ m_mat * Vec4{ v, 1 } }; }

    /**
     * @brief Apply the inverse transformation "action" on a 3D vector
     *
     * @param v The vector at hand
     * @return A copy of the vector transformed back
     */
    [[nodiscard]] constexpr Vec3 apply_inv(const Vec3& v) const { return Vec3{ m_mat_inv * Vec4{ v, 1 } }; }

    /**
     * @brief Get transformation object that performs the reverse transformation
     *
     * @return Copy of this that performs the reverse transfromation
     */
    [[nodiscard]] constexpr Transform3 reversed() const { return Transform3{ this->m_mat_inv, this->m_mat }; }

    /**
     * @brief Stack another Transform on top of this
     *
     * @param that The other Transform
     * @return This
     */
    constexpr Transform3& stack(const Transform3& that) { return this->stack(that.m_mat, that.m_mat_inv); }

    /**
     * @brief Stack (x', y', z') = (x + delta_x, y + delta_y, z + delta_z)
     *
     * @param delta_x How much to move in x-axis
     * @param delta_y How much to move in y-axis
     * @param delta_z How much to move in z-axis
     * @return This
     */
    constexpr Transform3& translate(const T delta_x, const T delta_y, const T delta_z)
    {
        const Vec4 vr = { +delta_x, +delta_y, +delta_z, 1 };
        const auto mr = Mat4{ Mat3::identity(), vr };

        const Vec4 vi = { -delta_x, -delta_y, -delta_z, 1 };
        const auto mi = Mat4{ Mat3::identity(), vi };

        return this->stack(mr, mi);
    }

    /**
     * @brief Stack (x', y', z') = (x + delta.x, y + delta.y, z + delta.z)
     *
     * @param delta How much to move as a Vec
     * @return This
     */
    constexpr Transform3& translate(const Vec3& delta) { return this->translate(delta.x, delta.y, delta.z); }

    /**
     * @brief Stack a rotation transformation
     *
     * @note A rotation object should be stored seperately,
     * to avoid gimbal lock. When all rotations are stacked
     * on it, then it should be added onto the Transform objects.
     *
     * @param rot The rotation object
     * @return This
     */
    Transform3& rotate(const Rot3D& rot)
    {
        const Mat4 mr = { rot.to_mat(), Vec4{ 0, 0, 0, 1 } };
        const Mat4 mi = mr.transposed();

        return this->stack(mr, mi);
    }

    /**
     * @brief Stack (x', y', z') = (scale_x * x, scale_y * y, scale_z * z),
     *        assuming scale_x * scale_y * scale_z != 0
     *
     * @param scale_x How much to scale x
     * @param scale_y How much to scale y
     * @param scale_z How much to scale z
     * @return This
     */
    constexpr Transform3& scale(const T scale_x, const T scale_y, const T scale_z)
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
     * @brief Stack (x', y', z') = (scale.x * x, scale.y * y, scale.z * z),
     *        assuming scale.x * scale.y * scale.z != 0
     *
     * @param scale How much to scale as a Vec
     * @return This
     */
    constexpr Transform3& scale(const Vec3& scale) { return this->scale(scale.x, scale.y, scale.z); }

    /**
     * @brief Stack (x', y', z') = (-x, y, z)
     *
     * @return This
     */
    constexpr Transform3& reflectX() { return this->scale(-1, 1, 1); }

    /**
     * @brief Stack (x', y', z') = (x, -y, z)
     *
     * @return This
     */
    constexpr Transform3& reflectY() { return this->scale(1, -1, 1); }

    /**
     * @brief Stack (x', y', z') = (x, y, -z)
     *
     * @return This
     */
    constexpr Transform3& reflectZ() { return this->scale(1, 1, -1); }

    /**
     * @brief Stack (x', y', z') = (x + s * z, y + t * z, z)
     *
     * @param s How much z should influence x in the direction of x
     * @param t How much z should influence y in the direction of y
     * @return This
     */
    constexpr Transform3& shearXY(const T s, const T t)
    {
        const Vec4 ar = { 1, 0, +s, 0 };
        const Vec4 br = { 0, 1, +t, 0 };
        const Vec4 cr = { 0, 0, 1., 0 };
        const Vec4 dr = { 0, 0, 0., 1 };
        const auto mr = Mat4::from_rows(ar, br, cr, dr);

        const Vec4 ai = { 1, 0, -s, 0 };
        const Vec4 bi = { 0, 1, -t, 0 };
        const Vec4 ci = { 0, 0, 1., 0 };
        const Vec4 di = { 0, 0, 0., 1 };
        const auto mi = Mat4::from_rows(ai, bi, ci, di);

        return this->stack(mr, mi);
    }

    /**
     * @brief Stack (x', y', z') = (x + s * y, y, z + t * y)
     *
     * @param s How much y should influence x in the direction of x
     * @param t How much y should influence z in the direction of z
     * @return This
     */
    constexpr Transform3& shearXZ(const T s, const T t)
    {
        const Vec4 ar = { 1, +s, 0, 0 };
        const Vec4 br = { 0, 1., 0, 0 };
        const Vec4 cr = { 0, +t, 1, 0 };
        const Vec4 dr = { 0, 0., 0, 1 };
        const auto mr = Mat4::from_rows(ar, br, cr, dr);

        const Vec4 ai = { 1, -s, 0, 0 };
        const Vec4 bi = { 0, 1., 0, 0 };
        const Vec4 ci = { 0, -t, 1, 0 };
        const Vec4 di = { 0, 0., 0, 1 };
        const auto mi = Mat4::from_rows(ai, bi, ci, di);

        return this->stack(mr, mi);
    }

    /**
     * @brief Stack (x', y', z') = (x, y + s * x, z + t * x)
     *
     * @param s How much x should influence y in the direction of y
     * @param t How much x should influence z in the direction of z
     * @return This
     */
    constexpr Transform3& shearYZ(const T s, const T t)
    {
        const Vec4 ar = { 1., 0, 0, 0 };
        const Vec4 br = { +s, 1, 0, 0 };
        const Vec4 cr = { +t, 0, 1, 0 };
        const Vec4 dr = { 0., 0, 0, 1 };
        const auto mr = Mat4::from_rows(ar, br, cr, dr);

        const Vec4 ai = { 1., 0, 0, 0 };
        const Vec4 bi = { -s, 1, 0, 0 };
        const Vec4 ci = { -t, 0, 1, 0 };
        const Vec4 di = { 0., 0, 0, 1 };
        const auto mi = Mat4::from_rows(ai, bi, ci, di);

        return this->stack(mr, mi);
    }
};

} // namespace asciirast::math
