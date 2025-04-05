/**
 * @file VecBase.h
 * @brief File with definitions of the generic and specialized vector base
 * classes
 */

#pragma once

#include <array>

#include "./Swizzled.h"

namespace asciirast::math {

/**
 * @brief Generic vector base class
 *
 * Takes a Vec type that accepts size_t and type as parameters. This is passed
 * along to the Swizzled class.
 *
 * @tparam Vec  The vector class as a type, unintansiated.
 * @tparam N    Number of components in the vector
 * @tparam T    Type of components
 */
template<template<std::size_t, typename> class Vec, std::size_t N, typename T>
class VecBase
{
public:
    std::array<T, N> m_components; ///< array of components
};

/**
 * @brief Specialized 1D vector base class.
 *
 * Takes a Vec type that accepts size_t and type as parameters. This is passed
 * along to the Swizzled class.
 *
 * @tparam Vec  The vector class as a type, unintansiated.
 * @tparam T    Type of components
 */
template<template<std::size_t, typename> class Vec, typename T>
class VecBase<Vec, 1, T>
{
    static constexpr std::size_t N = 1;

    template<std::size_t Index>
    using SingleComponent = SingleVectorComponent<N, T, Index>;

    template<std::size_t... Indicies>
    using SwizzledComponents = Swizzled<Vec<sizeof...(Indicies), T>, N, T, Indicies...>;

public:
    union
    {
        std::array<T, N> m_components; ///< array of components

        /**
         * @name Swizzled permutatons of {x} of max size 4 as members.
         *
         * E.g. x, xx, xxx, xxxx
         * @{
         */
        /// @cond DO_NOT_DOCUMENT
        SingleComponent<0> x;
        SwizzledComponents<0, 0> xx;
        SwizzledComponents<0, 0, 0> xxx;
        SwizzledComponents<0, 0, 0, 0> xxxx;
        /// @endcond
        /// @}
    };
};

/**
 * @brief Specialized 2D vector base class.
 *
 * Takes a Vec type that accepts size_t and type as parameters. This is passed
 * along to the Swizzled class.
 *
 * @tparam Vec  The vector class as a type, unintansiated.
 * @tparam T    Type of components
 */
template<template<std::size_t, typename> class Vec, typename T>
class VecBase<Vec, 2, T>
{
    static constexpr std::size_t N = 2;

    template<std::size_t Index>
    using SingleComponent = SingleVectorComponent<N, T, Index>;

    template<std::size_t... Indicies>
    using SwizzledComponents = Swizzled<Vec<sizeof...(Indicies), T>, N, T, Indicies...>;

public:
    union
    {
        std::array<T, N> m_components; ///< array of components

        /**
         * @name Swizzled permutatons of {x, y} of max size 4 as members.
         *
         * E.g. x, y, xx, ..., xxx, ..., xxxx, ..., yyyy.
         * @{
         */
        /// @cond DO_NOT_DOCUMENT
        SingleComponent<0> x;
        SingleComponent<1> y;

        SwizzledComponents<0, 0> xx;
        SwizzledComponents<0, 1> xy;
        SwizzledComponents<1, 0> yx;
        SwizzledComponents<1, 1> yy;

        SwizzledComponents<0, 0, 0> xxx;
        SwizzledComponents<0, 0, 1> xxy;
        SwizzledComponents<0, 1, 0> xyx;
        SwizzledComponents<0, 1, 1> xyy;
        SwizzledComponents<1, 0, 0> yxx;
        SwizzledComponents<1, 0, 1> yxy;
        SwizzledComponents<1, 1, 0> yyx;
        SwizzledComponents<1, 1, 1> yyy;

        SwizzledComponents<0, 0, 0, 0> xxxx;
        SwizzledComponents<0, 0, 0, 1> xxxy;
        SwizzledComponents<0, 0, 1, 0> xxyx;
        SwizzledComponents<0, 0, 1, 1> xxyy;
        SwizzledComponents<0, 1, 0, 0> xyxx;
        SwizzledComponents<0, 1, 0, 1> xyxy;
        SwizzledComponents<0, 1, 1, 0> xyyx;
        SwizzledComponents<0, 1, 1, 1> xyyy;

