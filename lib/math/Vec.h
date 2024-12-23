/**
 * @file Vec.h
 * @brief Math vector
 */

#pragma once

#include "VecBase.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <ostream>
#include <type_traits>

namespace asciirast::math {

/**
 * @brief N-dimensional math vector
 */
template <std::size_t N, typename T>
    requires(N > 1 && std::is_arithmetic_v<T>)
class Vec;

/**
 * @brief 2-dimensional math vector
 */
template <typename T>
using Vec2 = Vec<2, T>;

/**
 * @brief 3-dimensional math vector
 */
template <typename T>
using Vec3 = Vec<3, T>;

/**
 * @brief 4-dimensional math vector
 */
template <typename T>
using Vec4 = Vec<4, T>;

template <std::size_t N, typename T>
    requires(N > 1 && std::is_arithmetic_v<T>)
class Vec : public VecBase<Vec, N, T> {
private:
    using Base = VecBase<Vec, N, T>;

public:
    /**
     * @brief The value type.
     */
    using value_type = T;

    /**
     * @brief The size of the vector
     */
    constexpr auto size() const
    {
        return N;
    }

    /**
     * @brief Zero identity of type
     */
    static constexpr T zero()
    {
        return T{0};
    }

    /**
     * @brief One identity of type
     */
    static constexpr T one()
    {
        return T{1};
    }

    /**
     * @brief Default constructor. Set all values to 0.
     */
    Vec()
        : Base{zero()} {};

    /**
     * @brief Use initial value to fill the entire vector.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    Vec(const U &initial_value)
        : Base{initial_value} {};

    /**
     * @brief Initiate a 2-dimensional vector
     */
    template <typename U1, typename U2>
        requires(utils::non_narrowing_conv<value_type, U1, U2>)
    Vec(const U1 &x, const U2 &y)
        requires(N == 2)
        : Base{x, y} {};

    /**
     * @brief Initiate a 3-dimensional vector
     */
    template <typename U1, typename U2, typename U3>
        requires(utils::non_narrowing_conv<value_type, U1, U2, U3>)
    Vec(const U1 &x, const U2 &y, const U3 &z)
        requires(N == 3)
        : Base{x, y, z} {};

    /**
     * @brief Initiate a 3-dimensional vector with a value and a 2-dimensional
     * vector
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    Vec(const U &l, const Vec<2, T> &r)
        requires(N == 3)
        : Base{l, r[0], r[1]} {};

    /**
     * @brief Initiate a 3-dimensional vector with a 2-dimensional vector and
     * value
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    Vec(const Vec<2, T> &l, const U &r)
        requires(N == 3)
        : Base{l[0], l[1], r} {};

    /**
     * @brief Initiate a 4-dimensional vector
     */
    template <typename U1, typename U2, typename U3, typename U4>
        requires(utils::non_narrowing_conv<value_type, U1, U2, U3, U4>)
    Vec(const U1 &x, const U2 &y, const U3 &z, const U4 &w)
        requires(N == 4)
        : Base{x, y, z, w} {};

    /**
     * @brief Initiate a 4-dimensional vector with two 2-dimensional vectors
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    Vec(const U &l, const Vec<3, T> &r)
        requires(N == 4)
        : Base{l, r[0], r[1], r[2]} {};

    /**
     * @brief Initiate a 4-dimensional vector with a 3-dimensional vector and
     * value
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    Vec(const Vec<3, T> &l, const U &r)
        requires(N == 4)
        : Base{l[0], l[1], l[2], r} {};

    /**
     * @brief Initiate a 4-dimensional vector with a value and 3-dimensional
     * vector
     */
    Vec(const Vec<2, T> &l, const Vec<2, T> &r)
        requires(N == 4)
        : Base{l[0], l[1], r[0], r[1]} {};

    /**
     * @brief Initiate a 4-dimensional vector with two values and 2-dimensional
     * vector
     */
    template <typename U1, typename U2>
        requires(utils::non_narrowing_conv<value_type, U1, U2>)
    Vec(const U1 &l, const Vec<2, T> &m, const U2 &r)
        requires(N == 4)
        : Base{l, m[0], m[1], r} {};

    /**
     * @brief Initiate a N-dimensional vector
     */
    template <typename... Us>
        requires(utils::non_narrowing_conv<value_type, Us...>)
    Vec(const Us &...values)
        requires(4 < N && N == sizeof...(values))
        : Base{values...} {};

    /**
     * @brief Use array to fill the vector.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    Vec(const std::array<T, N> &array)
        : Base{array} {};

    /**
     * @brief Use pointer to fill the vector.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    Vec(const std::size_t n, const T *ptr)
        : Base{n, ptr} {};

    /**
     * @brief Print the vector
     */
    friend std::ostream &operator<<(std::ostream &out, const Vec &v)
    {
        out << "[" << v[0];
        for (std::size_t i = 1; i < N; i++) {
            out << "," << (v[i] >= zero() ? " " : "") << v[i];
        }
        out << "]";
        return out;
    }

