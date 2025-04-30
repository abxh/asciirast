/**
 * @file Rot.h
 * @brief Classes for rotation
 *
 * @todo use library or wait for c++26 for constexpr math functions
 * @todo spherical interpolation
 *
 * Quaternion implementation initially based on following:
 * - https://mathworld.wolfram.com/Quaternion.html
 * - https://marctenbosch.com/quaternions/
 */

#pragma once

#include <cmath>
#include <complex>
#include <cstdlib>
#include <type_traits>

#include "./Mat.h"
#include "./Vec.h"

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
    Rot2DType() {};

    /**
     * @brief (Implicitly) construct rotation object from angle
     *
     * @param radians The angle at hand in radians
     */
    Rot2DType(const T radians)
            : m_angle{ std::remainder(radians, 2 * std::numbers::pi_v<T>) } {};

    /**
     * @brief Construct rotation object from the angle between two vectors.
     *
     * @param from_dir The source direction vector
     * @param to_dir The destination direction vector
     */
    Rot2DType(const Vec2& from_dir, const Vec2& to_dir) noexcept
            : m_angle{ std::remainder(math::angle(from_dir, to_dir), 2 * std::numbers::pi_v<T>) } {};

    /**
     * @brief Get underlying angle in radians
     *
     * @return The angle in radians
     */
    [[nodiscard]] const T& angle() const { return m_angle; }

    /**
     * @brief Convert this to unit complex number
     *
     * @return The unit complex number as std::complex
     */
    [[nodiscard]] std::complex<T> to_complex() const { return std::complex<T>(std::cos(m_angle), std::sin(m_angle)); }

    /**
     * @brief Convert to 2D transformation matrix
     *
     * @return The tranformation matrix that performs the same rotation as this
     */
    [[nodiscard]] Mat2 to_mat() const
    {
        const Vec2 x_hat = apply(Vec2{ 1, 0 });
        const Vec2 y_hat = apply(Vec2{ 0, 1 });

        return Mat2::from_cols(x_hat, y_hat);
    }

    /**
     * @brief Get rotation object that performs the reverse rotation
     *
     * @return Copy of this that performs the reverse rotation
     */
    [[nodiscard]] Rot2D reversed() const { return Rot2D{ -m_angle }; }

    /**
     * @brief Stack another rotation object on top of this
     *
     * @note Normalization should be performed occasionally for fixing floating-point errors.
     *
     * @param that The other rotation at hand
     * @return This modified
     */
    Rot2D& stack(const Rot2D& that)
    {
        this->m_angle = std::remainder(this->m_angle + that.m_angle, 2 * std::numbers::pi_v<T>);
        return (*this);
    }

    /**
     * @brief Apply the rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated
     */
    [[nodiscard]] Vec2 apply(const Vec2& v) const { return as_vec(as_complex(v) * this->to_complex()); }

    /**
     * @brief Apply the inverse rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated back
     */
    [[nodiscard]] Vec2 apply_inv(const Vec2& v) const { return as_vec(as_complex(v) / this->to_complex()); }

private:
    T m_angle = 0.f;

    static Vec2 as_vec(const std::complex<T>& c) { return { c.real(), c.imag() }; }
    static std::complex<T> as_complex(const Vec2& v) { return { v.x, v.y }; }
};