        SwizzledComponents<1, 0, 0, 0> yxxx;
        SwizzledComponents<1, 0, 0, 1> yxxy;
        SwizzledComponents<1, 0, 1, 0> yxyx;
        SwizzledComponents<1, 0, 1, 1> yxyy;
        SwizzledComponents<1, 1, 0, 0> yyxx;
        SwizzledComponents<1, 1, 0, 1> yyxy;
        SwizzledComponents<1, 1, 1, 0> yyyx;
        SwizzledComponents<1, 1, 1, 1> yyyy;
        /// @endcond
        /// @}
    };
};

/**
 * @brief Specialized 3D vector base class.
 *
 * Takes a Vec type that accepts size_t and type as parameters. This is passed
 * along to the Swizzled class.
 *
 * @tparam Vec  The vector class as a type, unintansiated.
 * @tparam T    Type of components
 */
template<template<std::size_t, typename> class Vec, typename T>
class VecBase<Vec, 3, T>
{
    static constexpr std::size_t N = 3;

    template<std::size_t Index>
    using SingleComponent = SingleVectorComponent<N, T, Index>;

    template<std::size_t... Indicies>
    using SwizzledComponents = Swizzled<Vec<sizeof...(Indicies), T>, N, T, Indicies...>;

public:
    union
    {
        std::array<T, N> m_components; ///< array of components

        /**
         * @name Swizzled permutatons of {x, y, z} of max size 4 as members.
         *
         * E.g. x, y, z, xx, ..., xxx, ..., xxxx, ..., zzzz.
         * @{
         */
        /// @cond DO_NOT_DOCUMENT
        SingleComponent<0> x;
        SingleComponent<1> y;
        SingleComponent<2> z;

        SwizzledComponents<0, 0> xx;
        SwizzledComponents<0, 1> xy;
        SwizzledComponents<0, 2> xz;
        SwizzledComponents<1, 0> yx;
        SwizzledComponents<1, 1> yy;
        SwizzledComponents<1, 2> yz;
        SwizzledComponents<2, 0> zx;
        SwizzledComponents<2, 1> zy;
        SwizzledComponents<2, 2> zz;

        SwizzledComponents<0, 0, 0> xxx;
        SwizzledComponents<0, 0, 1> xxy;
        SwizzledComponents<0, 0, 2> xxz;
        SwizzledComponents<0, 1, 0> xyx;
        SwizzledComponents<0, 1, 1> xyy;
        SwizzledComponents<0, 1, 2> xyz;
        SwizzledComponents<0, 2, 0> xzx;
        SwizzledComponents<0, 2, 1> xzy;
        SwizzledComponents<0, 2, 2> xzz;
        SwizzledComponents<1, 0, 0> yxx;
        SwizzledComponents<1, 0, 1> yxy;
        SwizzledComponents<1, 0, 2> yxz;
        SwizzledComponents<1, 1, 0> yyx;
        SwizzledComponents<1, 1, 1> yyy;
        SwizzledComponents<1, 1, 2> yyz;
        SwizzledComponents<1, 2, 0> yzx;
        SwizzledComponents<1, 2, 1> yzy;
        SwizzledComponents<1, 2, 2> yzz;
        SwizzledComponents<2, 0, 0> zxx;
        SwizzledComponents<2, 0, 1> zxy;
        SwizzledComponents<2, 0, 2> zxz;
        SwizzledComponents<2, 1, 0> zyx;
        SwizzledComponents<2, 1, 1> zyy;
        SwizzledComponents<2, 1, 2> zyz;
        SwizzledComponents<2, 2, 0> zzx;
        SwizzledComponents<2, 2, 1> zzy;
        SwizzledComponents<2, 2, 2> zzz;

        SwizzledComponents<0, 0, 0, 0> xxxx;
        SwizzledComponents<0, 0, 0, 1> xxxy;
        SwizzledComponents<0, 0, 0, 2> xxxz;
        SwizzledComponents<0, 0, 1, 0> xxyx;
        SwizzledComponents<0, 0, 1, 1> xxyy;
        SwizzledComponents<0, 0, 1, 2> xxyz;
        SwizzledComponents<0, 0, 2, 0> xxzx;
        SwizzledComponents<0, 0, 2, 1> xxzy;
        SwizzledComponents<0, 0, 2, 2> xxzz;
        SwizzledComponents<0, 1, 0, 0> xyxx;
        SwizzledComponents<0, 1, 0, 1> xyxy;
        SwizzledComponents<0, 1, 0, 2> xyxz;
        SwizzledComponents<0, 1, 1, 0> xyyx;
        SwizzledComponents<0, 1, 1, 1> xyyy;
        SwizzledComponents<0, 1, 1, 2> xyyz;
        SwizzledComponents<0, 1, 2, 0> xyzx;
        SwizzledComponents<0, 1, 2, 1> xyzy;
        SwizzledComponents<0, 1, 2, 2> xyzz;
        SwizzledComponents<0, 2, 0, 0> xzxx;
        SwizzledComponents<0, 2, 0, 1> xzxy;
        SwizzledComponents<0, 2, 0, 2> xzxz;
        SwizzledComponents<0, 2, 1, 0> xzyx;
        SwizzledComponents<0, 2, 1, 1> xzyy;
        SwizzledComponents<0, 2, 1, 2> xzyz;
        SwizzledComponents<0, 2, 2, 0> xzzx;
        SwizzledComponents<0, 2, 2, 1> xzzy;
        SwizzledComponents<0, 2, 2, 2> xzzz;