    /**
     * @brief Check if exactly equal to another vector in terms of bitwise
     * equality.
     */
    bool equals_bitwise(const Vec<N, T> &other) const
    {
        return std::equal(this->begin(), this->end(), other.begin(),
                          other.end());
    }

    /**
     * @brief Check if equal to integral type vector
     */
    friend bool operator==(const Vec<N, T> &lhs, const Vec<N, T> &rhs)
        requires(std::is_integral_v<T>)
    {
        return lhs.equals_bitwise(rhs);
    }

    /**
     * @brief Check if equal to floating type vector, given a precision for
     * numbers of ulp (units in last place).
     *
     * The lower, the more precise --- desirable for small floats.
     * The higher, the less precise --- desirable for large floats
     *
     * Based on:
     * https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
     */
    bool equals_almost(const Vec<N, T> &other, const unsigned ulps) const
        requires(std::is_floating_point_v<T>)
    {
        const auto n = static_cast<float>(ulps);
        return std::equal(
            this->begin(), this->end(), other.begin(), other.end(),
            [&](const T &x, const T &y) {
                const T m = std::min(std::fabs(x), std::fabs(y));

                const int exp = m < std::numeric_limits<T>::min()
                                  ? std::numeric_limits<T>::min_exponent - 1
                                  : std::ilogb(m);

                const auto lhs = std::fabs(x - y);
                const auto rhs =
                    n * std::ldexp(std::numeric_limits<T>::epsilon(), exp);

                return lhs <= rhs;
            });
    }

    /**
     * @brief Add with another vector
     */
    const Vec &operator+=(const Vec &rvalue)
    {
        std::transform(this->begin(), this->end(), rvalue.begin(),
                       this->begin(), [](const T &x, const T &y) {
                           return x + y;
                       });
        return *this;
    }

