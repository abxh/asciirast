/**
 * @file Vec.h
 * @brief File with definition of the math vector class
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <ostream>
#include <type_traits>

#include "VecBase.h"
#include "non_narrowing.h"

namespace asciirast::math {

/**
 * @brief Trait to check if vector can constructed from arguments.
 *
 * @tparam N    Number of components in the vector
 * @tparam T    Type of components
 * @tparam Args Arguments consisting of values, swizzled components, and
 * vectors.
 */
template <std::size_t N, typename T, typename... Args>
struct vec_constructible_from;

/**
 * @brief Helper class to initialize vector from arguments.
 *
 * @tparam N    Number of components in the vector
 * @tparam T    Type of components
 */
template <std::size_t N, typename T>
struct vec_initializer;

/**
 * @brief Math vector class
 *
 * @tparam N    Number of components in the vector
 * @tparam T    Type of components
 */
template <std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
class Vec : public VecBase<Vec, N, T> {
protected:
    using VecBase<Vec, N, T>::m_components;

public:
    /**
     * @brief Implicitly construct vector from swizzled components.
     */
    template <std::size_t M, std::size_t... Is>
        requires(M > 1)
    Vec(const Swizzled<Vec, M, T, Is...>& swizzled) {
        for (auto [x, y] : std::views::zip(this->range(), swizzled.range())) {
            x = y;
        }
    }

    /**
     * @brief Construct default vector with all zeroes
     */
    explicit constexpr Vec() {
        for (auto x : this->range()) {
            x = T{0};
        }
    }

    /**
     * @brief Construct vector with default value for all components.
     */
    template <typename U>
        requires(non_narrowing_v<U, T>)
    explicit Vec(const U initial_value) {
        for (auto x : this->range()) {
            x = T{initial_value};
        }
    };

    /**
     * @brief Construct vector from a mix of values and (smaller) vectors,
     * padding the rest of the vector with zeroes.
     */
    template <typename... Args>
        requires(sizeof...(Args) > 0)
    explicit Vec(const Args&... args)
        requires(vec_constructible_from<N, T, Args...>::value)
    {
        vec_initializer<N, T>::init_from(*this, args...);
    };

    /**
     * @brief Construct vector from a larger truncated vector.
     */
    template <std::size_t M>
        requires(M > N)
    explicit Vec(const Vec<M, T>& that) {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x = y;
        }
    }

    /**
     * @brief Construct vector from input range.
     */
    explicit Vec(std::ranges::input_range auto it) {
        for (auto x : this->range()) {
            x = T{0};
        }
        for (auto [x, y] : std::views::zip(this->range(), it)) {
            x = y;
        }
    }

public:
    /**
     * @brief Get number of components
     */
    static constexpr std::size_t size() { return N; }

    /**
     * @brief Get pointer over underlying data
     */
    T* data() { return &m_components[0]; }

    /**
     * @brief Index the vector.
     */
    T& operator[](const std::size_t i) {
        assert(i < this->size() && "index is inside bounds");
        return m_components[i];
    }

    /**
     * @brief Index the vector.
     */
    T operator[](const std::size_t i) const {
        assert(i < this->size() && "index is inside bounds");
        return m_components[i];
    }

    /**
     * @brief Get range over vector components.
     */
    std::ranges::view auto range() {
        return std::ranges::views::all(m_components);
    }

    /**
     * @brief Get range over vector components.
     */
    std::ranges::view auto range() const {
        return std::ranges::views::all(m_components);
    }

    /**
     * @brief Print the vector
     */
    friend std::ostream& operator<<(std::ostream& out, const Vec& v) {
        auto view = v.range();
        out << "[";
        for (auto it = view.begin(); it != view.end(); ++it) {
            out << *it;
            if (std::next(it) != view.end()) {
                out << ", ";
            }
        }
        out << "]" << std::endl;
        return out;
    }

