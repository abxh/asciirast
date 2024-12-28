/**
 * @file Vec.h
 * @brief Math vector
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <ostream>
#include <stdexcept>

#include "VecBase.h"

namespace asciirast::math {

template <size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
class Vec;

/**
 * @brief Vector information trait
 */
template <typename TT>
struct vec_info {
    using value_type = void;
    static constexpr bool is_vec = false;
    static constexpr size_t size = 1;
};

/**
 * @brief Vector information trait
 */
template <size_t M, typename T>
struct vec_info<Vec<M, T>> {
    using value_type = T;
    static constexpr bool is_vec = true;
    static constexpr size_t size = M;
};

/**
 * @brief Is convertible vector trait.
 * E.g. Vec<2, float> is convertible to Vec<2, double>.
 */
template <typename TT, typename U>
constexpr bool is_conv_vec_v =
        vec_info<TT>::is_vec &&
        non_narrowing_conv<typename vec_info<TT>::value_type, U>;

/**
 * @brief math vector
 */
template <size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
class Vec : public VecBase<N, T> {
public:
    /**
     * @brief Value type
     */
    using value_type = T;

    /**
     * @brief Number of components
     */
    static constexpr size_t size() { return N; }

private:
    void set_components_from_args(size_t& idx, Vec& out) {};

    template <typename... Args, typename U>
        requires(!vec_info<U>::is_vec)
    void set_components_from_args(size_t& idx,
                                  Vec& out,
                                  const U& value,
                                  Args... args) {
        out.m_components[idx++] = T{value};
        set_components_from_args(idx, out, args...);
    }

    template <size_t M, typename U, typename... Args>
    void set_components_from_args(size_t& idx,
                                  Vec& out,
                                  const Vec<M, U>& vector,
                                  Args... args) {
        for (size_t i = 0; i < M; i++) {
            out.m_components[idx++] = T{vector.m_components[i]};
        }
        set_components_from_args(idx, out, args...);
    }

public:
    using VecBase<N, T>::m_components;

    /**
     * @brief Default constructor. Set all values to 0
     */
    Vec() {
        for (size_t i = 0; i < N; i++) {
            m_components[i] = T{0};
        }
    }

    /**
     * @brief Set all values to initial value
     */
    template <typename U>
        requires(non_narrowing_conv<U, T>)
    explicit Vec(const U& initial_value) {
        for (size_t i = 0; i < N; i++) {
            m_components[i] = T{initial_value};
        }
    };

    /**
     * @brief Set vector from a mix of values and vectors. Rest of the vector is
     * filled with zero.
     */
    template <typename... Args>
        requires((non_narrowing_conv<Args, T> || is_conv_vec_v<Args, T>) && ...)
    explicit Vec(const Args&... args)
        requires((vec_info<Args>::size + ...) <= N)
    {
        size_t idx = 0;
        set_components_from_args(idx, *this, args...);
        for (size_t i = idx; i < N; i++) {
            m_components[i] = T{0};
        }
    };

    /**
     * @brief Truncate larger vector and make into into a smaller vector.
     */
    template <std::size_t M, typename U>
        requires(M > N && non_narrowing_conv<U, T>)
    explicit Vec(const Vec<M, U>& larger_vec) {
        for (size_t i = 0; i < N; i++) {
            m_components[i] = larger_vec.m_components[i];
        }
    }

    /**
     * @brief Set vector using iterator. Iterator must point to N elements.
     */
    template <std::input_iterator Iterator>
        requires(std::same_as<std::iter_value_t<Iterator>, T>)
    explicit Vec(Iterator begin) {
        std::copy_n(begin, this->size(), this->begin());
    };

public:
    /**
     * @name Basic iterator support
     * @{
     * Exposing the std::array iterator
     */
    using iterator = typename std::array<T, N>::iterator;
    using const_iterator = typename std::array<T, N>::const_iterator;
    iterator begin() { return m_components.begin(); }
    iterator end() { return m_components.end(); }
    const_iterator begin() const { return m_components.begin(); }
    const_iterator end() const { return m_components.end(); }
    ///@}

