/**
 * @file Vec.h
 * @brief File with definition of the math vector class
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <ostream>
#include <type_traits>

#include "./VecBase.h"
#include "./utils.h"

namespace asciirast::math {

namespace detail {

/// @cond DO_NOT_DOCUMENT

template<std::size_t N, typename T, typename... Args>
struct vec_constructible_from;

template<std::size_t N, typename T>
struct vec_initializer;

template<typename T, typename... Args>
struct not_a_single_value;

template<typename T, typename... Args>
struct not_a_single_convertible_value;

/// @endcond

} // namespace detail

/**
 * @brief Math vector class
 *
 * @tparam N    Number of components in the vector
 * @tparam T    Type of components
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
class Vec;

/**
 * @brief Vector dot product
 *
 * @param lhs Left hand side of operand
 * @param rhs Right hande side of operand
 * @return The resulting number
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
constexpr auto
dot(const Vec<N, T>& lhs, const Vec<N, T>& rhs) -> T
{
    const std::array<T, N>& l = lhs.array();
    const std::array<T, N>& r = rhs.array();

    return std::inner_product(l.begin(), l.end(), r.begin(), T{});
}

/**
 * @brief Vector "2D cross product"
 *
 * By calculating the signed magnitude of the plane spanned by the two
 * vectors.
 *
 * @param lhs Left hand side of operand
 * @param rhs Right hande side of operand
 * @return The resulting number
 */
template<std::size_t N, typename T>
constexpr auto
cross(const Vec<N, T>& lhs, const Vec<N, T>& rhs) -> T
    requires(N == 2)
{
    return lhs.x * rhs.y - rhs.x * lhs.y; // XY_magnitude
}

/**
 * @brief Vector 3D cross product
 *
 * @return The resulting vector
 */
template<std::size_t N, typename T>
constexpr auto
cross(const Vec<N, T>& lhs, const Vec<N, T>& rhs) -> Vec<N, T>
    requires(N == 3)
{
    /*
        det([x_hat, y_hat, z_hat],
            [lhs.x, lhs.y, lhs.z],
            [rhs.x, rhs.y, rhs.z]])
        =
            x_hat det([lhs.y, lhs.z],
                      [rhs.y, rhs.z])
          - y_hat det([lhs.x, lhs.z],
                      [rhs.x, rhs.z])
          + z_hat det([lhs.x, lhs.y],
                      [rhs.x, rhs.y])
    */

    const T x_hat = lhs.y * rhs.z - rhs.y * lhs.z; // YZ_magnitude
    const T y_hat = lhs.x * rhs.z - rhs.x * lhs.z; // XZ_magnitude
    const T z_hat = lhs.x * rhs.y - rhs.x * lhs.y; // XY_magnitude

    return Vec<N, T>{ +x_hat, -y_hat, +z_hat };
}

/**
 * @brief Vector signed angle ranging from -pi and pi radians
 *
 * @todo make this function constexpr in c++26 or use library
 *
 * @param lhs Left hand side of operand
 * @param rhs Right hande side of operand
 * @return The resulting angle
 */
template<std::size_t N, typename T>
auto
angle(const Vec<N, T>& lhs, const Vec<N, T>& rhs) -> T
    requires(N == 2 && std::is_floating_point_v<T>)
{
    /*
        atan2(x,y)        = tan^-1(y/x), with quadrant
                                         sign considerations
        theta             = angle between lhs and rhs
        cross2d(lhs, rhs) = sin(theta) |lhs| |rhs|
        dot(lhs, rhs)     = cos(theta) |lhs| |rhs|
    */
    return std::atan2(cross(lhs, rhs), dot(lhs, rhs));
}

/**
 * @brief Vector signed angle ranging from -pi and pi radians
 *
 * @param lhs Left hand side of operand
 * @param rhs Right hande side of operand
 * @param up_ The "up" direction to measure the angle with
 * @param up_is_normalized Whether the "up" vector is normalized
 * @return The resulting angle
 */
template<std::size_t N, typename T>
auto
angle(const Vec<N, T>& lhs, const Vec<N, T>& rhs, const Vec<N, T>& up_, const bool up_is_normalized) -> T
    requires(N == 3 && std::is_floating_point_v<T>)
{
    const Vec<N, T>& up = up_is_normalized ? up_ : up_.normalized();

    return std::atan2(dot(cross(lhs, rhs), up), dot(lhs, rhs));
}