        SwizzledComponents<1, 0, 0, 0> yxxx;
        SwizzledComponents<1, 0, 0, 1> yxxy;
        SwizzledComponents<1, 0, 0, 2> yxxz;
        SwizzledComponents<1, 0, 1, 0> yxyx;
        SwizzledComponents<1, 0, 1, 1> yxyy;
        SwizzledComponents<1, 0, 1, 2> yxyz;
        SwizzledComponents<1, 0, 2, 0> yxzx;
        SwizzledComponents<1, 0, 2, 1> yxzy;
        SwizzledComponents<1, 0, 2, 2> yxzz;
        SwizzledComponents<1, 1, 0, 0> yyxx;
        SwizzledComponents<1, 1, 0, 1> yyxy;
        SwizzledComponents<1, 1, 0, 2> yyxz;
        SwizzledComponents<1, 1, 1, 0> yyyx;
        SwizzledComponents<1, 1, 1, 1> yyyy;
        SwizzledComponents<1, 1, 1, 2> yyyz;
        SwizzledComponents<1, 1, 2, 0> yyzx;
        SwizzledComponents<1, 1, 2, 1> yyzy;
        SwizzledComponents<1, 1, 2, 2> yyzz;
        SwizzledComponents<1, 2, 0, 0> yzxx;
        SwizzledComponents<1, 2, 0, 1> yzxy;
        SwizzledComponents<1, 2, 0, 2> yzxz;
        SwizzledComponents<1, 2, 1, 0> yzyx;
        SwizzledComponents<1, 2, 1, 1> yzyy;
        SwizzledComponents<1, 2, 1, 2> yzyz;
        SwizzledComponents<1, 2, 2, 0> yzzx;
        SwizzledComponents<1, 2, 2, 1> yzzy;
        SwizzledComponents<1, 2, 2, 2> yzzz;

        SwizzledComponents<2, 0, 0, 0> zxxx;
        SwizzledComponents<2, 0, 0, 1> zxxy;
        SwizzledComponents<2, 0, 0, 2> zxxz;
        SwizzledComponents<2, 0, 1, 0> zxyx;
        SwizzledComponents<2, 0, 1, 1> zxyy;
        SwizzledComponents<2, 0, 1, 2> zxyz;
        SwizzledComponents<2, 0, 2, 0> zxzx;
        SwizzledComponents<2, 0, 2, 1> zxzy;
        SwizzledComponents<2, 0, 2, 2> zxzz;
        SwizzledComponents<2, 1, 0, 0> zyxx;
        SwizzledComponents<2, 1, 0, 1> zyxy;
        SwizzledComponents<2, 1, 0, 2> zyxz;
        SwizzledComponents<2, 1, 1, 0> zyyx;
        SwizzledComponents<2, 1, 1, 1> zyyy;
        SwizzledComponents<2, 1, 1, 2> zyyz;
        SwizzledComponents<2, 1, 2, 0> zyzx;
        SwizzledComponents<2, 1, 2, 1> zyzy;
        SwizzledComponents<2, 1, 2, 2> zyzz;
        SwizzledComponents<2, 2, 0, 0> zzxx;
        SwizzledComponents<2, 2, 0, 1> zzxy;
        SwizzledComponents<2, 2, 0, 2> zzxz;
        SwizzledComponents<2, 2, 1, 0> zzyx;
        SwizzledComponents<2, 2, 1, 1> zzyy;
        SwizzledComponents<2, 2, 1, 2> zzyz;
        SwizzledComponents<2, 2, 2, 0> zzzx;
        SwizzledComponents<2, 2, 2, 1> zzzy;
        SwizzledComponents<2, 2, 2, 2> zzzz;
        /// @endcond
        /// @}
    };
};

/**
 * @brief Specialized 4D vector base class.
 *
 * Takes a Vec type that accepts size_t and type as parameters. This is passed
 * along to the Swizzled class.
 *
 * @tparam Vec  The vector class as a type, unintansiated.
 * @tparam T    Type of components
 */
template<template<std::size_t, typename> class Vec, typename T>
class VecBase<Vec, 4, T>
{
    static constexpr std::size_t N = 4;