    /**
     * @brief Print the vector
     */
    friend std::ostream& operator<<(std::ostream& out, const Vec& v) {
        auto it_end = v.end();
        out << "[";
        for (auto it = v.begin(); it != it_end; ++it) {
            out << *it;
            if ((it + 1) != it_end) {
                out << ", ";
            }
        }
        out << "]" << std::endl;
        return out;
    }

public:
    /**
     * @brief Index the vector with bounds checking
     * @throws std::out_of_range if index is out of bounds.
     */
    T& operator[](size_t i) {
        if (i >= N) {
            throw std::out_of_range("asciirast::math::Vec::operator[]");
        }
        return m_components[i];
    }

    /**
     * @brief Index the vector with bounds checking
     * @throws std::out_of_range if index is out of bounds.
     */
    const T& operator[](size_t i) const {
        if (i >= N) {
            throw std::out_of_range("asciirast::math::Vec::operator[]");
        }
        return m_components[i];
    }

    /**
     * @brief Check if exactly equal to another vector in terms of bitwise
     * equality.
     */
    bool equals_bitwise(const Vec& other) const {
        return std::equal(this->begin(), this->end(), other.begin(),
                          other.end());
    }

    /**
     * @brief Check if equal to integral type vector
     */
    friend bool operator==(const Vec& lhs, const Vec& rhs)
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
    bool equals_almost(const Vec& other, const unsigned ulps) const
        requires(std::is_floating_point_v<T>)
    {
        const auto n = static_cast<T>(ulps);
        return std::equal(
                this->begin(), this->end(), other.begin(), other.end(),
                [&n](const T& x, const T& y) {
                    T m = std::min(std::fabs(x), std::fabs(y));

                    int exp = m < std::numeric_limits<T>::min()
                                      ? std::numeric_limits<T>::min_exponent - 1
                                      : std::ilogb(m);

                    auto epsilon = std::numeric_limits<T>::epsilon();
                    auto lhs = std::fabs(x - y);
                    auto rhs = n * std::ldexp(epsilon, exp);

                    return lhs <= rhs;
                });
    }

    /**
     * @brief Add with another vector
     */
    const Vec& operator+=(const Vec& rvalue) {
        std::transform(this->begin(), this->end(), rvalue.begin(),
                       this->begin(), std::plus());
        return *this;
    }