/**
 * @brief Linearly interpolate the components of the two vectors with a
 *        parameter t ranging from 0 to 1.
 *
 * @param lhs Left hand side of operand
 * @param rhs Right hande side of operand
 * @param t How much as a percentage
 * @return The resulting vector
 */
template<std::size_t N, typename T>
constexpr auto
lerp(const Vec<N, T>& lhs, const Vec<N, T>& rhs, const T t) -> Vec<N, T>
    requires(std::is_floating_point_v<T>)
{
    Vec<N, T> res{};
    for (std::size_t i = 0; i < N; i++) {
        res[i] = std::lerp(lhs[i], rhs[i], t);
    }
    return res;
}

/**
 * @brief Take the max value of each component
 *
 * @param lhs Left hand side of operand
 * @param rhs Right hande side of operand
 * @return The resulting vector
 */
template<std::size_t N, typename T>
constexpr auto
max(const Vec<N, T>& lhs, const Vec<N, T>& rhs) -> Vec<N, T>
{
    Vec<N, T> res{};
    for (std::size_t i = 0; i < N; i++) {
        res[i] = std::max(lhs[i], rhs[i]);
    }
    return res;
}

/**
 * @brief Take the min value of each component
 *
 * @param lhs Left hand side of operand
 * @param rhs Right hande side of operand
 * @return The resulting vector
 */
template<std::size_t N, typename T>
constexpr auto
min(const Vec<N, T>& lhs, const Vec<N, T>& rhs) -> Vec<N, T>
{
    Vec<N, T> res{};
    for (std::size_t i = 0; i < N; i++) {
        res[i] = std::min(lhs[i], rhs[i]);
    }
    return res;
}

/**
 * @brief Clamp each component
 *
 * @param v The vector to work on
 * @param low Low values of components
 * @param high High values of components
 * @return The resulting vector
 */
template<std::size_t N, typename T>
constexpr auto
clamp(const Vec<N, T>& v, const Vec<N, T>& low, const Vec<N, T>& high) -> Vec<N, T>
{
    Vec<N, T> res{};
    for (std::size_t i = 0; i < N; i++) {
        res[i] = std::clamp(v[i], low[i], high[i]);
    }
    return res;
}

/**
 * @brief Take the absolute value of each component
 *
 * @param v The vector to work on
 * @return The resulting vector
 */
template<std::size_t N, typename T>
constexpr auto
abs(const Vec<N, T>& v) -> Vec<N, T>
{
    Vec<N, T> res{};
    for (std::size_t i = 0; i < N; i++) {
        res[i] = std::abs(v[i]);
    }
    return res;
}

/**
 * @brief Take the rounded value of each component
 *
 * @param v The vector to work on
 * @return The resulting vector
 */
template<std::size_t N, typename T>
constexpr auto
round(const Vec<N, T>& v) -> Vec<N, T>
    requires(std::is_floating_point_v<T>)
{
    Vec<N, T> res{};
    for (std::size_t i = 0; i < N; i++) {
        res[i] = std::round(v[i]);
    }
    return res;
}

/**
 * @brief Take the ceiled value of each component
 *
 * @param v The vector to work on
 * @return The resulting vector
 */
template<std::size_t N, typename T>
constexpr auto
ceil(const Vec<N, T>& v) -> Vec<N, T>
    requires(std::is_floating_point_v<T>)
{
    Vec<N, T> res{};
    for (std::size_t i = 0; i < N; i++) {
        res[i] = std::ceil(v[i]);
    }
    return res;
}

/**
 * @brief Take the ceiled value of each component
 *
 * @param v The vector to work on
 * @return The resulting vector
 */
template<std::size_t N, typename T>
constexpr auto
floor(const Vec<N, T>& v) -> Vec<N, T>
    requires(std::is_floating_point_v<T>)
{
    Vec<N, T> res{};
    for (std::size_t i = 0; i < N; i++) {
        res[i] = std::floor(v[i]);
    }
    return res;
}

