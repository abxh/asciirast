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

private:
    T m_real = 1; // real component
    T m_imag = 0; // imaginary component

public:
    /**
     * @brief Construct identity rotation object that does "nothing"
     */
    constexpr Rot2DType() {};

    /**
     * @brief (Implicitly) construct rotation object from angle
     *
     * @param angle The angle at hand in radians
     */
    Rot2DType(const T angle) noexcept
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
    Rot2DType(const Vec2& from_dir, const Vec2& to_dir, bool normalize = true) noexcept
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
    [[nodiscard]] Vec2 complex() const { return Vec2{ m_real, m_imag }; }

    /**
     * @brief Convert to angle in radians
     *
     * @return The angle in radians
     */
    [[nodiscard]] T to_angle() const { return std::atan2(m_imag, m_real); }

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
     * @brief Normalize underlying complex number in-place
     *
     * @return This
     */
    Rot2D& normalize()
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
    [[nodiscard]] Rot2D reversed() const
    {
        Rot2D res = (*this);
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
    Rot2D& stack(const Rot2D& that, bool normalize = true)
    {
        // yet another notation hack to do complex number multiplication
        (*this) = std::move(Rot2D{ reversed().complex(), that.complex(), normalize });
        return (*this);
    }

    /**
     * @brief Apply the rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated
     */
    [[nodiscard]] Vec2 apply(const Vec2& v) const { return Rot2D{ reversed().complex(), v, false }.complex(); }

    /**
     * @brief Apply the inverse rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated back
     */
    [[nodiscard]] Vec2 apply_inv(const Vec2& v) const { return Rot2D{ complex(), v, false }.complex(); }
};

/**
 * @brief Class for composing unit quaternions
 *
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the produced matrix is in column major
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

private:
    T m_s = 1;          ///< scalar component
    Vec3 m_dir = { 0 }; ///< non-normalized axis vector

public:
    /**
     * @brief Construct identity rotation object that does "nothing"
     */
    Rot3DType() {};

    /**
     * @brief Construct rotation object from axis and angle in radians
     *
     * @param axis The "axis" vector
     * @param angle The angle (in radians) the "axis" vector is rotated by
     * @param normalize Whether to normalize this in case the axis vector was not pre-normalized
     */
    Rot3DType(const Vec3& axis, const T angle, bool normalize = true)
            : m_s{ std::cos(angle / 2.f) }
            , m_dir{ std::sin(angle / 2.f) * ((normalize) ? axis.normalized() : axis) } {};

    /**
     * @brief Construct rotation object from the angle between two vectors
     */
    Rot3DType(const Vec3& from_dir, const Vec3& to_dir)
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
    Rot3DType(const Rot3D& lhs, const Rot3D& rhs, bool normalize = true)
            : m_s{ lhs.m_s * rhs.m_s - dot(lhs.m_dir, rhs.m_dir) }
            , m_dir{ lhs.m_s * rhs.m_dir + lhs.m_dir * rhs.m_s + cross(lhs.m_dir, rhs.m_dir) }
    {
        // quaternion multiplication definition in brief form. sidenote: as an excercise,
        // using geometric algebra rules, work out the multiplication.

        if (normalize) {
            this->normalize();
        }
    };

    /**
     * @brief Get the underlying quaternion as Vec4
     *
     * First component is the direction component.
     * Last component is the scalar component.
     *
     * @return Copy of the underlying quaternion as Vec4
     */
    [[nodiscard]] Vec4 quat() const { return Vec4{ m_dir, m_s }; }

    /**
     * @brief Convert to (normalized) axis and angle
     *
     * @return Tuple of unit axis vector and angle
     */
    [[nodiscard]] std::tuple<Vec3, T> to_axis_angle() const
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
    [[nodiscard]] Mat3 to_mat() const
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
    Rot3D& rotateX(const T angle) { return this->stack(Rot3D{ Vec3{ 1, 0, 0 }, angle }); }

    /**
     * @brief Rotate by angle in y-axis
     *
     * @param angle The angle in radians
     * @return This
     */
    Rot3D& rotateY(const T angle) { return this->stack(Rot3D{ Vec3{ 0, 1, 0 }, angle }); }

    /**
     * @brief Rotate by angle in z-axis
     *
     * @param angle The angle in radians
     * @return This
     */
    Rot3D& rotateZ(const T angle) { return this->stack(Rot3D{ Vec3{ 0, 0, 1 }, angle }); }

    /**
     * @brief Get the rotation object that performs the inverse
     * rotation as this
     *
     * @return Copy of this performing the inverse rotation
     */
    [[nodiscard]] constexpr Rot3D reversed() const
    {
        Rot3D res = (*this);
        res.m_dir *= -1;
        return res;
    }

    /**
     * @brief Normalize underlying quaternion in-place
     *
     * @return This
     */
    Rot3D& normalize()
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
    Rot3D& stack(const Rot3D& that, bool normalize = true)
    {
        *this = std::move(Rot3D{ *this, that, normalize });
        return *this;
    }

    /**
     * @brief Apply the rotation "action" on a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated
     */
    [[nodiscard]] Vec3 apply(const Vec3& v) const { return Rot3D{ Rot3D{ (*this), v }, reversed(), false }.m_dir; }

    /**
     * @brief Apply the inverse rotation "action" on
     * a vector
     *
     * @param v The vector at hand
     * @return The copy of the vector rotated back
     */
    [[nodiscard]] constexpr Vec3 apply_inv(const Vec3& v) const
    {
        return Rot3D{ Rot3D{ reversed(), v }, (*this), false }.m_dir;
    }

protected:
    /**
     * @brief Construct rotation object from the quaternion-vector product
     *
     * This is defined for internal code convienience
     *
     * @param lhs the left-hand side
     * @param rhs the right-hand side
     */
    Rot3DType(const Rot3D& lhs, const Vec3& rhs) // like Rot3 * Rot3 but with rhs.s == 0
            : m_s{ -dot(lhs.m_dir, rhs) }
            , m_dir{ lhs.m_s * rhs + cross(lhs.m_dir, rhs) } {};
};

} // namespace asciirast::math