    /**
     * @brief Perform vector addition
     */
    friend Vec operator+(const Vec& lhs, const Vec& rhs) {
        Vec res{};
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), res.begin(),
                       std::plus());
        return res;
    }

    /**
     * @brief Subtract with another vector
     */
    const Vec& operator-=(const Vec& rvalue) {
        std::transform(this->begin(), this->end(), rvalue.begin(),
                       this->begin(), std::minus());
        return *this;
    }

    /**
     * @brief Perform vector subtraction
     */
    friend Vec operator-(const Vec& lhs, const Vec& rhs) {
        Vec res{};
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), res.begin(),
                       std::minus());
        return res;
    }

    /**
     * @brief Element-wise multiply with another vector
     */
    const Vec& operator*=(const Vec& rvalue) {
        std::transform(this->begin(), this->end(), rvalue.begin(),
                       this->begin(), std::multiplies());
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
                       std::multiplies());
        return res;
    }

    /**
     * @brief Multiply vector with scalar from left-hand-side
     */
    template <typename U>
        requires(non_narrowing_conv<U, T>)
    friend Vec operator*(const U& scalar, const Vec& vec) {
        Vec res{};
        const T& scalar_{scalar};
        std::transform(vec.begin(), vec.end(), res.begin(),
                       [&](const T& x) { return scalar_ * x; });
        return res;
    }

    /**
     * @brief Multiply vector with scalar from right-hand-side
     */
    template <typename U>
        requires(non_narrowing_conv<U, T>)
    Vec& operator*=(const U& scalar) {
        const T& scalar_{scalar};
        std::transform(this->begin(), this->end(), this->begin(),
                       [&](const T& x) { return x * scalar_; });
        return *this;
    }

    /**
     * @brief Multiply vector with scalar from right-hand-side
     */
    template <typename U>
        requires(non_narrowing_conv<U, T>)
    friend Vec operator*(const Vec& vec, const U& scalar) {
        Vec res{};
        const T& scalar_{scalar};
        std::transform(vec.begin(), vec.end(), res.begin(),
                       [&](const T& x) { return x * scalar_; });
        return res;
    }

    /**
     * @brief Multiply vector with inverse scalar from right-hand-side
     */
    template <typename U>
        requires(non_narrowing_conv<U, T>)
    Vec& operator/=(const U& scalar) {
        const T& scalar_{scalar};
        std::transform(this->begin(), this->end(), this->begin(),
                       [&](const T& x) { return x / scalar_; });
        return *this;
    }

    /**
     * @brief Multiply vector with inverse scalar from right-hand-side
     */
    template <typename U>
        requires(non_narrowing_conv<U, T>)
    friend Vec operator/(const Vec& vec, const U& scalar)
    {
        Vec res{};
        const T& scalar_{scalar};
        std::transform(vec.begin(), vec.end(), res.begin(),
                       [&](const T& x) { return x / scalar_; });
        return res;
    }

    /**
     * @brief Get a vector from this pointing to other. Alias to
     * operator-(other, *this)
     */
    Vec vector_to(const Vec& other) const {
        return this->operator-(other, *this);
    }

    /**
     * @brief Take the dot product with another vector
     *
     * See:
     * https://www.youtube.com/watch?v=NzjF1pdlK7Y (3blue1brown)
     */
    T dot(const Vec& rhs) const {
        return std::transform_reduce(this->begin(), this->end(), rhs.begin(),
                                     T{0}, std::plus(), std::multiplies());
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
    Vec normalized() const
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
    Vec project_onto(const Vec other, const bool is_normalized = false) const
        requires(std::is_floating_point_v<T>)
    {
        if (is_normalized) {
            return this->dot(other);
        } else {
            return this->dot(other.normalized());
        }
    }

    /**
     * @brief Using projection, reflect this vector accross other vector,
     * assuming the tail of this, other, and the reflected vector are the same.
     *
     * @note Can pre-normalize vector and set second parameter to true.
     */
    Vec reflect(const Vec other, const bool is_normalized = false) const
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
    Vec cross(const Vec& rhs) const
        requires(N == 3)
    {
        const auto X = this->y * rhs.z - this->z * rhs.y;
        const auto Y = this->z * rhs.x - this->x * rhs.z;
        const auto Z = this->x * rhs.y - this->y * rhs.x;

        return Vec{X, Y, Z};
    }

    /**
     * @brief Take the cross product with another vector. Interpret 2d
     * vectors as 3d vectors with 0 as z-component, and return the value
     * of resultant z-component.
     */
    T cross(const Vec& rhs) const
        requires(N == 2)
    {
        const auto Z = this->x * rhs.y - this->y * rhs.x;
        return Z;
    }

    /**
     * @brief Calculate the signed angle ranging from -pi and pi (3D)
     */
    T angle(const Vec& rhs) const
        requires(N == 3 && std::is_floating_point_v<T>)
    {
        const Vec up = Vec{0, 0, 1};
        const bool up_is_normalized = true;

        if (up_is_normalized) {
            return std::atan2(this->cross(rhs).dot(up), this->dot(rhs));
        } else {
            return std::atan2(this->cross(rhs).dot(up.normalized()),
                              this->dot(rhs));
        }
    }

    /**
     * @brief Calculate the signed angle ranging from -pi and pi (3D)
     */
    T angle(const Vec& rhs, const Vec& up, const bool up_is_normalized) const
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
     * @brief Calculate the signed angle ranging from -pi and pi (2D)
     */
    T angle(const Vec& rhs) const
        requires(N == 2 && std::is_floating_point_v<T>)
    {
        return std::atan2(this->cross(rhs), this->dot(rhs));
    }

    /**
     * @brief Linearly interpolate the values of two vectors with a parameter
     * value with t ranging from 0 to 1.
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