/**
 * @brief Math vector class
 *
 * @tparam N    Number of components in the vector
 * @tparam T    Type of components
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
class Vec : public VecBase<Vec, N, T>
{
    template<typename... Args>
    static constexpr bool constructible_from_args_v = (detail::not_a_single_convertible_value<T, Args...>::value &&
                                                       detail::not_a_single_value<Vec, Args...>::value) &&
                                                      (detail::vec_constructible_from<N, T, Args...>::value);

protected:
    using VecBase<Vec, N, T>::m_components;

public:
    /// value type
    using value_type = T;

    /**
     * @brief Default constructor
     */
    constexpr Vec() = default;

    /**
     * @brief Default copy constructor
     */
    constexpr Vec(const Vec&) = default;

    /**
     * @brief Default move constructor
     */
    constexpr Vec(Vec&&) = default;

    /**
     * @brief Implicitly construct vector from swizzled components.
     *
     * @param that The Swizzled object
     */
    template<std::size_t M, std::size_t... Is>
        requires(M > 1)
    constexpr Vec(const Swizzled<Vec, M, T, Is...>& that)
    {
        for (std::size_t i = 0; i < N; i++) {
            (*this)[i] = that[i];
        }
    }

    /**
     * @brief Construct vector from a mix of values and (smaller) vectors,
     *        padding the rest of the vector with zeroes.
     *
     * @param args A mix of values and smaller vectors
     */
    template<typename... Args>
        requires(sizeof...(Args) > 0)
    constexpr Vec(Args&&... args)
        requires(constructible_from_args_v<Args...>)
    {
        using initializer = detail::vec_initializer<N, T>;

        initializer::init_from(*this, std::forward<Args>(args)...);
    };

    /**
     * @brief Construct vector with given a value for all components.
     *
     * @param value The value at hand
     */
    template<typename U>
        requires(std::is_convertible_v<U, T>)
    explicit constexpr Vec(const U value)
    {
        for (std::size_t i = 0; i < N; i++) {
            (*this)[i] = static_cast<T>(value);
        }
    }

    /**
     * @brief Construct vector from a larger vector that is truncated.
     *
     * @param that The larger vector to be truncated
     */
    template<std::size_t M>
        requires(M > N)
    explicit constexpr Vec(const Vec<M, T>& that)
    {
        for (std::size_t i = 0; i < N; i++) {
            (*this)[i] = that[i];
        }
    }

public:
    /**
     * @brief Get number of components
     *
     * @return Number of components as size_t
     */
    static constexpr std::size_t size() { return N; }

    /**
     * @brief Get pointer to the underlying data
     *
     * @return The pointer to the first component of the underlying data
     */
    constexpr T* data() { return &m_components[0]; }

    /**
     * @brief Get pointer to the underlying data
     *
     * @return The pointer to the first component of the underlying data
     */
    constexpr const T* data() const { return &m_components[0]; }

    /**
     * @brief Get underlying array
     *
     * @return A reference to the underlying array
     */
    constexpr std::array<T, N>& array() { return m_components; }

    /**
     * @brief Get underlying array
     *
     * @return A const reference to the underlying array
     */
    constexpr const std::array<T, N>& array() const { return m_components; }

    /**
     * @brief Index the vector
     *
     * @param i The index
     * @return Reference to the value at the index
     */
    constexpr T& operator[](const std::size_t i)
    {
        assert(i < this->size() && "index is inside bounds");

        return m_components[i];
    }

    /**
     * @brief Index the vector
     *
     * @param i The index
     * @return The value at the index
     */
    constexpr T operator[](const std::size_t i) const
    {
        assert(i < this->size() && "index is inside bounds");

        return m_components[i];
    }

    /**
     * @brief In-place assignment with other Vec
     *
     * @note this is neccessary to work alongside Swizzled
     *
     * @param that Other Vec object
     * @return This
     */
    constexpr Vec& operator=(const Vec& that)
    {
        this->m_components = that.m_components;

        return *this;
    }

    /**
     * @brief Print the vector
     *
     * @param out The output stream
     * @param v The vector at hand
     * @return A reference to the modified output stream
     */
    friend std::ostream& operator<<(std::ostream& out, const Vec& v)
    {
        const auto& array = v.array();

        out << "[";
        for (auto it = array.begin(); it != array.end(); ++it) {
            out << *it;
            if (std::next(it) != array.end()) {
                out << ", ";
            }
        }
        out << "]";

        return out;
    }

