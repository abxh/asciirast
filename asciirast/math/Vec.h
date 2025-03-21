/**
 * @file Vec.h
 * @brief File with definition of the math vector class
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <compare>
#include <cstdlib>
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
 * vectors.
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
 * @brief Trait to check if Args doesn't contain a single value of type T.
 *
 * @tparam T    Type of components
 * @tparam Args The argument types
 */
template<typename T, typename... Args>
struct not_a_single_value;

}

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
static T
dot(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
{
    return std::ranges::fold_left(
            std::views::zip_transform(std::multiplies(), lhs.range(), rhs.range()), T{ 0 }, std::plus());
}

/**
 * @brief Vector 2d "cross product"
 *
 * By calculating the signed magnitude of the plane spanned by the two
 * vectors.
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
static T
cross(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
    requires(N == 2)
{
    return lhs.x * rhs.y - rhs.x * lhs.y; // XY_magnitude
}

/**
 * @brief Vector 3d cross product
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
static Vec<N, T>
cross(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
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
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
static T
angle(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
    requires(N == 2 && std::is_floating_point_v<T>)
{
    /*
        atan2(x,y)        = tan^-1(y/x) [with quadrant sign considerations]
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
    requires(N > 0 && std::is_arithmetic_v<T>)
static T
angle(const Vec<N, T>& lhs, const Vec<N, T>& rhs, const Vec<N, T>& up_, const bool up_is_normalized)
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
    requires(N > 0 && std::is_arithmetic_v<T>)
static Vec<N, T>
lerp(const Vec<N, T>& lhs, const Vec<N, T>& rhs, const T t)
    requires(std::is_floating_point_v<T>)
{
    auto func = [=](const T x, const T y) -> T { return std::lerp(x, y, t); };
    auto view = std::views::zip_transform(func, lhs.range(), rhs.range());

    return Vec<N, T>{ view };
}

/**
 * @brief Take the max value of each component
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
static Vec<N, T>
max(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
{
    auto func = [=](const T lhs, const T rhs) -> T { return std::max(lhs, rhs); };
    auto view = std::views::zip_transform(func, lhs.range(), rhs.range());

    return Vec<N, T>{ view };
}

/**
 * @brief Take the min value of each component
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
static Vec<N, T>
min(const Vec<N, T>& lhs, const Vec<N, T>& rhs)
{
    auto func = [=](const T lhs, const T rhs) -> T { return std::min(lhs, rhs); };
    auto view = std::views::zip_transform(func, lhs.range(), rhs.range());

    return Vec<N, T>{ view };
}

/**
 * @brief Clamp each component
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
static Vec<N, T>
clamp(const Vec<N, T>& v, const Vec<N, T>& low, const Vec<N, T>& high)
    requires(std::is_integral_v<T>)
{
    auto func = [=](const T x, const T low_val, const T high_val) -> T { return std::clamp(x, low_val, high_val); };
    auto view = std::views::zip_transform(func, v.range(), low.range(), high.range());

    return Vec<N, T>{ view };
}

/**
 * @brief Take the absolute value of each component
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
static Vec<N, T>
abs(const Vec<N, T>& v)
{
    auto func = [](const T x) { return std::abs(x); };
    auto view = std::ranges::views::transform(v.range(), func);

    return Vec<N, T>{ view };
}

/**
 * @brief Take the rounded value of each component
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
static Vec<N, T>
round(const Vec<N, T>& v)
    requires(std::is_floating_point_v<T>)
{
    auto func = [](const T x) { return std::round(x); };
    auto view = std::ranges::views::transform(v.range(), func);

    return Vec<N, T>{ view };
}

/**
 * @brief Take the ceiled value of each component
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
static Vec<N, T>
ceil(const Vec<N, T>& v)
    requires(std::is_floating_point_v<T>)
{
    auto func = [](const T x) { return std::ceil(x); };
    auto view = std::ranges::views::transform(v.range(), func);

    return Vec<N, T>{ view };
}

/**
 * @brief Take the ceiled value of each component
 */
template<std::size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
static Vec<N, T>
floor(const Vec<N, T>& v)
    requires(std::is_floating_point_v<T>)
{
    auto func = [](const T x) { return std::floor(x); };
    auto view = std::ranges::views::transform(v.range(), func);

    return Vec<N, T>{ view };
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
protected:
    using VecBase<Vec, N, T>::m_components;

public:
    /**
     * @brief Construct default vector with all zeroes
     */
    Vec()
    {
        for (auto& x : this->range()) {
            x = 0;
        }
    }

    /**
     * @brief Implicitly construct vector from swizzled components.
     */
    template<std::size_t M, std::size_t... Is>
        requires(M > 1)
    Vec(const Swizzled<Vec, M, T, Is...>& that)
    {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x = y;
        }
    }

    /**
     * @brief Construct vector with given value for all components.
     */
    explicit Vec(const T y)
    {
        for (auto& x : this->range()) {
            x = y;
        }
    }

    /**
     * @brief Construct vector from a larger truncated vector.
     */
    template<std::size_t M>
        requires(M > N)
    explicit Vec(const Vec<M, T>& that)
    {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x = y;
        }
    }

    /**
     * @brief Construct vector from a mix of values and (smaller) vectors,
     * padding the rest of the vector with zeroes.
     */
    template<typename... Args>
        requires(sizeof...(Args) > 0 && detail::not_a_single_value<T, Args...>::value)
    explicit Vec(Args&&... args)
        requires(detail::vec_constructible_from<N, T, Args...>::value)
    {
        detail::vec_initializer<N, T>::init_from(*this, std::forward<Args>(args)...);
    };

    /**
     * @brief Construct vector from input range.
     */
    explicit Vec(std::ranges::input_range auto&& range)
    {
        assert(N == std::ranges::distance(range) && "range size is same as vector size");

        for (auto [x, y] : std::views::zip(this->range(), range)) {
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
    T& operator[](const std::size_t i)
    {
        assert(i < this->size() && "index is inside bounds");

        return m_components[i];
    }

    /**
     * @brief Index the vector.
     */
    T operator[](const std::size_t i) const
    {
        assert(i < this->size() && "index is inside bounds");

        return m_components[i];
    }

    /**
     * @brief Get range over vector components.
     */
    std::ranges::view auto range() { return std::ranges::views::all(m_components); }

    /**
     * @brief Get range over vector components.
     */
    std::ranges::view auto range() const { return std::ranges::views::all(m_components); }

    /**
     * @brief Print the vector
     */
    friend std::ostream& operator<<(std::ostream& out, const Vec& v)
    {
        auto view = v.range();

        out << "[";
        for (auto it = view.begin(); it != view.end(); ++it) {
            out << *it;

            if (std::next(it) != view.end()) {
                out << ", ";
            }
        }
        out << "]";

        return out;
    }

public:
    /**
     * @brief Check if equal to other vector
     */
    friend bool operator==(const Vec& lhs, const Vec& rhs)
        requires(std::is_integral_v<T>)
    {
        return std::ranges::equal(lhs.range(), rhs.range());
    }

    /**
     * @brief Check if approximately equal to other vector
     */
    friend bool operator==(const Vec& lhs, const Vec& rhs)
        requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        auto func = [](const T x, const T y) { return math::almost_equal<T>(x, y); };

        return std::ranges::equal(lhs.range(), rhs.range(), func);
    }

    /**
     * @brief Check if approximately not equal to other vector
     */
    friend bool operator!=(const Vec& lhs, const Vec& rhs)
        requires(std::is_integral_v<T> || std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        return !(lhs == rhs);
    }

    /**
     * @brief Compare lexigraphically with other vector
     */
    friend auto operator<=>(const Vec& lhs, const Vec& rhs)
        requires(std::is_integral_v<T>)
    {
        if (std::ranges::lexicographical_compare(lhs.range(), rhs.range())) {
            return std::strong_ordering::less;
        }
        if (std::ranges::lexicographical_compare(rhs.range(), lhs.range())) {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

    /**
     * @brief Compare lexigraphically with other vector
     */
    friend auto operator<=>(const Vec& lhs, const Vec& rhs)
        requires(std::is_floating_point_v<T>)
    {
        auto func = [](const T x, const T y) { return math::almost_less_than(x, y); };

        if (std::ranges::equal(lhs.range(), rhs.range(), func)) {
            return std::partial_ordering::less;
        }
        if (std::ranges::equal(rhs.range(), lhs.range(), func)) {
            return std::partial_ordering::greater;
        }
        return std::partial_ordering::equivalent;
    }

public:
    /**
     * @brief In-place component-wise addition with vector
     */
    Vec& operator+=(const Vec& that)
    {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x += y;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with vector
     */
    Vec& operator-=(const Vec& that)
    {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x -= y;
        }
        return *this;
    }

    /**
     * @brief In-place vector-scalar multiplication
     */
    Vec& operator*=(const T scalar)
    {
        for (auto& x : this->range()) {
            x *= scalar;
        }
        return *this;
    }

    /**
     * @brief In-place vector-scalar division
     */
    Vec& operator/=(const T scalar)
    {
        if constexpr (std::is_integral_v<T>) {
            assert(scalar != 0 && "non-zero division");
        }

        for (auto& x : this->range()) {
            x /= scalar;
        }
        return *this;
    }

public:
    /**
     * @brief Unary minus vector operator
     */
    Vec operator-() const
    {
        auto func = [](const T x) -> T { return -x; };
        auto view = std::ranges::views::transform(this->range(), func);

        return Vec{ view };
    }

    /**
     * @brief Vector-vector component-wise addition
     */
    friend Vec operator+(const Vec& lhs, const Vec& rhs)
    {
        auto func = std::plus();
        auto view = std::views::zip_transform(func, lhs.range(), rhs.range());

        return Vec{ view };
    }

    /**
     * @brief Vector-vector component-wise subtraction
     */
    friend Vec operator-(const Vec& lhs, const Vec& rhs)
    {
        auto func = std::minus();
        auto view = std::views::zip_transform(func, lhs.range(), rhs.range());

        return Vec{ view };
    }

    /**
     * @brief Scalar-vector multiplication
     */
    friend Vec operator*(const T scalar, const Vec& rhs)
    {
        auto func = [=](const T x) -> T { return scalar * x; };
        auto view = std::views::transform(rhs.range(), func);

        return Vec{ view };
    }

    /**
     * @brief Vector-scalar multiplication
     */
    friend Vec operator*(const Vec& lhs, const T scalar)
    {
        auto func = [=](const T x) -> T { return x * scalar; };
        auto view = std::views::transform(lhs.range(), func);

        return Vec{ view };
    }

    /**
     * @brief Vector-scalar division
     */
    friend Vec operator/(const Vec& lhs, const T scalar)
    {
        if constexpr (std::is_integral_v<T>) {
            assert(scalar != 0 && "non-zero division");
        }

        auto func = [=](const T x) -> T { return x / scalar; };
        auto view = std::views::transform(lhs.range(), func);

        return Vec{ view };
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
        const Vec v = (*this);

        return std::sqrt(dot(v, v));
    }

    /**
     * @brief Calculate the normalized vector
     */
    Vec normalized() const
        requires(std::is_floating_point_v<T>)
    {
        return (*this) / this->norm();
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
     * projection vector. Alias to dot(*this, that.normalized()).
     *
     * @note Can pre-normalize vector and set second parameter to true.
     */
    Vec project_onto(const Vec& that, const bool is_normalized = false) const
        requires(std::is_floating_point_v<T>)
    {
        const Vec v = (*this);
        const Vec that_unit = is_normalized ? that : that.normalized();

        return dot(v, that_unit) * v;
    }

    /**
     * @brief Using projection, reflect this vector across a given plane defined
     * by it's normal vector
     *
     * @note Can pre-normalize vector and set second parameter to true.
     */
    Vec reflect(const Vec& normal, const bool is_normalized = false) const
        requires(std::is_floating_point_v<T> && (N == 2 || N == 3))
    {
        return (*this) - T{ 2 } * this->project_onto(normal, is_normalized);
    }
};

template<typename T>
    requires(std::is_arithmetic_v<T>)
class Vec<1, T> : public VecBase<Vec, 1, T>
{
protected:
    using VecBase<Vec, 1, T>::m_components;

public:
    /**
     * @brief Explicitly construct 1d-vector with value
     */
    explicit Vec(const T initial_value) { m_components[0] = initial_value; };

    /**
     * @brief Implicitly convert to number
     */
    operator T() { return m_components[0]; }

    /**
     * @brief Get number of components
     */
    static constexpr std::size_t size() { return 1; }

    /**
     * @brief Get range over vector component.
     */
    std::ranges::view auto range() { return std::ranges::views::single(m_components[0]); }

    /**
     * @brief Get range over vector component.
     */
    std::ranges::view auto range() const { return std::ranges::views::single(m_components[0]); }
};

namespace detail {

template<typename T, typename... Args>
struct not_a_single_value_impl
{
    static constexpr bool value = sizeof...(Args) > 0 ||
                                  sizeof...(Args) == 1 &&
                                          !std::is_same_v<T, typename std::tuple_element<0, std::tuple<Args...>>::type>;
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
    static constexpr bool value = false;   ///< whether the type is vector like
    static constexpr std::size_t size = 0; ///< vector size
};

/**
 * @brief Vector information trait
 */
template<std::size_t M, typename U>
struct vec_info_impl<Vec<M, U>>
{
    static constexpr bool value = true;    ///< whether the type is vector like
    static constexpr std::size_t size = M; ///< vector size
};

/**
 * @brief Vector information trait
 */
template<std::size_t M, typename T, std::size_t... Is>
    requires(sizeof...(Is) > 1)
struct vec_info_impl<Swizzled<Vec<sizeof...(Is), T>, M, T, Is...>>
{
    static constexpr bool value = true;                ///< whether the type is vector like
    static constexpr std::size_t size = sizeof...(Is); ///< vector size
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
    static constexpr bool value = accepted_types && total_size_in_bounds; ///< whether the arguments are of acceptable
                                                                          ///< types and total size is inside bounds
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
        for (auto i : std::views::iota(idx, N)) {
            out[i] = 0;
        }
    }

private:
    static constexpr void init_from_inner(std::size_t& idx, Vec<N, T>& out)
    {
        (void)(idx);
        (void)(out);
    }

    static constexpr void init_from_inner(std::size_t& idx, Vec<N, T>& out, const T arg, auto&&... rest)
    {
        out[idx] = arg;
        idx += 1;
        init_from_inner(idx, out, rest...);
    }

    template<std::size_t M, typename U>
    static constexpr void init_from_inner(std::size_t& idx, Vec<N, T>& out, const Vec<M, U>& arg, auto&&... rest)
    {
        for (auto [i, j] : std::views::zip(std::views::iota(idx, idx + M), std::views::iota(0U, M))) {
            out[i] = static_cast<T>(arg[j]);
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
        auto values = arg.range();
        for (auto [i, value] : std::views::zip(std::views::iota(idx), values)) {
            out[i] = static_cast<T>(value);
        }
        idx += sizeof...(Is);
        init_from_inner(idx, out, rest...);
    }
};

}

} // namespace asciirast::math
