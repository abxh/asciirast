/**
 * @file Rot.h
 * @brief File with definition of the rotation abstraction
 * @todo spherical interpolation
 */

#pragma once

#include <cassert>
#include <cmath>
#include <type_traits>

#include "./Mat.h"
#include "./Vec.h"

namespace asciirast::math {

/**
 * @brief Rotation abstraction
 *
 * @tparam N             Number of dimensions being operated on.
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the produced matrix is in column major
 */
template<std::size_t N, typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Rot;

/**
 * @brief 2D rotation abstraction (unit complex number)
 *
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the produced matrix is in column major
 */
template<typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Rot<2, T, is_col_major>
{
    using Vec2 = Vec<2, T>;
    using Mat2 = Mat<2, 2, T, is_col_major>;

    union
    {
        Vec2 m_dir; ///< as direction vector
        struct
        {
            T m_real; ///< real component
            T m_imag; ///< imaginary component
        };
    };

public:
    /**
     * @brief Construct identity rotation object
     */
    Rot()
            : m_real{ 1 }
            , m_imag{ 0 } {};

    /**
     * @brief Construct rotation object from angle in radians
     */
    explicit Rot(const T angle)
            : m_real{ std::cos(angle) }
            , m_imag{ std::sin(angle) } {};

    /**
     * @brief Construct rotation object from the angle between two vectors
     */
    Rot(const Vec2& from_dir, const Vec2& to_dir, bool normalize = true)
            : m_real{ dot(from_dir, to_dir) }
            , m_imag{ cross(from_dir, to_dir) }
    {
        if (normalize) {
            this->normalize();
        }
    };

    /**
     * @brief Get the (underlying) direction vector
     */
    const Vec2& dir() const { return m_dir; }

    /**
     * @brief Convert to angle in radians
     */
    T to_angle() const { return std::atan2(m_imag, m_real); }

    /**
     * @brief Convert to 2D transformation matrix
     */
    Mat2 to_mat() const
    {
        const Vec2 x_hat = apply(Vec2{ 1, 0 });
        const Vec2 y_hat = apply(Vec2{ 0, 1 });

        return Mat2::from_cols(x_hat, y_hat);
    }

    /**
     * @brief Normalize rotation object in-place
     */
    Rot& normalize()
    {
        m_dir = std::move(m_dir.normalized());
        return (*this);
    }

    /**
     * @brief Get rotation object that performs the reverse rotation
     */
    Rot reversed() const
    {
        Rot res = (*this);
        res.m_imag *= -1;
        return res;
    }

    /**
     * @brief Stack another rotation object on top of this
     */
    Rot& stack(const Rot& that, bool normalize = true)
    {
        return *this = std::move(Rot{ reversed().m_dir, that.m_dir, normalize });
    }

    /**
     * @brief Apply the rotation to a vector
     */
    Vec2 apply(const Vec2& v) const { return Rot{ reversed().m_dir, v, false }.m_dir; }

    /**
     * @brief Invert the applied rotation from a vector
     */
    Vec2 invert(const Vec2& v) const { return Rot{ m_dir, v, false }.m_dir; }
};

/**
 * @brief 3D rotation abstraction (unit quaternion)
 *
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the produced matrix is in column major
 */
template<typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class Rot<3, T, is_col_major>
{
    using Vec3 = Vec<3, T>;
    using Vec4 = Vec<4, T>;
    using Mat3 = Mat<3, 3, T, is_col_major>;

    union
    {
        Vec4 m_quat; ///< as quaternion
        struct
        {
            Vec3 m_dir; ///< non-normalized axis vector
            T m_s;      ///< scalar component
        };
    };

public:
    /**
     * @brief Construct identity rotation object
     */
    Rot()
            : m_s{ 1 }
            , m_dir{ 0, 0, 0 } {};

    /**
     * @brief Construct rotation object from axis and angle in radians
     */
    Rot(const Vec3& axis, const T angle, bool normalize = true)
            : m_s{ std::cos(angle / 2.f) }
            , m_dir{ std::sin(angle / 2.f) * ((normalize) ? axis.normalized() : axis) } {};

    /**
     * @brief Construct rotation object from the angle between two vectors
     */
    Rot(const Vec3& from_dir, const Vec3& to_dir)
            : m_s{ 1 + dot(from_dir, to_dir) } // trick for half-angle
            , m_dir{ cross(from_dir, to_dir) }
    {
        this->normalize(); // trick requires normalization
    };

    /**
     * @brief Construct rotation object from the multiplication of two
     * rotation objects
     */
    Rot(const Rot& lhs, const Rot& rhs, bool normalize = true)
            : m_s{ lhs.m_s * rhs.m_s - dot(lhs.m_dir, rhs.m_dir) }
            , m_dir{ lhs.m_s * rhs.m_dir + lhs.m_dir * rhs.m_s + cross(lhs.m_dir, rhs.m_dir) }
    {
        if (normalize) {
            this->normalize();
        }
    };

    /**
     * @brief Construct rotation object from the multiplication of a rotation
     * object on a vector
     */
    Rot(const Rot& lhs, const Vec3& v) // like above but with rhs.s == 0
            : m_s{ -dot(lhs.m_dir, v) }
            , m_dir{ lhs.m_s * v + cross(lhs.m_dir, v) } {};

    /**
     * @brief Rotate by angle in x axis
     */
    Rot& rotateX(const T angle) { return this->stack(Rot{ Vec3{ 1, 0, 0 }, angle }); }

    /**
     * @brief Rotate by angle in y axis
     */
    Rot& rotateY(const T angle) { return this->stack(Rot{ Vec3{ 0, 1, 0 }, angle }); }

    /**
     * @brief Rotate by angle in z axis
     */
    Rot& rotateZ(const T angle) { return this->stack(Rot{ Vec3{ 0, 0, 1 }, angle }); }

    /**
     * @brief Get the underlying quaternion
     */
    const Vec4& quat() const { return m_quat; }

    /**
     * @brief Convert to (normalized) axis and angle
     */
    std::tuple<Vec3, T> to_axis_angle() const
    {
        const T half_angle = std::acos(m_s);
        const Vec3 axis = m_dir / std::sin(half_angle);

        return { axis, T{ 2 } * half_angle };
    }

    /**
     * @brief Convert to 3D transformation matrix
     */
    Mat3 to_mat() const
    {
        const Vec3 x_hat = apply(Vec3{ 1, 0, 0 });
        const Vec3 y_hat = apply(Vec3{ 0, 1, 0 });
        const Vec3 z_hat = apply(Vec3{ 0, 0, 1 });

        return Mat3::from_cols(x_hat, y_hat, z_hat);
    }

    /**
     * @brief Get rotation that performs the reverse rotation
     */
    Rot reversed() const
    {
        Rot res = (*this);
        res.m_dir *= -1;
        return res;
    }

    /**
     * @brief Normalize rotation in-place
     */
    Rot& normalize()
    {
        m_quat = std::move(m_quat.normalized());
        return (*this);
    }

    /**
     * @brief Stack another rotation on top of this
     */
    Rot& stack(const Rot& that, bool normalize = true) { return Rot{ this->m_quat, that.m_quat, normalize }; }

    /**
     * @brief Apply the rotation to a vector
     */
    Vec3 apply(const Vec3& v) const { return Rot{ Rot{ (*this), v }, reversed(), false }.m_dir; }

    /**
     * @brief Invert the applied rotation from a vector
     */
    Vec3 invert(const Vec3& v) const { return Rot{ Rot{ reversed(), v }, (*this), false }.m_dir; }
};

} // namespace asciirast::math
