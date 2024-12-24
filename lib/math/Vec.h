/**
 * @file Vec.h
 * @brief Math vector
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <ranges>
#include <type_traits>

#include "VecBase.h"

namespace asciirast::math {

/**
 * @brief Trait to check narrowing conversion
 */
template <typename To, typename... From>
concept non_narrowing_conversion = (requires(From f) { To{f}; } && ...);

/**
 * @brief N-dimensional math vector
 */
template <std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
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
    requires(N > 0 && std::is_arithmetic_v<T>)
class Vec : public VecBase<Vec, N, T> {
public:
    /**
     * @brief Initiate vector from initial value
     */
    template <typename U>
        requires(non_narrowing_conversion<T, U>)
    constexpr static void init(Vec& vec, const U& initial_value) {
        std::fill(vec.begin(), vec.end(), initial_value);
    }

    /**
     * @brief Initiate vector from list of values. Fill the rest with zero.
     */
    template <typename... Us>
        requires(non_narrowing_conversion<T, Us...>)
    constexpr static void init(Vec& vec, const Us&... values)
        requires(1 < sizeof...(values) && sizeof...(values) <= N)
    {
        const auto value_list = {values...};
        const auto indicies = std::views::iota(0);

        for (auto [i, value] : std::views::zip(indicies, value_list)) {
            vec.m_components[i] = value;
        }
    }

    /**
     * @brief Initiate vector from another (potentially smaller) vector.
     */
    template <std::size_t M>
    constexpr static void init(Vec& vec, const Vec<M, T>& other)
        requires(M <= N)
    {
        std::copy(other.begin(), other.end(), vec.begin());
    }

public:
    using VecBase<Vec, N, T>::m_components;
    using value_type = T;

    constexpr auto size() const { return N; }

    /**
     * @brief Default constructor. Fill the values as zero.
     */
    constexpr Vec() { Vec::init(*this, T{0}); };

    /**
     * @brief Use initial value to fill the entire vector.
     */
    template <typename U>
        requires(non_narrowing_conversion<T, U>)
    constexpr explicit Vec(const U& initial_value) {
        Vec::init(*this, initial_value);
    };

    /**
     * @brief Initiate a 2-dimensional vector
     */
    template <typename U1, typename U2>
        requires(non_narrowing_conversion<T, U1, U2>)
    constexpr Vec(const U1& x, const U2& y)
        requires(N == 2)
    {
        Vec::init(*this, x, y);
    };

    /**
     * @brief Initiate a 3-dimensional vector
     */
    template <typename U1, typename U2, typename U3>
        requires(non_narrowing_conversion<T, U1, U2, U3>)
    constexpr Vec(const U1& x, const U2& y, const U3& z)
        requires(N == 3)
    {
        Vec::init(*this, x, y, z);
    };

    /**
     * @brief Initiate a N-dimensional vector with a M-dimensional vector where
     * M < N. The rest of the values are set to 0.
     */
    template <std::size_t M>
    constexpr Vec(const Vec<M, T>& that)
        requires(M < N)
    {
        Vec::init(*this, that);
    };

    /**
     * @brief Initiate a 3-dimensional vector with a value and a 2-dimensional
     * vector
     */
    template <typename U>
        requires(non_narrowing_conversion<T, U>)
    constexpr Vec(const U& l, const Vec<2, T>& r)
        requires(N == 3)
    {
        const auto x = l;
        const auto [y, z] = r.m_components;
        Vec::init(*this, x, y, z);
    };

    /**
     * @brief Initiate a 3-dimensional vector with a 2-dimensional vector and
     * value
     */
    template <typename U>
        requires(non_narrowing_conversion<T, U>)
    constexpr Vec(const Vec<2, T>& l, const U& r)
        requires(N == 3)
    {
        const auto [x, y] = l.m_components;
        const auto z = r;
        Vec::init(*this, x, y, z);
    };

    /**
     * @brief Initiate a 4-dimensional vector
     */
    template <typename U1, typename U2, typename U3, typename U4>
        requires(non_narrowing_conversion<T, U1, U2, U3, U4>)
    constexpr Vec(const U1& x, const U2& y, const U3& z, const U4& w)
        requires(N == 4)
    {
        Vec::init(*this, x, y, z, w);
    };

    /**
     * @brief Initiate a 4-dimensional vector with two 2-dimensional vectors
     */
    template <typename U>
        requires(non_narrowing_conversion<T, U>)
    constexpr Vec(const U& l, const Vec<3, T>& r)
        requires(N == 4)
    {
        const auto x = l;
        const auto [y, z, w] = r.m_components;
        Vec::init(*this, x, y, z, w);
    };