public:
    /**
     * @brief Compare lexigraphically with other vector
     */
    friend bool operator<(const Vec& lhs, const Vec& rhs) {
        return std::ranges::lexicographical_compare(lhs.range(), rhs.range());
    }

    /**
     * @brief Compare lexigraphically with other vector
     */
    friend bool operator>(const Vec& lhs, const Vec& rhs) {
        return std::ranges::lexicographical_compare(rhs.range(), lhs.range());
    }

    /**
     * @brief Compare lexigraphically with other vector
     */
    friend bool operator<=(const Vec& lhs, const Vec& rhs) {
        return !(lhs > rhs);
    }

    /**
     * @brief Compare lexigraphically with other vector
     */
    friend bool operator>=(const Vec& lhs, const Vec& rhs) {
        return !(lhs < rhs);
    }

    /**
     * @brief Check if equal to another vector in terms of bitwise equality.
     */
    friend bool operator==(const Vec& lhs, const Vec& rhs) {
        return std::ranges::equal(lhs.range(), rhs.range());
    }

    /**
     * @brief Check if equal to floating type vector, given a precision for ulps
     * (units in last place).
     *
     * The lower, the more precise --- desirable for small floats.
     * The higher, the less precise --- desirable for large floats
     *
     * Based on:
     * https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
     */
    friend bool almost_equals(const Vec& lhs,
                              const Vec& rhs,
                              const unsigned ulps)
        requires(std::is_floating_point_v<T>)
    {
        const T n = static_cast<T>(ulps);
        auto pred = [=](const T x, const T y) -> bool {
            auto m = std::min(std::fabs(x), std::fabs(y));

            auto exp = m < std::numeric_limits<T>::min()
                               ? std::numeric_limits<T>::min_exponent - 1
                               : std::ilogb(m);

            auto epsilon = std::numeric_limits<T>::epsilon();
            auto lhs = std::fabs(x - y);
            auto rhs = n * std::ldexp(epsilon, exp);

            return lhs <= rhs;
        };
        return std::ranges::equal(lhs.range(), rhs.range(), pred);
    }

