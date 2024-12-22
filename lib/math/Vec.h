/**
 * @file Vec.h
 * @brief Math vector
 */

#include "VecBase.h"
#include <cstddef>
#include <type_traits>

namespace asciirast::math {

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

/**
 * @brief N-dimensional math vector
 */
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
    consteval auto size()
    {
        return N;
    }

    /**
     * @brief Default constructor. Fill all values in vector with default value
     * constructor.
     */
    Vec()
        : Base{T{}} {};

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
     * @brief Initiate a 3-dimensional vector with a value and a vector
     */
    template <typename U1, typename U2>
        requires(utils::non_narrowing_conv<value_type, U1, U2>)
    Vec(const U1 &l, const Vec<2, U2> &r)
        requires(N == 3)
        : Base{l, r[0], r[1]} {};

    /**
     * @brief Initiate a 3-dimensional vector with a vector and value
     */
    template <typename U1, typename U2>
        requires(utils::non_narrowing_conv<value_type, U1, U2>)
    Vec(const Vec<2, U1> &l, const U2 &r)
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
     * @brief Initiate a 4-dimensional vector with value(s) and vector(s)
     */
    template <typename U1, typename U2>
        requires(utils::non_narrowing_conv<value_type, U1, U2>)
    Vec(const U1 &l, const Vec<3, U2> &r)
        requires(N == 4)
        : Base{l, r[0], r[1], r[2]} {};

    /**
     * @brief Initiate a 4-dimensional vector with value(s) and vector(s)
     */
    template <typename U1, typename U2>
        requires(utils::non_narrowing_conv<value_type, U1, U2>)
    Vec(const Vec<3, U1> &l, const U2 &r)
        requires(N == 4)
        : Base{l[0], l[1], l[2], r} {};

    /**
     * @brief Initiate a 4-dimensional vector with value(s) and vector(s)
     */
    template <typename U1, typename U2>
        requires(utils::non_narrowing_conv<value_type, U1, U2>)
    Vec(const Vec<2, U1> &l, const Vec<2, U2> &r)
        requires(N == 4)
        : Base{l[0], l[1], r[0], r[1]} {};

    /**
     * @brief Initiate a 4-dimensional vector with value(s) and vector(s)
     */
    template <typename U1, typename U2, typename U3>
        requires(utils::non_narrowing_conv<value_type, U1, U2>)
    Vec(const U1 &l, const Vec<2, U2> &m, const U3 &r)
        requires(N == 4)
        : Base{l, m[0], m[1], r} {};


};

} // namespace asciirast::math