    template<std::size_t Index>
    using SingleComponent = SingleVectorComponent<N, T, Index>;

    template<std::size_t... Indicies>
    using SwizzledComponents = Swizzled<Vec<sizeof...(Indicies), T>, N, T, Indicies...>;

public:
    union
    {
        std::array<T, N> m_components; ///< array of components

        /**
         * @name Swizzled permutatons of {x, y, z, w} of max size 4.
         *
         * E.g. x, y, z, w, xx, ..., xxx, ..., xxxx, ..., wwww.
         * @{
         */
        /// @cond DO_NOT_DOCUMENT
        SingleComponent<0> x;
        SingleComponent<1> y;
        SingleComponent<2> z;
        SingleComponent<3> w;

        SwizzledComponents<0, 0> xx;
        SwizzledComponents<0, 1> xy;
        SwizzledComponents<0, 2> xz;
        SwizzledComponents<0, 3> xw;
        SwizzledComponents<1, 0> yx;
        SwizzledComponents<1, 1> yy;
        SwizzledComponents<1, 2> yz;
        SwizzledComponents<1, 3> yw;
        SwizzledComponents<2, 0> zx;
        SwizzledComponents<2, 1> zy;
        SwizzledComponents<2, 2> zz;
        SwizzledComponents<2, 3> zw;
        SwizzledComponents<3, 0> wx;
        SwizzledComponents<3, 1> wy;
        SwizzledComponents<3, 2> wz;
        SwizzledComponents<3, 3> ww;

        SwizzledComponents<0, 0, 0> xxx;
        SwizzledComponents<0, 0, 1> xxy;
        SwizzledComponents<0, 0, 2> xxz;
        SwizzledComponents<0, 0, 3> xxw;
        SwizzledComponents<0, 1, 0> xyx;
        SwizzledComponents<0, 1, 1> xyy;
        SwizzledComponents<0, 1, 2> xyz;
        SwizzledComponents<0, 1, 3> xyw;
        SwizzledComponents<0, 2, 0> xzx;
        SwizzledComponents<0, 2, 1> xzy;
        SwizzledComponents<0, 2, 2> xzz;
        SwizzledComponents<0, 2, 3> xzw;
        SwizzledComponents<0, 3, 0> xwx;
        SwizzledComponents<0, 3, 1> xwy;
        SwizzledComponents<0, 3, 2> xwz;
        SwizzledComponents<0, 3, 3> xww;
        SwizzledComponents<1, 0, 0> yxx;
        SwizzledComponents<1, 0, 1> yxy;
        SwizzledComponents<1, 0, 2> yxz;
        SwizzledComponents<1, 0, 3> yxw;
        SwizzledComponents<1, 1, 0> yyx;
        SwizzledComponents<1, 1, 1> yyy;
        SwizzledComponents<1, 1, 2> yyz;
        SwizzledComponents<1, 1, 3> yyw;
        SwizzledComponents<1, 2, 0> yzx;
        SwizzledComponents<1, 2, 1> yzy;
        SwizzledComponents<1, 2, 2> yzz;
        SwizzledComponents<1, 2, 3> yzw;
        SwizzledComponents<1, 3, 0> ywx;
        SwizzledComponents<1, 3, 1> ywy;
        SwizzledComponents<1, 3, 2> ywz;
        SwizzledComponents<1, 3, 3> yww;
        SwizzledComponents<2, 0, 0> zxx;
        SwizzledComponents<2, 0, 1> zxy;
        SwizzledComponents<2, 0, 2> zxz;
        SwizzledComponents<2, 0, 3> zxw;
        SwizzledComponents<2, 1, 0> zyx;
        SwizzledComponents<2, 1, 1> zyy;
        SwizzledComponents<2, 1, 2> zyz;
        SwizzledComponents<2, 1, 3> zyw;
        SwizzledComponents<2, 2, 0> zzx;
        SwizzledComponents<2, 2, 1> zzy;
        SwizzledComponents<2, 2, 2> zzz;
        SwizzledComponents<2, 2, 3> zzw;
        SwizzledComponents<2, 3, 0> zwx;
        SwizzledComponents<2, 3, 1> zwy;
        SwizzledComponents<2, 3, 2> zwz;
        SwizzledComponents<2, 3, 3> zww;
        SwizzledComponents<3, 0, 0> wxx;
        SwizzledComponents<3, 0, 1> wxy;
        SwizzledComponents<3, 0, 2> wxz;
        SwizzledComponents<3, 0, 3> wxw;
        SwizzledComponents<3, 1, 0> wyx;
        SwizzledComponents<3, 1, 1> wyy;
        SwizzledComponents<3, 1, 2> wyz;
        SwizzledComponents<3, 1, 3> wyw;
        SwizzledComponents<3, 2, 0> wzx;
        SwizzledComponents<3, 2, 1> wzy;
        SwizzledComponents<3, 2, 2> wzz;
        SwizzledComponents<3, 2, 3> wzw;
        SwizzledComponents<3, 3, 0> wwx;
        SwizzledComponents<3, 3, 1> wwy;
        SwizzledComponents<3, 3, 2> wwz;
        SwizzledComponents<3, 3, 3> www;