    /**
     * @brief Perform vector addition
     */
    friend Vec operator+(const Vec &lhs, const Vec &rhs)
    {
        Vec res;
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), res.begin(),
                       [](const T &x, const T &y) {
                           return x + y;
                       });
        return res;
    }

    /**
     * @brief Subtract with another vector
     */
    const Vec &operator-=(const Vec &rvalue)
    {
        std::transform(this->begin(), this->end(), rvalue.begin(),
                       this->begin(), [](const T &x, const T &y) {
                           return x - y;
                       });
        return *this;
    }

    /**
     * @brief Perform vector subtraction
     */
    friend Vec operator-(const Vec &lhs, const Vec &rhs)
    {
        Vec res;
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), res.begin(),
                       [](const T &x, const T &y) {
                           return x - y;
                       });
        return res;
    }

    /**
     * @brief Element-wise multiply with another vector
     */
    const Vec &operator*=(const Vec &rvalue)
    {
        std::transform(this->begin(), this->end(), rvalue.begin(),
                       this->begin(), [](const T &x, const T &y) {
                           return x * y;
                       });
        return *this;
    }

    /**
     * @brief Perform element-wise vector multiplication
     *
     * See:
     * https://www.youtube.com/watch?v=htYh-Tq7ZBI (Freya Holmer's talk)
     */
    friend Vec operator*(const Vec &lhs, const Vec &rhs)
    {
        Vec res;
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), res.begin(),
                       [](const T &x, const T &y) {
                           return x * y;
                       });
        return res;
    }

    /**
     * @brief Multiply vector with scalar from left-hand-side
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    friend Vec operator*(const U &scalar, const Vec &vec)
    {
        Vec res;
        std::transform(vec.begin(), vec.end(), res.begin(), [&](const T &x) {
            return scalar * x;
        });
        return res;
    }

    /**
     * @brief Multiply vector with scalar from right-hand-side
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    friend Vec operator*(const Vec &vec, const U &scalar)
    {
        Vec res;
        std::transform(vec.begin(), vec.end(), res.begin(), [&](const T &x) {
            return x * scalar;
        });
        return res;
    }

    /**
     * @brief Multiply vector with inverse scalar from right-hand-side
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    friend Vec operator/(const Vec &vec, const U &scalar)
        requires(std::is_floating_point_v<T>)
    {
        Vec res;
        std::transform(vec.begin(), vec.end(), res.begin(), [&](const T &x) {
            return x / scalar;
        });
        return res;
    }

    /**
     * @brief Get a vector from this pointing to other. Alias to
     * operator-(other, *this)
     */
    Vec<N, T> vector_to(const Vec &other) const
    {
        return this->operator-(other, *this);
    }

    /**
     * @brief Take the dot product with another vector
     *
     * See:
     * https://www.youtube.com/watch?v=NzjF1pdlK7Y (3blue1brown)
     */
    T dot(const Vec &rhs) const
    {
        return std::transform_reduce(
            this->begin(), this->end(), rhs.begin(), zero(),
            [](const auto &lterm, const auto &rterm) {
                return lterm + rterm;
            },
            [](const auto &lcomp, const auto &rcomp) {
                return lcomp * rcomp;
            });
    }

    /**
     * @brief Calculate the norm of the vector.
     */
    T norm() const
        requires(std::is_floating_point_v<T>)
    {
        return std::sqrt(this->dot(*this));
    }

    /**
     * @brief Calculate the normalized vector
     */
    Vec<N, T> normalized() const
        requires(std::is_floating_point_v<T>)
    {
        return *this / this->norm();
    }

    /**
     * @brief Calculate the length of the vector. Alias to norm()
     */
    T length() const
        requires(std::is_floating_point_v<T>)
    {
        return this->norm();
    }

    /**
     * @brief Calculate the magnitude of the vector. Alias to norm()
     */
    T magnitude() const
        requires(std::is_floating_point_v<T>)
    {
        return this->norm();
    }

    /**
     * @brief Calculate the normalized direction of the vector. Alias to
     * normalized()
     */
    T direction() const
        requires(std::is_floating_point_v<T>)
    {
        return this->normalized();
    }

    /**
     * @brief Project this vector onto other vector and calculate the resulting
     * projection vector. Alias to this->dot(other.normalized()).
     *
     * See:
     * https://www.youtube.com/watch?v=LyGKycYT2v0 (3blue1brown)
     *
     * @note Can pre-normalize vector and set second parameter to true.
     */
    Vec<N, T> project_onto(const Vec other,
                           const bool is_normalized = false) const
        requires(std::is_floating_point_v<T>)
    {
        if (is_normalized) {
            return this->dot(other);
        }
        else {
            return this->dot(other.normalized());
        }
    }

    /**
     * @brief Using projection, reflect this vector accross other vector,
     * assuming the tail of this, other, and the reflected vector are the same.
     *
     * @note Can pre-normalize vector and set second parameter to true.
     */
    Vec<N, T> reflect(const Vec other, const bool is_normalized = false) const
        requires(std::is_floating_point_v<T>)
    {
        const auto projected_vector = this->project_onto(other, is_normalized);
        const auto perpendicular_component = projected_vector.vector_to(*this);

        return *this - perpendicular_component - perpendicular_component;
    }

    /**
     * @brief Take the cross product with another vector.
     *
     * See:
     * https://www.youtube.com/watch?v=eu6i7WJeinw (3blue1brown)
     */
    Vec<N, T> cross(const Vec<N, T> &rhs) const
        requires(N == 3)
    {
        const auto X = this->y * rhs.z - this->z * rhs.y;
        const auto Y = this->z * rhs.x - this->x * rhs.z;
        const auto Z = this->x * rhs.y - this->y * rhs.x;

        return Vec<N, T>(X, Y, Z);
    }

    /**
     * @brief Take the cross product with another vector. Interpret 2d
     * vectors as 3d vectors with 0 as z-component, and return the value
     * of resultant z-component.
     */
    T cross(const Vec<N, T> &rhs) const
        requires(N == 2)
    {
        const auto Z = this->x * rhs.y - this->y * rhs.x;
        return Z;
    }

    /**
     * @brief Calculate the signed angle ranging from -pi and pi
     */
    T angle(const Vec<N, T> &rhs,
            const Vec<N, T> &up = Vec<N, T>{zero(), zero(), one()},
            const bool up_is_normalized = true) const
        requires(N == 3 && std::is_floating_point_v<T>)
    {
        if (up_is_normalized) {
            return std::atan2(this->cross(rhs).dot(up), this->dot(rhs));
        }
        else {
            return std::atan2(this->cross(rhs).dot(up.normalized()),
                              this->dot(rhs));
        }
    }

    /**
     * @brief Calculate the signed angle ranging from -pi and pi
     */
    T angle(const Vec<N, T> &rhs) const
        requires(N == 2 && std::is_floating_point_v<T>)
    {
        return std::atan2(this->cross(rhs), this->dot(rhs));
    }

    /**
     * @brief Linearly interpolate the values of two vectors with a parameter
     * value with t ranging from 0 to 1. Is implemented so when t == 1, returns
     * b.
     *
     * See:
     * https://www.youtube.com/watch?v=NzjF1pdlK7Y (Freya Holmer's talk)
     */
    friend Vec lerp(const Vec a, const Vec b, const T &t)
        requires(std::is_floating_point_v<T>)
    {
        return a * (one() - t) + b * t;
    }
};
} // namespace asciirast::math