/**
 * @brief Class for composing 3D rotations
 *
 * @tparam T                Type of elements
 * @tparam is_col_major     Whether the produced matrix is in column major
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
    Rot3DType() {};

    /**
     * @brief Construct rotation object from axis and angle in radians
     *
     * @param axis The "axis" vector
     * @param radians The angle (in radians) the "axis" vector is rotated by
     * @param normalize Whether to normalize this in case the axis vector was not pre-normalized
     */
    Rot3DType(const Vec3& axis, const T radians, bool normalize = true)
            : m_quat{ quat_from(normalize ? axis.normalized() : axis, radians) } {};

    /**
     * @brief Construct rotation object from the angle between two vectors
     *
     * @param from_dir The source vector
     * @param to_dir The destination vector
     */
    Rot3DType(const Vec3& from_dir, const Vec3& to_dir)
            : m_quat{ cross(from_dir, to_dir), 1 + dot(from_dir, to_dir) }
    {
        this->normalize(); // trick for half-angle cosine
    };

    /**
     * @brief Get underlying quaternion
     *
     * @return Quaternion as Vec4
     */
    [[nodiscard]] const Vec4& quat() const { return m_quat; }

    /**
     * @brief Convert to (normalized) axis and angle
     *
     * @return Tuple of unit axis vector and angle
     */
    [[nodiscard]] std::tuple<Vec3, T> to_axis_angle() const
    {
        const T half_angle = std::acos(this->m_quat.w);

        if (almost_equal<T>(half_angle, 0.f)) {
            return { this->m_quat.xyz, T{ 2 } * half_angle };
        } else {
            return { this->m_quat.xyz / std::sin(half_angle), T{ 2 } * half_angle };
        }
    }

    /**
     * @brief Convert to 3D transformation matrix
     *
     * @return Matrix performing the same rotation as this
     */
    [[nodiscard]] Mat3 to_mat() const
    {
        const Vec3 x_hat = apply(Vec3{ 1, 0, 0 });
        const Vec3 y_hat = apply(Vec3{ 0, 1, 0 });
        const Vec3 z_hat = apply(Vec3{ 0, 0, 1 });

        return Mat3::from_cols(x_hat, y_hat, z_hat);
    }

    /**
     * @brief Rotate YZ plane by angle around x-axis
     *
     * @param radians The angle in radians measured from y-axis
     * @param normalize Whether to normalize quaternion for numerical stability purposes
     * @return This
     */
    Rot3D& rotateYZ(const T radians, const bool normalize = true)
    {
        return this->stack(Rot3D{ Vec3{ 1, 0, 0 }, -radians }, normalize);
    }

    /**
     * @brief Rotate XZ plane by angle around y-axis
     *
     * @param radians The angle in radians measured from x-axis
     * @param normalize Whether to normalize quaternion for numerical stability purposes
     * @return This
     */
    Rot3D& rotateXZ(const T radians, const bool normalize = true)
    {
        return this->stack(Rot3D{ Vec3{ 0, 1, 0 }, +radians }, normalize);
    }

    /**
     * @brief Rotate XY plane by angle around z-axis
     *
     * @param radians The angle in radians measured from x-axis
     * @param normalize Whether to normalize quaternion for numerical stability purposes
     * @return This
     */
    Rot3D& rotateXY(const T radians, const bool normalize = true)
    {
        return this->stack(Rot3D{ Vec3{ 0, 0, 1 }, -radians }, normalize);
    }

    /**
     * @brief Get the rotation object that performs the inverse
     * rotation as this
     *
     * @return Copy of this performing the inverse rotation
     */
    [[nodiscard]] Rot3D reversed() const
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
    Rot3D& normalize()
    {
        m_quat = std::move(m_quat.normalized());
        return (*this);
    }

    /**
     * @brief Stack another rotation object on top of this
     *
     * @note Normalization should be performed occasionally for fixing floating-point errors.
     *
     * @param that The other rotation at hand
     * @param normalize Whether to normalize the rotation object
     * @return This modified
     */
    Rot3D& stack(const Rot3D& that, bool normalize = true)
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
    [[nodiscard]] Vec3 apply(const Vec3& v) const { return quat_mul(quat_mul(this->m_quat, v), reversed().m_quat).xyz; }

    /**
     * @brief Apply the inverse rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated back
     */
    [[nodiscard]] Vec3 apply_inv(const Vec3& v) const
    {
        return quat_mul(quat_mul(reversed().m_quat, v), this->m_quat).xyz;
    }

private:
    Vec4 m_quat = { 0, 0, 0, 1 }; ///< quaternion object

    static Vec4 quat_from(const Vec3& n, const T radians)
    {
        return { std::sin(radians / 2.f) * n, std::cos(radians / 2.f) };
    }

    static Vec4 quat_mul(const Vec4& lhs, const Vec4& rhs)
    {
        return { lhs.w * rhs.xyz + lhs.xyz * rhs.w + cross(lhs.xyz.to_vec(), rhs.xyz.to_vec()),
                 lhs.w * rhs.w - dot(lhs.xyz.to_vec(), rhs.xyz.to_vec()) };
    }
};

} // namespace asciirast::inverse
