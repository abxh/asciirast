/**
 * @file Rot.h
 * @brief File with definition of the rotation abstraction
 */

#pragma once

#include <cassert>
#include <cmath>
#include <type_traits>

#include "Mat.h"
#include "Vec.h"

namespace asciirast::math {

/**
 * @brief Rotation abstraction
 *
 * @tparam N                    Number of dimensions being operated on.
 * @tparam T                    Type of elements
 * @tparam mat_is_column_major  Whether the produced matrix is in column major
 */
template<std::size_t N, typename T, bool mat_is_column_major>
    requires(std::is_floating_point_v<T>)
class Rot;

/**
 * @brief 2D rotation abstraction (unit complex number)
 *
 * @tparam T                    Type of elements
 * @tparam mat_is_column_major  Whether the produced matrix is in column major
 */
template<typename T, bool mat_is_column_major>
    requires(std::is_floating_point_v<T>)
class Rot<2, T, mat_is_column_major>
{
public:
    using Vec2 = Vec<2, T>;
    using Mat2 = Mat<2, 2, T, mat_is_column_major>;

    union
    {
        Vec2 dir; ///< as direction vector
        struct
        {
            T real; ///< real component
            T imag; ///< imaginary component
        };
    };

    /**
     * @brief Construct identity rotation object
     */
    Rot()
            : real{ 1 }
            , imag{ 0 } {};

    /**
     * @brief Construct rotation object from angle in radians
     */
    explicit Rot(const T angle)
            : real{ std::cos(angle) }
            , imag{ std::sin(angle) } {};

    /**
     * @brief Construct rotation object from the angle between two vectors
     */
    Rot(const Vec2& from_dir, const Vec2& to_dir, bool normalize = true)
            : real{ dot(from_dir, to_dir) }
            , imag{ cross(from_dir, to_dir) }
    {
        if (normalize) {
            this->normalize();
        }
    };

public:
    /**
     * @brief Convert to angle in radians
     */
    T to_angle() const { return std::atan2(imag, real); }

    /**
     * @brief Convert to 2D transformation matrix
     */
    Mat2 to_mat2() const
    {
        const Vec2 x_hat = apply(Vec2{ 1, 0 });
        const Vec2 y_hat = apply(Vec2{ 0, 1 });

        return Mat2::from_columns(x_hat, y_hat);
    }

    /**
     * @brief Normalize rotation object in-place
     */
    Rot& normalize()
    {
        dir = std::move(dir.normalized());
        return (*this);
    }

    /**
     * @brief Get rotation object that performs the reverse rotation
     */
    Rot reversed() const
    {
        Rot res = (*this);
        res.imag *= -1;
        return res;
    }

    /**
     * @brief Stack another rotation object on top of this
     */
    Rot& stack(const Rot& that, bool normalize = true)
    {
        return *this = std::move(Rot{ reversed().dir, that.dir, normalize });
    }

    /**
     * @brief Apply the rotation to a vector
     */
    Vec2 apply(const Vec2& v) const
    {
        return Rot{ reversed().dir, v, false }.dir;
    }

    /**
     * @brief Invert the applied rotation from a vector
     */
    Vec2 invert(const Vec2& v) const { return Rot{ dir, v, false }.dir; }
};

/**
 * @brief 3D rotation abstraction (unit quaternion)
 *
 * @tparam T                    Type of elements
 * @tparam mat_is_column_major  Whether the produced matrix is in column major
 */
template<typename T, bool mat_is_column_major>
    requires(std::is_floating_point_v<T>)
class Rot<3, T, mat_is_column_major>
{
public:
    using Vec3 = Vec<3, T>;
    using Vec4 = Vec<4, T>;
    using Mat3 = Mat<3, 3, T, mat_is_column_major>;

    union
    {
        Vec4 quat; ///< as quaternion
        struct
        {
            Vec3 dir; ///< non-normalized axis vector
            T s;      ///< scalar component
        };
    };

    /**
     * @brief Rotate by angle in x axis
     */
    static Rot X(const T angle) { return Rot{ Vec3{ 1, 0, 0 }, angle }; }

    /**
     * @brief Rotate by angle in y axis
     */
    static Rot Y(const T angle) { return Rot{ Vec3{ 0, 1, 0 }, angle }; }

    /**
     * @brief Rotate by angle in z axis
     */
    static Rot Z(const T angle) { return Rot{ Vec3{ 0, 0, 1 }, angle }; }

    /**
     * @brief Construct identity rotation object
     */
    Rot()
            : s{ 1 }
            , dir{ 0, 0, 0 } {};

    /**
     * @brief Construct rotation object from axis and angle in radians
     */
    Rot(const Vec3& axis, const T angle, bool normalize = true)
            : s{ std::cos(angle / 2.f) }
            , dir{ std::sin(angle / 2.f) *
                   ((normalize) ? axis.normalized() : axis) } {};

    /**
     * @brief Construct rotation object from the angle between two vectors
     */
    Rot(const Vec3& from_dir, const Vec3& to_dir)
            : s{ 1 + dot(from_dir, to_dir) } // trick for half-angle
            , dir{ cross(from_dir, to_dir) }
    {
        this->normalize(); // trick requires normalization
    };

    /**
     * @brief Construct rotation object from the multiplication of two
     * rotation objects
     */
    Rot(const Rot& lhs, const Rot& rhs, bool normalize = true)
            : s{ lhs.s * rhs.s - dot(lhs.dir, rhs.dir) }
            , dir{ lhs.s * rhs.dir + lhs.dir * rhs.s + cross(lhs.dir, rhs.dir) }
    {
        if (normalize) {
            this->normalize();
        }
    };

    /**
     * @brief Construct rotation object from the multiplication of a rotation
     * object on a vector
     */
    Rot(const Rot& lhs, const Vec3& v)
            : s{ -dot(lhs.dir, v) }
            , dir{ lhs.s * v + cross(lhs.dir, v) } {};

public:
    /**
     * @brief Convert to (normalized) axis and angle
     */
    std::tuple<Vec3, T> to_axis_angle() const
    {
        const T half_angle = std::acos(s);
        const Vec3 axis = dir / std::sin(half_angle);

        return { axis, T{ 2 } * half_angle };
    }

    /**
     * @brief Convert to 3D transformation matrix
     */
    Mat3 to_mat3() const
    {
        const Vec3 x_hat = apply(Vec3{ 1, 0, 0 });
        const Vec3 y_hat = apply(Vec3{ 0, 1, 0 });
        const Vec3 z_hat = apply(Vec3{ 0, 0, 1 });

        return Mat3::from_columns(x_hat, y_hat, z_hat);
    }

    /**
     * @brief Normalize rotation in-place
     */
    Rot& normalize()
    {
        quat = std::move(quat.normalized());
        return (*this);
    }

    /**
     * @brief Get rotation that performs the reverse rotation
     */
    Rot reversed() const
    {
        Rot res = (*this);
        res.dir *= -1;
        return res;
    }

    /**
     * @brief Stack another rotation on top of this
     */
    Rot& stack(const Rot& that, bool normalize = true)
    {
        return Rot{ this->quat, that.quat, normalize };
    }

    /**
     * @brief Apply the rotation to a vector
     */
    Vec3 apply(const Vec3& v) const
    {
        return Rot{ Rot{ (*this), v }, reversed() }.dir;
    }

    /**
     * @brief Invert the applied rotation from a vector
     */
    Vec3 invert(const Vec3& v) const
    {
        return Rot{ Rot{ reversed(), v }, (*this) }.dir;
    }
};

}