        SwizzledComponents<0, 0, 0, 0> xxxx;
        SwizzledComponents<0, 0, 0, 1> xxxy;
        SwizzledComponents<0, 0, 0, 2> xxxz;
        SwizzledComponents<0, 0, 0, 3> xxxw;
        SwizzledComponents<0, 0, 1, 0> xxyx;
        SwizzledComponents<0, 0, 1, 1> xxyy;
        SwizzledComponents<0, 0, 1, 2> xxyz;
        SwizzledComponents<0, 0, 1, 3> xxyw;
        SwizzledComponents<0, 0, 2, 0> xxzx;
        SwizzledComponents<0, 0, 2, 1> xxzy;
        SwizzledComponents<0, 0, 2, 2> xxzz;
        SwizzledComponents<0, 0, 2, 3> xxzw;
        SwizzledComponents<0, 0, 3, 0> xxwx;
        SwizzledComponents<0, 0, 3, 1> xxwy;
        SwizzledComponents<0, 0, 3, 2> xxwz;
        SwizzledComponents<0, 0, 3, 3> xxww;
        SwizzledComponents<0, 1, 0, 0> xyxx;
        SwizzledComponents<0, 1, 0, 1> xyxy;
        SwizzledComponents<0, 1, 0, 2> xyxz;
        SwizzledComponents<0, 1, 0, 3> xyxw;
        SwizzledComponents<0, 1, 1, 0> xyyx;
        SwizzledComponents<0, 1, 1, 1> xyyy;
        SwizzledComponents<0, 1, 1, 2> xyyz;
        SwizzledComponents<0, 1, 1, 3> xyyw;
        SwizzledComponents<0, 1, 2, 0> xyzx;
        SwizzledComponents<0, 1, 2, 1> xyzy;
        SwizzledComponents<0, 1, 2, 2> xyzz;
        SwizzledComponents<0, 1, 2, 3> xyzw;
        SwizzledComponents<0, 1, 3, 0> xywx;
        SwizzledComponents<0, 1, 3, 1> xywy;
        SwizzledComponents<0, 1, 3, 2> xywz;
        SwizzledComponents<0, 1, 3, 3> xyww;
        SwizzledComponents<0, 2, 0, 0> xzxx;
        SwizzledComponents<0, 2, 0, 1> xzxy;
        SwizzledComponents<0, 2, 0, 2> xzxz;
        SwizzledComponents<0, 2, 0, 3> xzxw;
        SwizzledComponents<0, 2, 1, 0> xzyx;
        SwizzledComponents<0, 2, 1, 1> xzyy;
        SwizzledComponents<0, 2, 1, 2> xzyz;
        SwizzledComponents<0, 2, 1, 3> xzyw;
        SwizzledComponents<0, 2, 2, 0> xzzx;
        SwizzledComponents<0, 2, 2, 1> xzzy;
        SwizzledComponents<0, 2, 2, 2> xzzz;
        SwizzledComponents<0, 2, 2, 3> xzzw;
        SwizzledComponents<0, 2, 3, 0> xzwx;
        SwizzledComponents<0, 2, 3, 1> xzwy;
        SwizzledComponents<0, 2, 3, 2> xzwz;
        SwizzledComponents<0, 2, 3, 3> xzww;
        SwizzledComponents<0, 3, 0, 0> xwxx;
        SwizzledComponents<0, 3, 0, 1> xwxy;
        SwizzledComponents<0, 3, 0, 2> xwxz;
        SwizzledComponents<0, 3, 0, 3> xwxw;
        SwizzledComponents<0, 3, 1, 0> xwyx;
        SwizzledComponents<0, 3, 1, 1> xwyy;
        SwizzledComponents<0, 3, 1, 2> xwyz;
        SwizzledComponents<0, 3, 1, 3> xwyw;
        SwizzledComponents<0, 3, 2, 0> xwzx;
        SwizzledComponents<0, 3, 2, 1> xwzy;
        SwizzledComponents<0, 3, 2, 2> xwzz;
        SwizzledComponents<0, 3, 2, 3> xwzw;
        SwizzledComponents<0, 3, 3, 0> xwwx;
        SwizzledComponents<0, 3, 3, 1> xwwy;
        SwizzledComponents<0, 3, 3, 2> xwwz;
        SwizzledComponents<0, 3, 3, 3> xwww;