    /**
     * @brief Initiate a 4-dimensional vector with a 3-dimensional vector and
     * value
     */
    template <typename U>
        requires(non_narrowing_conversion<T, U>)
    constexpr Vec(const Vec<3, T>& l, const U& r)
        requires(N == 4)
    {
        const auto [x, y, z] = l.m_components;
        const auto w = r;
        Vec::init(*this, x, y, z, w);
    };

    /**
     * @brief Initiate a 4-dimensional vector with a value and 3-dimensional
     * vector
     */
    constexpr Vec(const Vec<2, T>& l, const Vec<2, T>& r)
        requires(N == 4)
    {
        const auto [x, y] = l.m_components;
        const auto [z, w] = r.m_components;
        Vec::init(*this, x, y, z, w);
    };

    /**
     * @brief Initiate a 4-dimensional vector with two values and 2-dimensional
     * vector
     */
    template <typename U1, typename U2>
        requires(non_narrowing_conversion<T, U1, U2>)
    constexpr Vec(const U1& l, const Vec<2, T>& m, const U2& r)
        requires(N == 4)
    {
        const auto x = l;
        const auto [y, z] = m.m_components;
        const auto w = r;
        Vec::init(*this, x, y, z, w);
    };

    /**
     * @brief Initiate a N-dimensional vector
     */
    template <typename... Us>
        requires(non_narrowing_conversion<T, Us...>)
    constexpr Vec(const Us&... values)
        requires(4 < N && N == sizeof...(values))
    {
        Vec::init(*this, values...);
    };

    /**
     * @brief Use iterator to fill the vector.
     */
    template <std::input_iterator Iterator>
        requires(std::same_as<std::iter_value_t<Iterator>, T>)
    constexpr Vec(Iterator begin) {
        std::copy_n(begin, N, this->begin());
    };

    /**
     * @brief Begin iterator
     */
    constexpr auto begin() { return &m_components[0]; }

    /**
     * @brief End iterator
     */
    constexpr auto end() { return &m_components[N]; }

    /**
     * @brief Begin const-iterator
     */
    constexpr auto begin() const { return const_cast<Vec&>(*this).begin(); }

    /**
     * @brief End const-iterator
     */
    constexpr auto end() const { return const_cast<Vec&>(*this).end(); }

public:
    /**
     * @brief Check if exactly equal to another vector in terms of bitwise
     * equality.
     */
    bool equals_bitwise(const Vec<N, T>& other) const {
        return std::equal(this->begin(), this->end(), other.begin(),
                          other.end());
    }

