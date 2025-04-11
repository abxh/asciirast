/**
 * @file Vec.h
 * @brief File with definition of the math vector class
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <complex>
#include <numeric>
#include <ostream>
#include <type_traits>

#include "./VecBase.h"
#include "./utils.h"

namespace asciirast::math {

namespace detail {

/**
 * @brief Trait to check if vector can constructed from arguments.
 *
 * @tparam N    Number of components in the vector
 * @tparam T    Type of components
 * @tparam Args Arguments consisting of values, swizzled components, and
 *              vectors.
 */
template<std::size_t N, typename T, typename... Args>
struct vec_constructible_from;

/**
 * @brief Helper class to initialize vector from arguments.
 *
 * @tparam N    Number of components in the vector
 * @tparam T    Type of components
 */
template<std::size_t N, typename T>
struct vec_initializer;

/**
 * @brief Trait to check if Args doesn't contain a single value of given
 *        type.
 *
 * @tparam T    The given type
 * @tparam Args The argument types
 */
template<typename T, typename... Args>
struct not_a_single_value;

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
 * @brief Vector 2d "cross product"
 *
 * By calculating the signed magnitude of the plane spanned by the two
 * vectors.
 */
template<std::size_t N, typename T>
constexpr auto
cross(const Vec<N, T>& lhs, const Vec<N, T>& rhs) -> T
    requires(N == 2)
{
    return lhs.x * rhs.y - rhs.x * lhs.y; // XY_magnitude
}

/**
 * @brief Vector 3d cross product
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
 * @todo make this function constexpr in c++26 or use library
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
 * parameter t ranging from 0 to 1.
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
    static constexpr bool constructible_from_args_v =
            (detail::not_a_single_value<T, Args...>::value && detail::not_a_single_value<Vec, Args...>::value) &&
            (detail::vec_constructible_from<N, T, Args...>::value);

protected:
    using VecBase<Vec, N, T>::m_components;

public:
    using value_type = T; ///@< value type

    /**
     * @name default constructors
     * @{
     */
    constexpr Vec() = default;
    constexpr Vec(const Vec&) = default;
    constexpr Vec(Vec&&) = default;
    ///@}

    /**
     * @brief Implicitly construct vector from swizzled components.
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
     * @brief Implicitly construct vector from complex number
     */
    constexpr Vec(const std::complex<T>& v)
        requires(N == 2)
    {
        m_components[0] = std::real(v);
        m_components[1] = std::imag(v);
    }

    /**
     * @brief Construct vector from a mix of values and (smaller) vectors,
     * padding the rest of the vector with zeroes.
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
     */
    explicit constexpr Vec(const T value)
    {
        for (std::size_t i = 0; i < N; i++) {
            (*this)[i] = value;
        }
    }

    /**
     * @brief Construct vector from a larger truncated vector.
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
     */
    static constexpr std::size_t size() { return N; }

    /**
     * @brief Get pointer over underlying data
     */
    constexpr T* data() { return &m_components[0]; }

    /**
     * @brief Get pointer over underlying data
     */
    constexpr const T* data() const { return &m_components[0]; }

    /**
     * @brief Get underlying array
     */
    constexpr std::array<T, N>& array() { return m_components; }

    /**
     * @brief Get underlying array
     */
    constexpr const std::array<T, N>& array() const { return m_components; }

    /**
     * @brief Index the vector.
     */
    constexpr T& operator[](const std::size_t i)
    {
        assert(i < this->size() && "index is inside bounds");

        return m_components[i];
    }

    /**
     * @brief Index the vector.
     */
    constexpr T operator[](const std::size_t i) const
    {
        assert(i < this->size() && "index is inside bounds");

        return m_components[i];
    }

    /**
     * @brief Vector copy operator (redefined to work with Swizzled)
     */
    constexpr Vec& operator=(const Vec& that)
    {
        this->m_components = that.m_components;

        return *this;
    }

    /**
     * @brief Print the vector
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

    /**
     * @brief Convert to complex number
     */
    constexpr std::complex<T> to_complex() const
        requires(N == 2)
    {
        return std::complex(m_components[0], m_components[1]);
    }