        SwizzledComponents<1, 0, 0, 0> yxxx;
        SwizzledComponents<1, 0, 0, 1> yxxy;
        SwizzledComponents<1, 0, 0, 2> yxxz;
        SwizzledComponents<1, 0, 0, 3> yxxw;
        SwizzledComponents<1, 0, 1, 0> yxyx;
        SwizzledComponents<1, 0, 1, 1> yxyy;
        SwizzledComponents<1, 0, 1, 2> yxyz;
        SwizzledComponents<1, 0, 1, 3> yxyw;
        SwizzledComponents<1, 0, 2, 0> yxzx;
        SwizzledComponents<1, 0, 2, 1> yxzy;
        SwizzledComponents<1, 0, 2, 2> yxzz;
        SwizzledComponents<1, 0, 2, 3> yxzw;
        SwizzledComponents<1, 0, 3, 0> yxwx;
        SwizzledComponents<1, 0, 3, 1> yxwy;
        SwizzledComponents<1, 0, 3, 2> yxwz;
        SwizzledComponents<1, 0, 3, 3> yxww;
        SwizzledComponents<1, 1, 0, 0> yyxx;
        SwizzledComponents<1, 1, 0, 1> yyxy;
        SwizzledComponents<1, 1, 0, 2> yyxz;
        SwizzledComponents<1, 1, 0, 3> yyxw;
        SwizzledComponents<1, 1, 1, 0> yyyx;
        SwizzledComponents<1, 1, 1, 1> yyyy;
        SwizzledComponents<1, 1, 1, 2> yyyz;
        SwizzledComponents<1, 1, 1, 3> yyyw;
        SwizzledComponents<1, 1, 2, 0> yyzx;
        SwizzledComponents<1, 1, 2, 1> yyzy;
        SwizzledComponents<1, 1, 2, 2> yyzz;
        SwizzledComponents<1, 1, 2, 3> yyzw;
        SwizzledComponents<1, 1, 3, 0> yywx;
        SwizzledComponents<1, 1, 3, 1> yywy;
        SwizzledComponents<1, 1, 3, 2> yywz;
        SwizzledComponents<1, 1, 3, 3> yyww;
        SwizzledComponents<1, 2, 0, 0> yzxx;
        SwizzledComponents<1, 2, 0, 1> yzxy;
        SwizzledComponents<1, 2, 0, 2> yzxz;
        SwizzledComponents<1, 2, 0, 3> yzxw;
        SwizzledComponents<1, 2, 1, 0> yzyx;
        SwizzledComponents<1, 2, 1, 1> yzyy;
        SwizzledComponents<1, 2, 1, 2> yzyz;
        SwizzledComponents<1, 2, 1, 3> yzyw;
        SwizzledComponents<1, 2, 2, 0> yzzx;
        SwizzledComponents<1, 2, 2, 1> yzzy;
        SwizzledComponents<1, 2, 2, 2> yzzz;
        SwizzledComponents<1, 2, 2, 3> yzzw;
        SwizzledComponents<1, 2, 3, 0> yzwx;
        SwizzledComponents<1, 2, 3, 1> yzwy;
        SwizzledComponents<1, 2, 3, 2> yzwz;
        SwizzledComponents<1, 2, 3, 3> yzww;
        SwizzledComponents<1, 3, 0, 0> ywxx;
        SwizzledComponents<1, 3, 0, 1> ywxy;
        SwizzledComponents<1, 3, 0, 2> ywxz;
        SwizzledComponents<1, 3, 0, 3> ywxw;
        SwizzledComponents<1, 3, 1, 0> ywyx;
        SwizzledComponents<1, 3, 1, 1> ywyy;
        SwizzledComponents<1, 3, 1, 2> ywyz;
        SwizzledComponents<1, 3, 1, 3> ywyw;
        SwizzledComponents<1, 3, 2, 0> ywzx;
        SwizzledComponents<1, 3, 2, 1> ywzy;
        SwizzledComponents<1, 3, 2, 2> ywzz;
        SwizzledComponents<1, 3, 2, 3> ywzw;
        SwizzledComponents<1, 3, 3, 0> ywwx;
        SwizzledComponents<1, 3, 3, 1> ywwy;
        SwizzledComponents<1, 3, 3, 2> ywwz;
        SwizzledComponents<1, 3, 3, 3> ywww;