public:
    /**
     * @brief In-place component-wise addition with vector
     */
    Vec& operator+=(const Vec& that) {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x += y;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with vector
     */
    Vec& operator-=(const Vec& that) {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x -= y;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with vector
     */
    Vec& operator*=(const Vec& that) {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x *= y;
        }
        return *this;
    }

    /**
     * @brief In-place vector-scalar multiplication
     */
    template <typename U>
        requires(non_narrowing_v<U, T>)
    Vec& operator*=(const U scalar) {
        for (auto x : this->range()) {
            x *= T{scalar};
        }
        return *this;
    }

    /**
     * @brief In-place vector-scalar division
     */
    template <typename U>
        requires(non_narrowing_v<U, T>)
    Vec& operator/=(const U scalar) {
        assert(T{scalar} != T{0} && "non-zero division");
        for (auto x : this->range()) {
            x /= T{scalar};
        }
        return *this;
    }

public:
    /**
     * @brief Vector-vector component-wise addition
     */
    friend Vec operator+(const Vec& lhs, const Vec& rhs) {
        return Vec{std::views::zip_transform(std::plus(), lhs.range(),
                                             rhs.range())};
    }

    /**
     * @brief Vector-vector component-wise subtraction
     */
    friend Vec operator-(const Vec& lhs, const Vec& rhs) {
        return Vec{std::views::zip_transform(std::minus(), lhs.range(),
                                             rhs.range())};
    }

    /**
     * @brief Vector-vector component-wise multiplication
     */
    friend Vec operator*(const Vec& lhs, const Vec& rhs) {
        return Vec{std::views::zip_transform(std::multiplies(), lhs.range(),
                                             rhs.range())};
    }

    /**
     * @brief Scalar-vector multiplication
     */
    template <typename U>
        requires(non_narrowing_v<U, T>)
    friend Vec operator*(const U scalar, const Vec& rhs) {
        return Vec{std::views::transform(
                rhs.range(), [=](const T x) { return T{scalar} * x; })};
    }

    /**
     * @brief Vector-scalar multiplication
     */
    template <typename U>
        requires(non_narrowing_v<U, T>)
    friend Vec operator*(const Vec& lhs, const U scalar) {
        return Vec{std::views::transform(
                lhs.range(), [=](const T x) { return x * T{scalar}; })};
    }

    /**
     * @brief Vector-scalar division
     */
    template <typename U>
        requires(non_narrowing_v<U, T>)
    friend Vec operator/(const Vec& lhs, const U scalar) {
        assert(T{scalar} != T{0} && "non-zero division");
        return Vec{std::views::transform(
                lhs.range(), [=](const T x) { return x / T{scalar}; })};
    }

public:
    /**
     * @brief Get a vector from this pointing to that. Alias to
     * that - (*this).
     */
    Vec vector_to(const Vec& that) const { return that - (*this); }

    /**
     * @brief Calculate the norm of the vector.
     */
    T norm() const
        requires(std::is_floating_point_v<T>)
    {
        return std::sqrt(dot(*this, *this));
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
     * @brief Calculate the length of the vector. Alias to .norm()
     */
    T length() const
        requires(std::is_floating_point_v<T>)
    {
        return this->norm();
    }

    /**
     * @brief Calculate the magnitude of the vector. Alias to .norm()
     */
    T magnitude() const
        requires(std::is_floating_point_v<T>)
    {
        return this->norm();
    }

    /**
     * @brief Calculate the normalized direction of the vector. Alias to
     * .normalized()
     */
    Vec direction() const
        requires(std::is_floating_point_v<T>)
    {
        return this->normalized();
    }

    /**
     * @brief Project this vector onto other vector and calculate the resulting
     * projection vector. Alias to this->dot(that.normalized()).
     *
     * To pique your interest:
     * https://www.youtube.com/watch?v=LyGKycYT2v0 (3blue1brown)
     *
     * @note Can pre-normalize vector and set second parameter to true.
     */
    Vec project_onto(const Vec& that, const bool is_normalized = false) const
        requires(std::is_floating_point_v<T>)
    {
        if (is_normalized) {
            return dot(*this, that) * (*this);
        } else {
            return dot(*this, that.normalized()) * (*this);
        }
    }

    /**
     * @brief Using projection, reflect this vector across other vector.
     *
     * Assuming the tail of this, that, and the reflected vector are the same.
     *
     * @note Can pre-normalize vector and set second parameter to true.
     */
    Vec reflect(const Vec& that, const bool is_normalized = false) const
        requires(std::is_floating_point_v<T>)
    {
        const Vec projected_vector = this->project_onto(that, is_normalized);
        const Vec perpendicular_component = projected_vector.vector_to(*this);

        return *this - perpendicular_component - perpendicular_component;
    }

public:
    /**
     * @brief Vector dot product
     *
     * To pique your interest:
     * https://www.youtube.com/watch?v=NzjF1pdlK7Y (3blue1brown)
     */
    friend T dot(const Vec& lhs, const Vec& rhs) {
        auto view = std::ranges::views::zip_transform(std::multiplies(),
                                                      lhs.range(), rhs.range());
        return std::ranges::fold_left(view, T{0}, std::plus());
    }

    /**
     * @brief Vector cross product
     *
     * Interpret 2d vectors as 3d vectors with 0 as z-component, and return the
     * value of resultant z-component.
     */
    friend T cross(const Vec& lhs, const Vec& rhs)
        requires(N == 2)
    {
        const T z = lhs.x * rhs.y - lhs.x * rhs.y;
        return z;
    }

    /**
     * @brief Vector cross product
     */
    friend Vec cross(const Vec& lhs, const Vec& rhs)
        requires(N == 3)
    {
        /*
            Quick verification:
            det([[lhs.x, [rhs.x, [x,        det([[lhs.x, [rhs.x,
                  lhs.y,  rhs.y,  y,  = (+)       lhs.y], rhs.y]])
                  lhs.z], rhs.z], z]]
                                        (-) det([[lhs.x, [rhs.x,
                                                  lhs.z], rhs.z]])

                                        (+) det([[lhs.x, [rhs.x,
                                                  lhs.y]  rhs.y]])
        */
        const T x = lhs.x * rhs.y - lhs.y * rhs.x;
        const T y = lhs.z * rhs.x - lhs.x * rhs.z;
        const T z = lhs.x * rhs.y - lhs.x * rhs.y;

        return Vec{x, y, z};
    }

    /**
     * @brief Vector signed angle ranging from -pi and pi
     */
    friend T angle(const Vec& lhs, const Vec& rhs)
        requires(N == 2 && std::is_floating_point_v<T>)
    {
        return std::atan2(cross(lhs, rhs), dot(lhs, rhs));
    }

    /**
     * @brief Vector signed angle ranging from -pi and pi
     */
    friend T angle(const Vec& lhs,
                   const Vec& rhs,
                   const Vec& up,
                   const bool up_is_normalized)
        requires(N == 3 && std::is_floating_point_v<T>)
    {
        if (up_is_normalized) {
            return std::atan2(dot(cross(lhs, rhs), up), dot(lhs, rhs));
        } else {
            return std::atan2(dot(cross(lhs, rhs), up.normalized()),
                              dot(lhs, rhs));
        }
    }

    /**
     * @brief Linearly interpolate the values of two vectors with a parameter
     * value with t ranging from 0 to 1.
     *
     * To pique your interest:
     * https://www.youtube.com/watch?v=NzjF1pdlK7Y (Freya Holmer's talk)
     */
    static Vec lerp(const Vec& a, const Vec& b, const T t)
        requires(std::is_floating_point_v<T>)
    {
        const T weight_a = T{1} - t;
        const T weight_b = t;

        return a * weight_a + b * weight_b;
    }
};

template <typename T>
    requires(std::is_arithmetic_v<T>)
class Vec<1, T> : public VecBase<Vec, 1, T> {
protected:
    using VecBase<Vec, 1, T>::m_components;

public:
    /**
     * @brief Explicitly construct 1d-vector with value
     */
    template <typename U>
        requires(non_narrowing_v<U, T>)
    explicit Vec(const U initial_value) {
        m_components[0] = T{initial_value};
    };

    /**
     * @brief Implicitly convert to number
     */
    operator T() { return m_components[0]; }
};

/**
 * @brief Vector information trait
 */
template <typename TT>
struct vec_info {
    static constexpr bool value = false;    ///< whether the type is vector like
    static constexpr std::size_t size = 0;  ///< vector size
};

/**
 * @brief Vector information trait
 */
template <std::size_t M, typename U>
struct vec_info<Vec<M, U>> {
    static constexpr bool value = true;     ///< whether the type is vector like
    static constexpr std::size_t size = M;  ///< vector size
};

/**
 * @brief Vector information trait
 */
template <std::size_t M, typename T, std::size_t... Is>
    requires(sizeof...(Is) > 1)
struct vec_info<Swizzled<Vec<sizeof...(Is), T>, M, T, Is...>> {
    static constexpr bool value = true;  ///< whether the type is vector like
    static constexpr std::size_t size = sizeof...(Is);  ///< vector size
};

template <std::size_t N, typename T, typename... Args>
struct vec_constructible_from {
private:
    static constexpr bool accepted_types =
            ((non_narrowing_v<Args, T> || vec_info<Args>::value) && ...);
    static constexpr std::size_t num_values =
            ((non_narrowing_v<Args, T> ? 1 : 0) + ...);
    static constexpr bool total_size_in_bounds =
            (N >= num_values + (vec_info<Args>::size + ...));

public:
    static constexpr bool value =
            accepted_types &&
            total_size_in_bounds;  ///< whether the arguments are of acceptable
                                   ///< types and total size is inside bounds
};

template <std::size_t N, typename T>
struct vec_initializer {
public:
    /**
     * @brief Initialize vector from arguments
     *
     * @param out   The vector to initialize
     * @param args  The arguments
     */
    template <typename... Args>
    static constexpr void init_from(Vec<N, T>& out, const Args&... args)
        requires(vec_constructible_from<N, T, Args...>::value)
    {
        std::size_t idx = 0;
        init_from_inner(idx, out, args...);
        for (auto i : std::views::iota(idx, N)) {
            out[i] = T{0};
        }
    }

private:
    static constexpr void init_from_inner(std::size_t& idx, Vec<N, T>& out) {
        (void)(idx);
        (void)(out);
    }

    template <typename U>
        requires(non_narrowing_v<U, T>)
    static constexpr void init_from_inner(std::size_t& idx,
                                          Vec<N, T>& out,
                                          const U& arg,
                                          const auto&... rest) {
        out[idx] = T{arg};
        idx += 1;
        init_from_inner(idx, out, rest...);
    }

    template <std::size_t M>
    static constexpr void init_from_inner(std::size_t& idx,
                                          Vec<N, T>& out,
                                          const Vec<M, T>& arg,
                                          const auto&... rest) {
        for (auto [i, j] : std::views::zip(std::views::iota(idx, idx + M),
                                           std::views::iota(0U, M))) {
            out[i] = arg[j];
        }
        idx += M;
        init_from_inner(idx, out, rest...);
    }

    template <std::size_t M, std::size_t... Is>
        requires(sizeof...(Is) > 1)
    static constexpr void init_from_inner(
            std::size_t& idx,
            Vec<N, T>& out,
            const Swizzled<Vec<sizeof...(Is), T>, M, T, Is...>& arg,
            const auto&... rest) {
        auto values = arg.range();
        for (auto [i, value] : std::views::zip(std::views::iota(idx), values)) {
            out[i] = value;
        }
        idx += sizeof...(Is);
        init_from_inner(idx, out, rest...);
    }
};

}  // namespace asciirast::math
