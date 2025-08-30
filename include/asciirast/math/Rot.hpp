/**
 * @file Rot.hpp
 * @brief Classes for rotation
 *
 * @todo spherical interpolation
 *
 * Quaternion implementation initially based on following:
 * - https://mathworld.wolfram.com/Quaternion.html
 * - https://marctenbosch.com/quaternions/
 */

#pragma once

#include <cmath>
#include <type_traits>

#include "./Mat.hpp"
#include "./Vec.hpp"

namespace asciirast::math {

/**
 * @brief Class for composing 2D rotations
 *
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the produced matrix is in column major
 */
template<typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Rot2DType
{
public:
    using Vec2 = Vec<2, T>;
    using Mat2 = Mat<2, 2, T, is_col_major>;
    using Rot2D = Rot2DType<T, is_col_major>;

    /**
     * @brief Construct identity rotation object that does "nothing"
     */
    constexpr Rot2DType() = default;

    /**
     * @brief (Implicitly) construct rotation object from angle
     *
     * @param radians The angle at hand in radians
     */
    constexpr Rot2DType(const T radians)
        : m_complex{ complex_from(radians) } {};

    /**
     * @brief Construct rotation object from the angle between two
     * vectors.
     *
     * @param from_dir The source direction vector
     * @param to_dir The destination direction vector
     * @param normalize Whether to_dir is not normalized
     */
    constexpr Rot2DType(const Vec2& from_dir, const Vec2& to_dir, bool normalize = true)
        : m_complex{ complex_div(to_dir, from_dir, normalize) } {};

    /**
     * @brief Construct rotation object from complex number
     *
     * @param complex The complex number at hand
     */
    explicit constexpr Rot2DType(const Vec2& complex)
        : m_complex{ complex } {};

    /**
     * @brief Normalize underlying complex number in-place
     *
     * @return This
     */
    constexpr Rot2D& normalize()
    {
        m_complex = std::move(m_complex.normalized());
        return (*this);
    }

    /**
     * @brief Convert to angle in radians
     */
    [[nodiscard]] constexpr T to_angle() { return atan2(m_complex.y, m_complex.x); }

    /**
     * @brief Convert to complex number
     */
    [[nodiscard]] constexpr Vec2 to_complex() const { return m_complex; }

    /**
     * @brief Convert to 2D transformation matrix
     *
     * @return The tranformation matrix that performs the same rotation as
     * this
     */
    [[nodiscard]] constexpr Mat2 to_mat() const
    {
        const Vec2 x_hat = apply({ 1, 0 });
        const Vec2 y_hat = apply({ 0, 1 });

        return Mat2::from_cols(x_hat, y_hat);
    }

    /**
     * @brief Get rotation object that performs the inverse rotation
     *
     * @return Copy of this that performs the inverse rotation
     */
    [[nodiscard]] constexpr Rot2D inversed() const { return Rot2D{ complex_conj(m_complex) }; }

    /**
     * @brief Stack another rotation object on top of this
     *
     * @note Normalization should be performed occasionally for fixing
     * floating-point errors.
     *
     * @param that The other rotation at hand
     * @param normalize Whether to normalize the rotation object
     * @return This modified
     */
    constexpr Rot2D& stack(const Rot2D& that, bool normalize = true)
    {
        this->m_complex = complex_mul(this->m_complex, that.m_complex);
        if (normalize) {
            this->normalize();
        }
        return (*this);
    }

    /**
     * @brief Apply the rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated
     */
    [[nodiscard]] constexpr Vec2 apply(const Vec2& v) const { return complex_mul(v, m_complex); }

    /**
     * @brief Apply the inverse rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated back
     */
    [[nodiscard]] constexpr Vec2 apply_inv(const Vec2& v) const { return complex_div(v, m_complex, false); }

private:
    Vec2 m_complex = { 1, 0 };

    static constexpr Vec2 complex_from(const T angle) { return { cos(angle), sin(angle) }; }

    static constexpr Vec2 complex_conj(const Vec2& v) { return { v.x, -v.y }; }

    static constexpr Vec2 complex_mul(const Vec2& lhs, const Vec2& rhs)
    {
        /*
            (a + bi) * (c + di), where i^2 = -1
            = ac - bd + (ad + bc)i
            = ac + (-b)d + (ad - (-b)c)i
         */
        const auto lhs_conj = complex_conj(lhs);
        return { dot(lhs_conj, rhs), cross(lhs_conj, rhs) };
    }

    static constexpr Vec2 complex_div(const Vec2& lhs, const Vec2& rhs, const bool normalize)
    {
        const T d = normalize ? dot(rhs, rhs) : 1;
        return complex_mul(lhs, complex_conj(rhs)) / d;
    }
};

/**
 * @brief Class for composing 3D rotations
 *
 * @tparam T                Type of elements
 * @tparam is_col_major     Whether the produced matrix is in column
 * major
 */
template<typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Rot3DType
{
public:
    using Vec3 = Vec<3, T>;
    using Vec4 = Vec<4, T>;
    using Mat3 = Mat<3, 3, T, is_col_major>;
    using Rot3D = Rot3DType<T, is_col_major>;

    /**
     * @brief Construct identity rotation object that does "nothing"
     */
    constexpr Rot3DType() = default;

    /**
     * @brief Construct rotation object from axis and angle in radians
     *
     * @param axis The "axis" vector
     * @param radians The angle (in radians) the "axis" vector is rotated
     * by
     * @param normalize Whether to normalize this in case the axis vector
     * was not pre-normalized
     */
    constexpr Rot3DType(const Vec3& axis, const T radians, bool normalize = true)
        : m_quat{ quat_from(normalize ? axis.normalized() : axis, radians) } {};

    /**
     * @brief Conver to quaternion
     *
     * @return Quaternion as Vec4
     */
    [[nodiscard]] constexpr Vec4 to_quat() const { return m_quat; }

    /**
     * @brief Convert to (normalized) axis and angle
     *
     * @return Tuple of unit axis vector and angle
     */
    [[nodiscard]] constexpr std::tuple<Vec3, T> to_axis_angle() const
    {
        const T half_angle = acos(this->m_quat.w);

        if (almost_equal<T>(half_angle, 0.f)) {
            return { this->m_quat.xyz, T{ 2 } * half_angle };
        } else {
            return { this->m_quat.xyz / sin(half_angle), T{ 2 } * half_angle };
        }
    }

    /**
     * @brief Convert to 3D transformation matrix
     *
     * @return Matrix performing the same rotation as this
     */
    [[nodiscard]] constexpr Mat3 to_mat() const
    {
        const Vec3 x_hat = apply({ 1, 0, 0 });
        const Vec3 y_hat = apply({ 0, 1, 0 });
        const Vec3 z_hat = apply({ 0, 0, 1 });

        return Mat3::from_cols(x_hat, y_hat, z_hat);
    }

    /**
     * @brief Rotate YZ plane by angle around x-axis
     *
     * @param radians The angle in radians measured from y-axis
     * @param normalize Whether to normalize quaternion for numerical
     * stability purposes
     * @return This
     */
    constexpr Rot3D& rotateYZ(const T radians, const bool normalize = true)
    {
        return this->stack(Rot3D{ Vec3{ 1, 0, 0 }, -radians }, normalize);
    }

    /**
     * @brief Rotate ZX plane by angle around y-axis
     *
     * @param radians The angle in radians measured from z-axis
     * @param normalize Whether to normalize quaternion for numerical
     * stability purposes
     * @return This
     */
    constexpr Rot3D& rotateZX(const T radians, const bool normalize = true)
    {
        return this->stack(Rot3D{ Vec3{ 0, 1, 0 }, -radians }, normalize);
    }

    /**
     * @brief Rotate XY plane by angle around z-axis
     *
     * @param radians The angle in radians measured from x-axis
     * @param normalize Whether to normalize quaternion for numerical
     * stability purposes
     * @return This
     */
    constexpr Rot3D& rotateXY(const T radians, const bool normalize = true)
    {
        return this->stack(Rot3D{ Vec3{ 0, 0, 1 }, -radians }, normalize);
    }

    /**
     * @brief Get the rotation object that performs the inverse
     * rotation as this
     *
     * @return Copy of this performing the inverse rotation
     */
    [[nodiscard]] constexpr Rot3D inversed() const
    {
        Rot3D res = (*this);
        res.m_quat.xyz *= -1;
        return res;
    }

    /**
     * @brief Normalize underlying quaternion in-place
     *
     * @return This
     */
    constexpr Rot3D& normalize()
    {
        m_quat = std::move(m_quat.normalized());
        return (*this);
    }

    /**
     * @brief Stack another rotation object on top of this
     *
     * @note Normalization should be performed occasionally for fixing
     * floating-point errors.
     *
     * @param that The other rotation at hand
     * @param normalize Whether to normalize the rotation object
     * @return This modified
     */
    constexpr Rot3D& stack(const Rot3D& that, bool normalize = true)
    {
        this->m_quat = quat_mul(this->m_quat, that.m_quat);
        if (normalize) {
            this->normalize();
        }
        return *this;
    }

    /**
     * @brief Apply the rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated
     */
    [[nodiscard]] constexpr Vec3 apply(const Vec3& v) const
    {
        return quat_mul(quat_mul(this->m_quat, { v.xyz, 0 }), inversed().m_quat).xyz;
    }

    /**
     * @brief Apply the inverse rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated back
     */
    [[nodiscard]] constexpr Vec3 apply_inv(const Vec3& v) const
    {
        return quat_mul(quat_mul(inversed().m_quat, { v.xyz, 0 }), this->m_quat).xyz;
    }

private:
    Vec4 m_quat = { 0, 0, 0, 1 }; ///< quaternion object

    static constexpr Vec4 quat_from(const Vec3& n, const T radians)
    {
        return { sin(radians / 2.f) * n, cos(radians / 2.f) };
    }

    static constexpr Vec4 quat_mul(const Vec4& lhs, const Vec4& rhs)
    {
        return { lhs.w * rhs.xyz + lhs.xyz * rhs.w + cross(lhs.xyz, rhs.xyz), lhs.w * rhs.w - dot(lhs.xyz, rhs.xyz) };
    }
};

} // namespace asciirast::math