        SwizzledComponents<2, 0, 0, 0> zxxx;
        SwizzledComponents<2, 0, 0, 1> zxxy;
        SwizzledComponents<2, 0, 0, 2> zxxz;
        SwizzledComponents<2, 0, 0, 3> zxxw;
        SwizzledComponents<2, 0, 1, 0> zxyx;
        SwizzledComponents<2, 0, 1, 1> zxyy;
        SwizzledComponents<2, 0, 1, 2> zxyz;
        SwizzledComponents<2, 0, 1, 3> zxyw;
        SwizzledComponents<2, 0, 2, 0> zxzx;
        SwizzledComponents<2, 0, 2, 1> zxzy;
        SwizzledComponents<2, 0, 2, 2> zxzz;
        SwizzledComponents<2, 0, 2, 3> zxzw;
        SwizzledComponents<2, 0, 3, 0> zxwx;
        SwizzledComponents<2, 0, 3, 1> zxwy;
        SwizzledComponents<2, 0, 3, 2> zxwz;
        SwizzledComponents<2, 0, 3, 3> zxww;
        SwizzledComponents<2, 1, 0, 0> zyxx;
        SwizzledComponents<2, 1, 0, 1> zyxy;
        SwizzledComponents<2, 1, 0, 2> zyxz;
        SwizzledComponents<2, 1, 0, 3> zyxw;
        SwizzledComponents<2, 1, 1, 0> zyyx;
        SwizzledComponents<2, 1, 1, 1> zyyy;
        SwizzledComponents<2, 1, 1, 2> zyyz;
        SwizzledComponents<2, 1, 1, 3> zyyw;
        SwizzledComponents<2, 1, 2, 0> zyzx;
        SwizzledComponents<2, 1, 2, 1> zyzy;
        SwizzledComponents<2, 1, 2, 2> zyzz;
        SwizzledComponents<2, 1, 2, 3> zyzw;
        SwizzledComponents<2, 1, 3, 0> zywx;
        SwizzledComponents<2, 1, 3, 1> zywy;
        SwizzledComponents<2, 1, 3, 2> zywz;
        SwizzledComponents<2, 1, 3, 3> zyww;
        SwizzledComponents<2, 2, 0, 0> zzxx;
        SwizzledComponents<2, 2, 0, 1> zzxy;
        SwizzledComponents<2, 2, 0, 2> zzxz;
        SwizzledComponents<2, 2, 0, 3> zzxw;
        SwizzledComponents<2, 2, 1, 0> zzyx;
        SwizzledComponents<2, 2, 1, 1> zzyy;
        SwizzledComponents<2, 2, 1, 2> zzyz;
        SwizzledComponents<2, 2, 1, 3> zzyw;
        SwizzledComponents<2, 2, 2, 0> zzzx;
        SwizzledComponents<2, 2, 2, 1> zzzy;
        SwizzledComponents<2, 2, 2, 2> zzzz;
        SwizzledComponents<2, 2, 2, 3> zzzw;
        SwizzledComponents<2, 2, 3, 0> zzwx;
        SwizzledComponents<2, 2, 3, 1> zzwy;
        SwizzledComponents<2, 2, 3, 2> zzwz;
        SwizzledComponents<2, 2, 3, 3> zzww;
        SwizzledComponents<2, 3, 0, 0> zwxx;
        SwizzledComponents<2, 3, 0, 1> zwxy;
        SwizzledComponents<2, 3, 0, 2> zwxz;
        SwizzledComponents<2, 3, 0, 3> zwxw;
        SwizzledComponents<2, 3, 1, 0> zwyx;
        SwizzledComponents<2, 3, 1, 1> zwyy;
        SwizzledComponents<2, 3, 1, 2> zwyz;
        SwizzledComponents<2, 3, 1, 3> zwyw;
        SwizzledComponents<2, 3, 2, 0> zwzx;
        SwizzledComponents<2, 3, 2, 1> zwzy;
        SwizzledComponents<2, 3, 2, 2> zwzz;
        SwizzledComponents<2, 3, 2, 3> zwzw;
        SwizzledComponents<2, 3, 3, 0> zwwx;
        SwizzledComponents<2, 3, 3, 1> zwwy;
        SwizzledComponents<2, 3, 3, 2> zwwz;
        SwizzledComponents<2, 3, 3, 3> zwww;