public:
    /**
     * @brief Check if two vectors are equal
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return Whether the vectors are equal
     */
    friend constexpr bool operator==(const Vec& lhs, const Vec& rhs)
        requires(std::is_integral_v<T>)
    {
        bool res = true;
        for (std::size_t i = 0; i < N; i++) {
            res &= lhs[i] == rhs[i];
        }
        return res;
    }

    /**
     * @brief Check if two vectors are (approximately) equal
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return Whether the vectors are (approximately) equal
     */
    friend constexpr bool operator==(const Vec& lhs, const Vec& rhs)
        requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        bool res = true;
        for (std::size_t i = 0; i < N; i++) {
            res &= almost_equal(lhs[i], rhs[i]);
        }
        return res;
    }

    /**
     * @brief Check if two vectors are (approximately) not equal
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return Whether the vectors are (approximately) not equal
     */
    friend constexpr bool operator!=(const Vec& lhs, const Vec& rhs)
        requires(std::is_integral_v<T> || std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        return !(lhs == rhs);
    }

    /**
     * @brief Lexicographically compare two vectors
     *
     * @param lhs left hand side
     * @param rhs right hand side
     * @return boolean convertible depending on operator
     */
    friend constexpr std::strong_ordering operator<=>(const Vec& lhs, const Vec& rhs)
        requires(std::is_integral_v<T>)
    {
        const auto lexicographical_compare = [](const Vec& l, const Vec& r) -> bool {
            for (std::size_t i = 0; i < N; i++) {
                if (l[i] < r[i]) {
                    return true;
                } else if (r[i] < l[i]) {
                    return false;
                }
            }
            return true;
        };
        if (lexicographical_compare(lhs, rhs)) {
            return std::strong_ordering::less;
        }
        if (lexicographical_compare(rhs, lhs)) {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

    /**
     * @brief Lexicographically compare two vectors
     *
     * @param lhs left hand side
     * @param rhs right hand side
     * @return boolean convertible depending on operator
     */
    friend constexpr std::partial_ordering operator<=>(const Vec& lhs, const Vec& rhs)
        requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        const auto lexicographical_compare = [](const Vec& l, const Vec& r) -> bool {
            for (std::size_t i = 0; i < N; i++) {
                if (almost_less_than(l[i], r[i])) {
                    return true;
                } else if (almost_less_than(r[i], l[i])) {
                    return false;
                }
            }
            return true;
        };
        if (lexicographical_compare(lhs, rhs)) {
            return std::partial_ordering::less;
        }
        if (lexicographical_compare(rhs, lhs)) {
            return std::partial_ordering::greater;
        }
        return std::partial_ordering::equivalent;
    }

public:
    /**
     * @brief In-place component-wise addition with vector
     *
     * @param that The other vector at hand
     * @return This
     */
    constexpr Vec& operator+=(const Vec& that)
    {
        for (std::size_t i = 0; i < N; i++) {
            (*this)[i] += that[i];
        }
        return (*this);
    }

    /**
     * @brief In-place component-wise subtraction with vector
     *
     * @param that The other vector at hand
     * @return This
     */
    constexpr Vec& operator-=(const Vec& that)
    {
        for (std::size_t i = 0; i < N; i++) {
            (*this)[i] -= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place vector-scalar multiplication
     *
     * @param scalar The scalar at hand
     * @return This
     */
    constexpr Vec& operator*=(const T scalar)
    {
        for (std::size_t i = 0; i < N; i++) {
            (*this)[i] *= scalar;
        }
        return *this;
    }

    /**
     * @brief In-place vector-scalar division
     *
     * @param scalar The scalar at hand
     * @return This
     */
    constexpr Vec& operator/=(const T scalar)
    {
        assert(scalar != 0 && "non-zero division");

        for (std::size_t i = 0; i < N; i++) {
            (*this)[i] /= scalar;
        }
        return *this;
    }

public:
    /**
     * @brief Unary plus vector operator
     *
     * @return The copy of the vector as-is
     */
    constexpr Vec operator+() const { return *this; }

    /**
     * @brief Unary minus vector operator
     *
     * @return The vector with it's components sign-flipped
     */
    constexpr Vec operator-() const
    {
        Vec res{};
        for (std::size_t i = 0; i < N; i++) {
            res[i] = -(*this)[i];
        }
        return res;
    }

    /**
     * @brief Vector-vector component-wise addition
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return The resulting vector
     */
    friend constexpr Vec operator+(const Vec& lhs, const Vec& rhs)
    {
        Vec res{};
        for (std::size_t i = 0; i < N; i++) {
            res[i] = lhs[i] + rhs[i];
        }
        return res;
    }

    /**
     * @brief Vector-vector component-wise subtraction
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return The resulting vector
     */
    friend constexpr Vec operator-(const Vec& lhs, const Vec& rhs)
    {
        Vec res{};
        for (std::size_t i = 0; i < N; i++) {
            res[i] = lhs[i] - rhs[i];
        }
        return res;
    }

    /**
     * @brief Scalar-vector multiplication
     *
     * @param scalar The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return The resulting vector
     */
    friend constexpr Vec operator*(const T scalar, const Vec& rhs)
    {
        Vec res{};
        for (std::size_t i = 0; i < N; i++) {
            res[i] = scalar * rhs[i];
        }
        return res;
    }

    /**
     * @brief Vector-scalar multiplication
     *
     * @param lhs The left hand side of the operand
     * @param scalar The right hand side of the operand
     * @return The resulting vector
     */
    friend constexpr Vec operator*(const Vec& lhs, const T scalar)
    {
        Vec res{};
        for (std::size_t i = 0; i < N; i++) {
            res[i] = lhs[i] * scalar;
        }
        return res;
    }

    /**
     * @brief Vector-vector component-wise multiplication (hadamard
     * product)
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return The resulting vector
     */
    friend constexpr Vec operator*(const Vec& lhs, const Vec& rhs)
    {
        Vec res{};
        for (std::size_t i = 0; i < N; i++) {
            res[i] = lhs[i] * rhs[i];
        }
        return res;
    }

    /**
     * @brief Vector-scalar division
     *
     * @param lhs The left hand side of the operand
     * @param scalar The right hand side of the operand
     * @return The resulting vector
     */
    friend constexpr Vec operator/(const Vec& lhs, const T scalar)
    {
        assert(scalar != 0 && "non-zero division");
        Vec res{};
        for (std::size_t i = 0; i < N; i++) {
            res[i] = lhs[i] / scalar;
        }
        return res;
    }

    /**
     * @brief Vector-vector component-wise division (hadamard divsion)
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return The resulting vector
     */
    friend constexpr Vec operator/(const Vec& lhs, const Vec& rhs)
    {
#ifndef NDEBUG
        for (std::size_t i = 0; i < N; i++) {
            assert(rhs[i] != 0 && "non-zero division");
        }
#endif
        Vec res{};
        for (std::size_t i = 0; i < N; i++) {
            res[i] = lhs[i] / rhs[i];
        }
        return res;
    }

public:
    /**
     * @brief Get a vector from this pointing to that.
     *
     * Alias to that - (*this).
     *
     * @param that Where the head of the resulting vector lies
     * @return A vector with it's tail at This and it's head at that
     */
    constexpr Vec vector_to(const Vec& that) const { return that - (*this); }

    /**
     * @brief Calculate the norm of the vector
     *
     * @return The norm of this vector as a number
     */
    constexpr T norm() const
        requires(std::is_floating_point_v<T>)
    {
        const Vec v = (*this);

        return detail::sqrt(dot(v, v));
    }

    /**
     * @brief Calculate the normalized vector
     *
     * @return Copy of this normalized
     */
    constexpr Vec normalized() const
        requires(std::is_floating_point_v<T>)
    {
        return (*this) / this->norm();
    }

    /**
     * @brief Calculate the length of the vector.
     *
     * Alias to .norm()
     *
     * @return The length of this as a number
     */
    constexpr T length() const
        requires(std::is_floating_point_v<T>)
    {
        return this->norm();
    }

    /**
     * @brief Calculate the magnitude of the vector.
     *
     * Alias to .norm()
     *
     * @return The magnitude of this as a number
     */
    constexpr T magnitude() const
        requires(std::is_floating_point_v<T>)
    {
        return this->norm();
    }

    /**
     * @brief Calculate the normalized direction of the vector.
     *
     * Alias to .normalized()
     *
     * @return The direction of this as a normalized vector
     */
    constexpr Vec direction() const
        requires(std::is_floating_point_v<T>)
    {
        return this->normalized();
    }

    /**
     * @brief Project this vector onto other vector
     *
     * Alias to dot(*this, that.normalized()).
     *
     * @param that The other vector
     * @param is_normalized Whether the other vector is pre-normalized
     * @return The projection of this onto that as a vector
     */
    constexpr Vec project_onto(const Vec& that, const bool is_normalized = false) const
        requires(std::is_floating_point_v<T>)
    {
        const Vec v = (*this);
        const Vec that_unit = is_normalized ? that : that.normalized();

        return dot(v, that_unit) * v;
    }

    /**
     * @brief Reflect this vector across a given plane
     *
     * @param normal The normal of the plane
     * @param is_normalized Whether the normal is pre-normalized
     * @return This reflected accross the plane
     */
    constexpr Vec reflect(const Vec& normal, const bool is_normalized = false) const
        requires(std::is_floating_point_v<T> && (N == 2 || N == 3))
    {
        return (*this) - T{ 2 } * this->project_onto(normal, is_normalized);
    }
};

namespace detail {

/// @cond DO_NOT_DOCUMENT

template<typename T, typename... Args>
struct not_a_single_value_impl
{
    static constexpr bool value =
            (sizeof...(Args) != 1) ||
            (sizeof...(Args) == 1 && !std::is_same_v<T, typename std::tuple_element<0, std::tuple<Args...>>::type>);
};

template<typename T, typename... Args>
struct not_a_single_value : not_a_single_value_impl<T, std::remove_cvref_t<Args>...>
{};

template<typename T, typename... Args>
struct not_a_single_convertible_value_impl
{
    static constexpr bool value =
            (sizeof...(Args) != 1) ||
            (sizeof...(Args) == 1 &&
             !std::is_convertible_v<typename std::tuple_element<0, std::tuple<Args...>>::type, T>);
};

template<typename T, typename... Args>
struct not_a_single_convertible_value : not_a_single_convertible_value_impl<T, std::remove_cvref_t<Args>...>
{};

template<typename TT>
struct vec_info_impl
{
    static constexpr bool value = false;
    static constexpr std::size_t size = 0;
};

template<std::size_t M, typename U>
struct vec_info_impl<Vec<M, U>>
{
    static constexpr bool value = true;
    static constexpr std::size_t size = M;
};

template<std::size_t M, typename T, std::size_t... Is>
    requires(sizeof...(Is) > 1)
struct vec_info_impl<Swizzled<Vec<sizeof...(Is), T>, M, T, Is...>>
{
    static constexpr bool value = true;
    static constexpr std::size_t size = sizeof...(Is);
};

template<typename TT>
using vec_info = vec_info_impl<std::remove_cvref_t<TT>>;

template<std::size_t N, typename T, typename... Args>
struct vec_constructible_from_impl
{
private:
    static constexpr bool accepted_types = ((std::convertible_to<Args, T> || vec_info_impl<Args>::value) && ...);

    static constexpr std::size_t num_values = ((std::convertible_to<Args, T> ? 1 : 0) + ...);

    static constexpr bool total_size_in_bounds = (N >= num_values + (vec_info_impl<Args>::size + ...));

public:
    static constexpr bool value = accepted_types && total_size_in_bounds;
};

template<std::size_t N, typename T, typename... Args>
struct vec_constructible_from : vec_constructible_from_impl<N, T, std::remove_cvref_t<Args>...>
{};

template<std::size_t N, typename T>
struct vec_initializer
{
public:
    template<typename... Args>
    static constexpr void init_from(Vec<N, T>& out, Args&&... args)
        requires(vec_constructible_from<N, T, Args...>::value)
    {
        std::size_t idx = 0;
        init_from_inner(idx, out, std::forward<Args>(args)...);
    }

private:
    static constexpr void init_from_inner(std::size_t& idx, Vec<N, T>& out)
    {
        (void)(idx);
        (void)(out);
    }

    template<typename U>
        requires(std::convertible_to<U, T>)
    static constexpr void init_from_inner(std::size_t& idx, Vec<N, T>& out, const U arg, auto&&... rest)
    {
        out[idx] = static_cast<T>(arg);
        idx += 1;
        init_from_inner(idx, out, rest...);
    }

    template<std::size_t M, typename U>
    static constexpr void init_from_inner(std::size_t& idx, Vec<N, T>& out, const Vec<M, U>& arg, auto&&... rest)
    {
        for (std::size_t j = 0; j < M; j++) {
            out[idx + j] = static_cast<T>(arg[j]);
        }
        idx += M;
        init_from_inner(idx, out, rest...);
    }

    template<std::size_t M, typename U, std::size_t... Is>
        requires(sizeof...(Is) > 1)
    static constexpr void init_from_inner(std::size_t& idx,
                                          Vec<N, T>& out,
                                          const Swizzled<Vec<sizeof...(Is), U>, M, U, Is...>& arg,
                                          auto&&... rest)
    {
        for (std::size_t j = 0; j < sizeof...(Is); j++) {
            out[idx + j] = static_cast<T>(arg[j]);
        }
        idx += sizeof...(Is);
        init_from_inner(idx, out, rest...);
    }
};

/// @endcond

} // namespace detail

} // namespace asciirast::math