    /**
     * @brief Check if equal to integral type vector
     */
    friend bool operator==(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
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
    bool equals_almost(const Vec<N, T>& other, const unsigned ulps) const
        requires(std::is_floating_point_v<T>)
    {
        const auto n = static_cast<float>(ulps);
        return std::equal(
                this->begin(), this->end(), other.begin(), other.end(),
                [&](const T& x, const T& y) {
                    const T m = std::min(std::fabs(x), std::fabs(y));

                    const int exp =
                            m < std::numeric_limits<T>::min()
                                    ? std::numeric_limits<T>::min_exponent - 1
                                    : std::ilogb(m);

                    const auto lhs = std::fabs(x - y);
                    const auto rhs =
                            n *
                            std::ldexp(std::numeric_limits<T>::epsilon(), exp);

                    return lhs <= rhs;
                });
    }

    /**
     * @brief Add with another vector
     */
    const Vec& operator+=(const Vec& rvalue) {
        std::transform(this->begin(), this->end(), rvalue.begin(),
                       this->begin(),
                       [](const T& x, const T& y) { return x + y; });
        return *this;
    }

    /**
     * @brief Perform vector addition
     */
    friend Vec operator+(const Vec& lhs, const Vec& rhs) {
        Vec res{};
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), res.begin(),
                       [](const T& x, const T& y) { return x + y; });
        return res;
    }

    /**
     * @brief Subtract with another vector
     */
    const Vec& operator-=(const Vec& rvalue) {
        std::transform(this->begin(), this->end(), rvalue.begin(),
                       this->begin(),
                       [](const T& x, const T& y) { return x - y; });
        return *this;
    }

    /**
     * @brief Perform vector subtraction
     */
    friend Vec operator-(const Vec& lhs, const Vec& rhs) {
        Vec res{};
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), res.begin(),
                       [](const T& x, const T& y) { return x - y; });
        return res;
    }

    /**
     * @brief Element-wise multiply with another vector
     */
    const Vec& operator*=(const Vec& rvalue) {
        std::transform(this->begin(), this->end(), rvalue.begin(),
                       this->begin(),
                       [](const T& x, const T& y) { return x * y; });
        return *this;
    }

    /**
     * @brief Perform element-wise vector multiplication
     *
     * See:
     * https://www.youtube.com/watch?v=htYh-Tq7ZBI (Freya Holmer's talk)
     */
    friend Vec operator*(const Vec& lhs, const Vec& rhs) {
        Vec res{};
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), res.begin(),
                       [](const T& x, const T& y) { return x * y; });
        return res;
    }

    /**
     * @brief Multiply vector with scalar from left-hand-side
     */
    template <typename U>
        requires(non_narrowing_conversion<T, U>)
    friend Vec operator*(const U& scalar, const Vec& vec) {
        Vec res{};
        std::transform(vec.begin(), vec.end(), res.begin(),
                       [&](const T& x) { return scalar * x; });
        return res;
    }

    /**
     * @brief Multiply vector with scalar from right-hand-side
     */
    template <typename U>
        requires(non_narrowing_conversion<T, U>)
    friend Vec operator*(const Vec& vec, const U& scalar) {
        Vec res{};
        std::transform(vec.begin(), vec.end(), res.begin(),
                       [&](const T& x) { return x * scalar; });
        return res;
    }

    /**
     * @brief Multiply vector with inverse scalar from right-hand-side
     */
    template <typename U>
        requires(non_narrowing_conversion<T, U>)
    friend Vec operator/(const Vec& vec, const U& scalar)
        requires(std::is_floating_point_v<T>)
    {
        Vec res{};
        std::transform(vec.begin(), vec.end(), res.begin(),
                       [&](const T& x) { return x / scalar; });
        return res;
    }

    /**
     * @brief Get a vector from this pointing to other. Alias to
     * operator-(other, *this)
     */
    Vec<N, T> vector_to(const Vec& other) const {
        return this->operator-(other, *this);
    }

    /**
     * @brief Take the dot product with another vector
     *
     * See:
     * https://www.youtube.com/watch?v=NzjF1pdlK7Y (3blue1brown)
     */
    T dot(const Vec& rhs) const {
        return std::transform_reduce(
                this->begin(), this->end(), rhs.begin(), T{0},
                [](const auto& lterm, const auto& rterm) {
                    return lterm + rterm;
                },
                [](const auto& lcomp, const auto& rcomp) {
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
                           const bool is_normalized = true) const
        requires(std::is_floating_point_v<T>)
    {
        if (is_normalized) {
            return this->dot(other);
        } else {
            return this->dot();
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
    Vec<N, T> cross(const Vec<N, T>& rhs) const
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
    T cross(const Vec<N, T>& rhs) const
        requires(N == 2)
    {
        const auto Z = this->x * rhs.y - this->y * rhs.x;
        return Z;
    }

    /**
     * @brief Calculate the signed angle ranging from -pi and pi
     */
    T angle(const Vec<N, T>& rhs) const
        requires(N == 3 && std::is_floating_point_v<T>)
    {
        const Vec<N, T> up = Vec<N, T>{T{0}, T{0}, T{1}};
        const bool up_is_normalized = true;

        if (up_is_normalized) {
            return std::atan2(this->cross(rhs).dot(up), this->dot(rhs));
        } else {
            return std::atan2(this->cross(rhs).dot(up.normalized()),
                              this->dot(rhs));
        }
    }

    /**
     * @brief Calculate the signed angle ranging from -pi and pi
     */
    T angle(const Vec<N, T>& rhs,
            const Vec<N, T>& up,
            const bool up_is_normalized) const
        requires(N == 3 && std::is_floating_point_v<T>)
    {
        if (up_is_normalized) {
            return std::atan2(this->cross(rhs).dot(up), this->dot(rhs));
        } else {
            return std::atan2(this->cross(rhs).dot(up.normalized()),
                              this->dot(rhs));
        }
    }

    /**
     * @brief Calculate the signed angle ranging from -pi and pi
     */
    T angle(const Vec<N, T>& rhs) const
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
    friend Vec lerp(const Vec a, const Vec b, const T& t)
        requires(std::is_floating_point_v<T>)
    {
        return a * (T{1} - t) + b * t;
    }
};

}  // namespace asciirast::math