        SwizzledComponents<3, 0, 0, 0> wxxx;
        SwizzledComponents<3, 0, 0, 1> wxxy;
        SwizzledComponents<3, 0, 0, 2> wxxz;
        SwizzledComponents<3, 0, 0, 3> wxxw;
        SwizzledComponents<3, 0, 1, 0> wxyx;
        SwizzledComponents<3, 0, 1, 1> wxyy;
        SwizzledComponents<3, 0, 1, 2> wxyz;
        SwizzledComponents<3, 0, 1, 3> wxyw;
        SwizzledComponents<3, 0, 2, 0> wxzx;
        SwizzledComponents<3, 0, 2, 1> wxzy;
        SwizzledComponents<3, 0, 2, 2> wxzz;
        SwizzledComponents<3, 0, 2, 3> wxzw;
        SwizzledComponents<3, 0, 3, 0> wxwx;
        SwizzledComponents<3, 0, 3, 1> wxwy;
        SwizzledComponents<3, 0, 3, 2> wxwz;
        SwizzledComponents<3, 0, 3, 3> wxww;
        SwizzledComponents<3, 1, 0, 0> wyxx;
        SwizzledComponents<3, 1, 0, 1> wyxy;
        SwizzledComponents<3, 1, 0, 2> wyxz;
        SwizzledComponents<3, 1, 0, 3> wyxw;
        SwizzledComponents<3, 1, 1, 0> wyyx;
        SwizzledComponents<3, 1, 1, 1> wyyy;
        SwizzledComponents<3, 1, 1, 2> wyyz;
        SwizzledComponents<3, 1, 1, 3> wyyw;
        SwizzledComponents<3, 1, 2, 0> wyzx;
        SwizzledComponents<3, 1, 2, 1> wyzy;
        SwizzledComponents<3, 1, 2, 2> wyzz;
        SwizzledComponents<3, 1, 2, 3> wyzw;
        SwizzledComponents<3, 1, 3, 0> wywx;
        SwizzledComponents<3, 1, 3, 1> wywy;
        SwizzledComponents<3, 1, 3, 2> wywz;
        SwizzledComponents<3, 1, 3, 3> wyww;
        SwizzledComponents<3, 2, 0, 0> wzxx;
        SwizzledComponents<3, 2, 0, 1> wzxy;
        SwizzledComponents<3, 2, 0, 2> wzxz;
        SwizzledComponents<3, 2, 0, 3> wzxw;
        SwizzledComponents<3, 2, 1, 0> wzyx;
        SwizzledComponents<3, 2, 1, 1> wzyy;
        SwizzledComponents<3, 2, 1, 2> wzyz;
        SwizzledComponents<3, 2, 1, 3> wzyw;
        SwizzledComponents<3, 2, 2, 0> wzzx;
        SwizzledComponents<3, 2, 2, 1> wzzy;
        SwizzledComponents<3, 2, 2, 2> wzzz;
        SwizzledComponents<3, 2, 2, 3> wzzw;
        SwizzledComponents<3, 2, 3, 0> wzwx;
        SwizzledComponents<3, 2, 3, 1> wzwy;
        SwizzledComponents<3, 2, 3, 2> wzwz;
        SwizzledComponents<3, 2, 3, 3> wzww;
        SwizzledComponents<3, 3, 0, 0> wwxx;
        SwizzledComponents<3, 3, 0, 1> wwxy;
        SwizzledComponents<3, 3, 0, 2> wwxz;
        SwizzledComponents<3, 3, 0, 3> wwxw;
        SwizzledComponents<3, 3, 1, 0> wwyx;
        SwizzledComponents<3, 3, 1, 1> wwyy;
        SwizzledComponents<3, 3, 1, 2> wwyz;
        SwizzledComponents<3, 3, 1, 3> wwyw;
        SwizzledComponents<3, 3, 2, 0> wwzx;
        SwizzledComponents<3, 3, 2, 1> wwzy;
        SwizzledComponents<3, 3, 2, 2> wwzz;
        SwizzledComponents<3, 3, 2, 3> wwzw;
        SwizzledComponents<3, 3, 3, 0> wwwx;
        SwizzledComponents<3, 3, 3, 1> wwwy;
        SwizzledComponents<3, 3, 3, 2> wwwz;
        SwizzledComponents<3, 3, 3, 3> wwww;
        /// @endcond
        /// @}
    };
};

} // namespace asciirast::math
