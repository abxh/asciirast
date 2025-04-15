/**
 * @file Rot.h
 * @brief Classes for rotation
 *
 * @todo use library or wait for c++26 for constexpr math functions
 * @todo spherical interpolation
 *
 * Quaternion implementation initially based on following:
 * - https://marctenbosch.com/quaternions/
 * - https://mathworld.wolfram.com/Quaternion.html
 */

#pragma once

#include <cmath>
#include <type_traits>

#include "./Mat.h"
#include "./Vec.h"

namespace asciirast::math {

/**
 * @brief Class for composing unit complex numbers
 *
 * @note not using std::complex, since they don't have constexpr math functions yet.
 * yes, the custom code is unoptimized, but probably will be optimized away by the compiler.
 *
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the produced matrix is in column major
 */
template<typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Rot2
{
    using Vec2 = Vec<2, T>;
    using Mat2 = Mat<2, 2, T, is_col_major>;

    T m_real = 1; // real component
    T m_imag = 0; // imaginary component

public:
    /**
     * @brief Construct identity rotation object that does "nothing"
     */
    constexpr Rot2() {};

    /**
     * @brief (Implicitly) construct rotation object from angle
     *
     * @param angle The angle at hand in radians
     */
    Rot2(const T angle) noexcept
            : m_real{ std::cos(angle) }
            , m_imag{ std::sin(angle) } {};

    /**
     * @brief Construct rotation object from the angle between two
     * (potentially not-normalized) vectors.
     *
     * If both vectors are normalized, then the normalization step can be omitted.
     *
     * @param from_dir The source direction vector
     * @param to_dir The destination direction vector
     * @param normalize Normalize the resulting complex number
     */
    constexpr Rot2(const Vec2& from_dir, const Vec2& to_dir, bool normalize = true) noexcept
            : m_real{ dot(from_dir, to_dir) }
            , m_imag{ cross(from_dir, to_dir) }
    {
        if (normalize) {
            this->normalize();
        }
        // above is a notation hack to do from_dir / to_dir as complex numbers
    };

    /**
     * @brief Get the underlying complex number
     *
     * @return The underlying complex number
     */
    constexpr Vec2 complex() const { return Vec2{ m_real, m_imag }; }

    /**
     * @brief Convert to angle in radians
     *
     * @return The angle in radians
     */
    T to_angle() const { return std::atan2(m_imag, m_real); }

    /**
     * @brief Convert to 2D transformation matrix
     *
     * @return The tranformation matrix that performs the same rotation as this
     */
    constexpr Mat2 to_mat() const
    {
        const Vec2 x_hat = apply(Vec2{ 1, 0 });
        const Vec2 y_hat = apply(Vec2{ 0, 1 });

        return Mat2::from_cols(x_hat, y_hat);
    }

    /**
     * @brief Normalize underlying complex number in-place
     *
     * @return This
     */
    constexpr Rot2& normalize()
    {
        const auto v = this->complex().normalized();
        m_real = v.x;
        m_imag = v.y;
        return (*this);
    }

    /**
     * @brief Get rotation object that performs the reverse rotation
     *
     * @return Copy of this that performs the reverse rotation
     */
    constexpr Rot2 reversed() const
    {
        Rot2 res = (*this);
        res.m_imag *= -1;
        return res;
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
    constexpr Rot2& stack(const Rot2& that, bool normalize = true)
    {
        // yet another notation hack to do complex number multiplication
        (*this) = std::move(Rot2{ reversed().complex(), that.complex(), normalize });
        return (*this);
    }

    /**
     * @brief Apply the rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated
     */
    constexpr Vec2 apply(const Vec2& v) const { return Rot2{ reversed().complex(), v, false }.complex(); }

    /**
     * @brief Apply the inverse rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated back
     */
    constexpr Vec2 apply_inv(const Vec2& v) const { return Rot2{ complex(), v, false }.complex(); }
};

/**
 * @brief Class for composing unit quaternions
 *
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the produced matrix is in column major
 */
template<typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Rot3
{
    using Vec3 = Vec<3, T>;
    using Vec4 = Vec<4, T>;
    using Mat3 = Mat<3, 3, T, is_col_major>;

    T m_s = 1;          ///< scalar component
    Vec3 m_dir = { 0 }; ///< non-normalized axis vector

public:
    /**
     * @brief Construct identity rotation object that does "nothing"
     */
    constexpr Rot3() {};

    /**
     * @brief Construct rotation object from axis and angle in radians
     *
     * @param axis The "axis" vector
     * @param angle The angle (in radians) the "axis" vector is rotated by
     * @param normalize Whether to normalize this in case the axis vector was not pre-normalized
     */
    Rot3(const Vec3& axis, const T angle, bool normalize = true)
            : m_s{ std::cos(angle / 2.f) }
            , m_dir{ std::sin(angle / 2.f) * ((normalize) ? axis.normalized() : axis) } {};

    /**
     * @brief Construct rotation object from the angle between two vectors
     */
    constexpr Rot3(const Vec3& from_dir, const Vec3& to_dir)
            : m_s{ 1 + dot(from_dir, to_dir) }
            , m_dir{ cross(from_dir, to_dir) }
    {
        // using trick for half-angle, with the cosine half-angle formula:
        // cos(theta / 2) = sqrt((1 + cos(theta))/2)

        this->normalize(); // trick requires normalization
    };

    /**
     * @brief Construct rotation object from the angle between two
     * (potentially not-normalized) vectors.
     *
     * If both vectors are normalized, then the normalization step can be omitted.
     *
     * @param lhs The source direction vector
     * @param rhs The destination direction vector
     * @param normalize Normalize the resulting quaternion
     */
    constexpr Rot3(const Rot3& lhs, const Rot3& rhs, bool normalize = true)
            : m_s{ lhs.m_s * rhs.m_s - dot(lhs.m_dir, rhs.m_dir) }
            , m_dir{ lhs.m_s * rhs.m_dir + lhs.m_dir * rhs.m_s + cross(lhs.m_dir, rhs.m_dir) }
    {
        // quaternion multiplication definition in brief form. sidenote: as an excercise,
        // using geometric algebra rules, work out the multiplication.

        if (normalize) {
            this->normalize();
        }
    };

protected:
    /**
     * @brief Construct rotation object from the quaternion-vector product
     *
     * This is defined for internal code convienience
     *
     * @param lhs the left-hand side
     * @param rhs the right-hand side
     */
    constexpr Rot3(const Rot3& lhs, const Vec3& rhs) // like above but with rhs.s == 0
            : m_s{ -dot(lhs.m_dir, rhs) }
            , m_dir{ lhs.m_s * rhs + cross(lhs.m_dir, rhs) } {};

public:
    /**
     * @brief Get the underlying quaternion as Vec4
     *
     * First component is the direction component.
     * Last component is the scalar component.
     *
     * @return Copy of the underlying quaternion as Vec4
     */
    constexpr Vec4 quat() const { return Vec4{ m_dir, m_s }; }

    /**
     * @brief Convert to (normalized) axis and angle
     *
     * @return Tuple of unit axis vector and angle
     */
    std::tuple<Vec3, T> to_axis_angle() const
    {
        const T half_angle = std::acos(m_s);
        const Vec3 axis = m_dir / std::sin(half_angle);

        return { axis, T{ 2 } * half_angle };
    }

    /**
     * @brief Convert to 3D transformation matrix
     *
     * @return Matrix performing the same rotation as this
     */
    constexpr Mat3 to_mat() const
    {
        const Vec3 x_hat = apply(Vec3{ 1, 0, 0 });
        const Vec3 y_hat = apply(Vec3{ 0, 1, 0 });
        const Vec3 z_hat = apply(Vec3{ 0, 0, 1 });

        return Mat3::from_cols(x_hat, y_hat, z_hat);
    }

    /**
     * @brief Rotate by angle in x-axis
     *
     * @param angle The angle in radians
     * @return This
     */
    Rot3& rotateX(const T angle) { return this->stack(Rot3{ Vec3{ 1, 0, 0 }, angle }); }

    /**
     * @brief Rotate by angle in y-axis
     *
     * @param angle The angle in radians
     * @return This
     */
    Rot3& rotateY(const T angle) { return this->stack(Rot3{ Vec3{ 0, 1, 0 }, angle }); }

    /**
     * @brief Rotate by angle in z-axis
     *
     * @param angle The angle in radians
     * @return This
     */
    Rot3& rotateZ(const T angle) { return this->stack(Rot3{ Vec3{ 0, 0, 1 }, angle }); }

    /**
     * @brief Get the rotation object that performs the inverse
     * rotation as this
     *
     * @return Copy of this performing the inverse rotation
     */
    constexpr Rot3 reversed() const
    {
        Rot3 res = (*this);
        res.m_dir *= -1;
        return res;
    }

    /**
     * @brief Normalize underlying quaternion in-place
     *
     * @return This
     */
    constexpr Rot3& normalize()
    {
        const auto v = this->quat().normalized();
        m_dir = v.xyz;
        m_s = v.w;
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
    constexpr Rot3& stack(const Rot3& that, bool normalize = true)
    {
        *this = std::move(Rot3{ *this, that, normalize });
        return *this;
    }

    /**
     * @brief Apply the rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated
     */
    constexpr Vec3 apply(const Vec3& v) const { return Rot3{ Rot3{ (*this), v }, reversed(), false }.m_dir; }

    /**
     * @brief Apply the inverse rotation "action" on
     * a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated back
     */
    constexpr Vec3 apply_inv(const Vec3& v) const { return Rot3{ Rot3{ reversed(), v }, (*this), false }.m_dir; }
};

} // namespace asciirast::math