public:
    /**
     * @brief Check if equal to other vector
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
     * @brief Check if approximately equal to other vector
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
     * @brief Check if approximately not equal to other vector
     */
    friend constexpr bool operator!=(const Vec& lhs, const Vec& rhs)
        requires(std::is_integral_v<T> || std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        return !(lhs == rhs);
    }

    /**
     * @brief Compare lexigraphically with other vector
     */
    friend constexpr auto operator<=>(const Vec& lhs, const Vec& rhs)
        requires(std::is_integral_v<T>)
    {
        const auto lexicographical_compare = [](const Vec& l, const Vec& r) -> bool {
            bool res = true;
            for (std::size_t i = 0; i < N; i++) {
                res &= l[i] <= r[i];
            }
            return res;
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
     * @brief Compare lexigraphically with other vector
     */
    friend constexpr auto operator<=>(const Vec& lhs, const Vec& rhs)
        requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        const auto lexicographical_compare = [](const Vec& l, const Vec& r) -> bool {
            bool res = true;
            for (std::size_t i = 0; i < N; i++) {
                res &= math::almost_less_than(l[i], r[i]) || math::almost_equal(l[i], r[i]);
            }
            return res;
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
     */
    constexpr Vec operator+() const { return *this; }

    /**
     * @brief Unary minus vector operator
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
     * @brief Get a vector from this pointing to that. Alias to
     * that - (*this).
     */
    constexpr Vec vector_to(const Vec& that) const { return that - (*this); }

    /**
     * @brief Calculate the norm of the vector.
     */
    constexpr T norm() const
        requires(std::is_floating_point_v<T>)
    {
        const Vec v = (*this);

        return math::sqrt(dot(v, v));
    }

    /**
     * @brief Calculate the normalized vector
     */
    constexpr Vec normalized() const
        requires(std::is_floating_point_v<T>)
    {
        return (*this) / this->norm();
    }

    /**
     * @brief Calculate the length of the vector. Alias to .norm()
     */
    constexpr T length() const
        requires(std::is_floating_point_v<T>)
    {
        return this->norm();
    }

    /**
     * @brief Calculate the magnitude of the vector. Alias to .norm()
     */
    constexpr T magnitude() const
        requires(std::is_floating_point_v<T>)
    {
        return this->norm();
    }

    /**
     * @brief Calculate the normalized direction of the vector. Alias to
     * .normalized()
     */
    constexpr Vec direction() const
        requires(std::is_floating_point_v<T>)
    {
        return this->normalized();
    }

    /**
     * @brief Project this vector onto other vector and calculate the
     * resulting projection vector. Alias to dot(*this,
     * that.normalized()).
     *
     * @note Can pre-normalize vector and set second parameter to true.
     */
    constexpr Vec project_onto(const Vec& that, const bool is_normalized = false) const
        requires(std::is_floating_point_v<T>)
    {
        const Vec v = (*this);
        const Vec that_unit = is_normalized ? that : that.normalized();

        return dot(v, that_unit) * v;
    }

    /**
     * @brief Using projection, reflect this vector across a given plane
     * defined by it's normal vector
     *
     * @note Can pre-normalize vector and set second parameter to true.
     */
    constexpr Vec reflect(const Vec& normal, const bool is_normalized = false) const
        requires(std::is_floating_point_v<T> && (N == 2 || N == 3))
    {
        return (*this) - T{ 2 } * this->project_onto(normal, is_normalized);
    }
};

namespace detail {

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

/**
 * @brief Vector information trait
 */
template<typename TT>
struct vec_info_impl
{
    static constexpr bool value = false;   ///@< default value
    static constexpr std::size_t size = 0; ///@< default size
};

/**
 * @brief Vector information trait
 */
template<std::size_t M, typename U>
struct vec_info_impl<Vec<M, U>>
{
    static constexpr bool value = true;    ///@< whether the type is vector like
    static constexpr std::size_t size = M; ///@< vector size
};

/**
 * @brief Vector information trait
 */
template<std::size_t M, typename T, std::size_t... Is>
    requires(sizeof...(Is) > 1)
struct vec_info_impl<Swizzled<Vec<sizeof...(Is), T>, M, T, Is...>>
{
    static constexpr bool value = true;                ///@< whether the type is vector like
    static constexpr std::size_t size = sizeof...(Is); ///@< vector size
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
    /**
     * @brief Initialize vector from arguments
     *
     * @param out   The vector to initialize
     * @param args  The arguments
     */
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

} // namespace detail

} // namespace asciirast::math
