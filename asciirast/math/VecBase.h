/**
 * @file VecBase.h
 * @brief File with definitions of the generic and specialized vector
 *        base classes
 */

#pragma once

#include <array>

#include "./Swizzled.h"

namespace asciirast::math {

/**
 * @brief Generic vector base class
 *
 * Takes a Vec type that accepts size_t and type as parameters. This is
 * passed along to the Swizzled class.
 *
 * @tparam Vec  The vector class as a type, unintansiated.
 * @tparam N    Number of components in the vector
 * @tparam T    Type of components
 */
template<template<std::size_t, typename> class Vec, std::size_t N, typename T>
class VecBase
{
public:
    std::array<T, N> m_components = {}; ///< array of components

    /**
     * @name default constructors
     * @{
     */
    constexpr VecBase() = default;
    constexpr VecBase(const VecBase&) = default;
    constexpr VecBase(VecBase&&) = default;
    ///@}
};

/**
 * @brief Specialized 1D vector base class.
 *
 * Takes a Vec type that accepts size_t and type as parameters. This is
 * passed along to the Swizzled class.
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
    /**
     * @name default constructors
     * @{
     */
    constexpr VecBase() = default;
    constexpr VecBase(const VecBase&) = default;
    constexpr VecBase(VecBase&&) = default;
    ///@}

    union
    {
        std::array<T, N> m_components = {}; ///< array of components

        /**
         * @name Swizzled permutatons of {x} or {r} of max size 4 as
         * members.
         * @{
         */
        /// @cond DO_NOT_DOCUMENT
        SingleComponent<0> x, r;
        SwizzledComponents<0, 0> xx, rr;
        SwizzledComponents<0, 0, 0> xxx, rrr;
        SwizzledComponents<0, 0, 0, 0> xxxx, rrrr;
        /// @endcond
        /// @}
    };
};

/**
 * @brief Specialized 2D vector base class.
 *
 * Takes a Vec type that accepts size_t and type as parameters. This is
 * passed along to the Swizzled class.
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
    /**
     * @name default constructors
     * @{
     */
    constexpr VecBase() = default;
    constexpr VecBase(const VecBase&) = default;
    constexpr VecBase(VecBase&&) = default;
    ///@}

    union
    {
        std::array<T, N> m_components = {}; ///< array of components

        /**
         * @name Swizzled permutatons of {x, y}, {r, g} of max size 4 as
         * members.
         * @{
         */
        /// @cond DO_NOT_DOCUMENT
        SingleComponent<0> x, r;
        SingleComponent<1> y, g;

        SwizzledComponents<0, 0> xx, rr;
        SwizzledComponents<0, 1> xy, rg;
        SwizzledComponents<1, 0> yx, gr;
        SwizzledComponents<1, 1> yy, gg;

        SwizzledComponents<0, 0, 0> xxx, rrr;
        SwizzledComponents<0, 0, 1> xxy, rrg;
        SwizzledComponents<0, 1, 0> xyx, rgr;
        SwizzledComponents<0, 1, 1> xyy, rgg;
        SwizzledComponents<1, 0, 0> yxx, grr;
        SwizzledComponents<1, 0, 1> yxy, grg;
        SwizzledComponents<1, 1, 0> yyx, ggr;
        SwizzledComponents<1, 1, 1> yyy, ggg;

        SwizzledComponents<0, 0, 0, 0> xxxx, rrrr;
        SwizzledComponents<0, 0, 0, 1> xxxy, rrrg;
        SwizzledComponents<0, 0, 1, 0> xxyx, rrgr;
        SwizzledComponents<0, 0, 1, 1> xxyy, rrgg;
        SwizzledComponents<0, 1, 0, 0> xyxx, rgrr;
        SwizzledComponents<0, 1, 0, 1> xyxy, rgrg;
        SwizzledComponents<0, 1, 1, 0> xyyx, rggr;
        SwizzledComponents<0, 1, 1, 1> xyyy, rggg;

        SwizzledComponents<1, 0, 0, 0> yxxx, grrr;
        SwizzledComponents<1, 0, 0, 1> yxxy, grrg;
        SwizzledComponents<1, 0, 1, 0> yxyx, grgr;
        SwizzledComponents<1, 0, 1, 1> yxyy, grgg;
        SwizzledComponents<1, 1, 0, 0> yyxx, ggrr;
        SwizzledComponents<1, 1, 0, 1> yyxy, ggrg;
        SwizzledComponents<1, 1, 1, 0> yyyx, gggr;
        SwizzledComponents<1, 1, 1, 1> yyyy, gggg;
        /// @endcond
        /// @}
    };
};

/**
 * @brief Specialized 3D vector base class.
 *
 * Takes a Vec type that accepts size_t and type as parameters. This is
 * passed along to the Swizzled class.
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
    /**
     * @name default constructors
     * @{
     */
    constexpr VecBase() = default;
    constexpr VecBase(const VecBase&) = default;
    constexpr VecBase(VecBase&&) = default;
    ///@}

    union
    {
        std::array<T, N> m_components = {}; ///< array of components

        /**
         * @name Swizzled permutatons of {x, y, z} or {r, g, b} of max size
         * 4 as members.
         * @{
         */
        /// @cond DO_NOT_DOCUMENT
        SingleComponent<0> x, r;
        SingleComponent<1> y, g;
        SingleComponent<2> z, b;

        SwizzledComponents<0, 0> xx, rr;
        SwizzledComponents<0, 1> xy, rb;
        SwizzledComponents<0, 2> xz, rg;
        SwizzledComponents<1, 0> yx, gr;
        SwizzledComponents<1, 1> yy, gg;
        SwizzledComponents<1, 2> yz, gb;
        SwizzledComponents<2, 0> zx, br;
        SwizzledComponents<2, 1> zy, bg;
        SwizzledComponents<2, 2> zz, bb;

        SwizzledComponents<0, 0, 0> xxx, rrr;
        SwizzledComponents<0, 0, 1> xxy, rrg;
        SwizzledComponents<0, 0, 2> xxz, rrb;
        SwizzledComponents<0, 1, 0> xyx, rgr;
        SwizzledComponents<0, 1, 1> xyy, rgg;
        SwizzledComponents<0, 1, 2> xyz, rgb;
        SwizzledComponents<0, 2, 0> xzx, rbr;
        SwizzledComponents<0, 2, 1> xzy, rbg;

        SwizzledComponents<0, 2, 2> xzz, rbb;
        SwizzledComponents<1, 0, 0> yxx, grr;
        SwizzledComponents<1, 0, 1> yxy, grg;
        SwizzledComponents<1, 0, 2> yxz, grb;
        SwizzledComponents<1, 1, 0> yyx, ggr;
        SwizzledComponents<1, 1, 1> yyy, ggg;
        SwizzledComponents<1, 1, 2> yyz, ggb;
        SwizzledComponents<1, 2, 0> yzx, gbr;

        SwizzledComponents<1, 2, 1> yzy, gbg;
        SwizzledComponents<1, 2, 2> yzz, gbb;
        SwizzledComponents<2, 0, 0> zxx, brr;
        SwizzledComponents<2, 0, 1> zxy, brg;
        SwizzledComponents<2, 0, 2> zxz, brb;
        SwizzledComponents<2, 1, 0> zyx, bgr;
        SwizzledComponents<2, 1, 1> zyy, bgg;
        SwizzledComponents<2, 1, 2> zyz, bgb;

        SwizzledComponents<2, 2, 0> zzx, bbr;
        SwizzledComponents<2, 2, 1> zzy, bbg;
        SwizzledComponents<2, 2, 2> zzz, bbb;
        SwizzledComponents<0, 0, 0, 0> xxxx, rrrr;
        SwizzledComponents<0, 0, 0, 1> xxxy, rrrg;
        SwizzledComponents<0, 0, 0, 2> xxxz, rrrb;
        SwizzledComponents<0, 0, 1, 0> xxyx, rrgr;
        SwizzledComponents<0, 0, 1, 1> xxyy, rrgg;

        SwizzledComponents<0, 0, 1, 2> xxyz, rrgb;
        SwizzledComponents<0, 0, 2, 0> xxzx, rrbr;
        SwizzledComponents<0, 0, 2, 1> xxzy, rrbg;
        SwizzledComponents<0, 0, 2, 2> xxzz, rrbb;
        SwizzledComponents<0, 1, 0, 0> xyxx, rgrr;
        SwizzledComponents<0, 1, 0, 1> xyxy, rgrg;
        SwizzledComponents<0, 1, 0, 2> xyxz, rgrb;
        SwizzledComponents<0, 1, 1, 0> xyyx, rggr;

        SwizzledComponents<0, 1, 1, 1> xyyy, rggg;
        SwizzledComponents<0, 1, 1, 2> xyyz, rggb;
        SwizzledComponents<0, 1, 2, 0> xyzx, rgbr;
        SwizzledComponents<0, 1, 2, 1> xyzy, rgbg;
        SwizzledComponents<0, 1, 2, 2> xyzz, rgbb;
        SwizzledComponents<0, 2, 0, 0> xzxx, rbrr;
        SwizzledComponents<0, 2, 0, 1> xzxy, rbrg;
        SwizzledComponents<0, 2, 0, 2> xzxz, rbrb;

        SwizzledComponents<0, 2, 1, 0> xzyx, rbgr;
        SwizzledComponents<0, 2, 1, 1> xzyy, rbgg;
        SwizzledComponents<0, 2, 1, 2> xzyz, rbgb;
        SwizzledComponents<0, 2, 2, 0> xzzx, rbbr;
        SwizzledComponents<0, 2, 2, 1> xzzy, rbbg;
        SwizzledComponents<0, 2, 2, 2> xzzz, rbbb;
        SwizzledComponents<1, 0, 0, 0> yxxx, grrr;
        SwizzledComponents<1, 0, 0, 1> yxxy, grrg;

        SwizzledComponents<1, 0, 0, 2> yxxz, grrb;
        SwizzledComponents<1, 0, 1, 0> yxyx, grgr;
        SwizzledComponents<1, 0, 1, 1> yxyy, grgg;
        SwizzledComponents<1, 0, 1, 2> yxyz, grgb;
        SwizzledComponents<1, 0, 2, 0> yxzx, grbr;
        SwizzledComponents<1, 0, 2, 1> yxzy, grbg;
        SwizzledComponents<1, 0, 2, 2> yxzz, grbb;
        SwizzledComponents<1, 1, 0, 0> yyxx, ggrr;

        SwizzledComponents<1, 1, 0, 1> yyxy, ggrg;
        SwizzledComponents<1, 1, 0, 2> yyxz, ggrb;
        SwizzledComponents<1, 1, 1, 0> yyyx, gggr;
        SwizzledComponents<1, 1, 1, 1> yyyy, gggg;
        SwizzledComponents<1, 1, 1, 2> yyyz, gggb;
        SwizzledComponents<1, 1, 2, 0> yyzx, ggbr;
        SwizzledComponents<1, 1, 2, 1> yyzy, ggbg;
        SwizzledComponents<1, 1, 2, 2> yyzz, ggbb;

        SwizzledComponents<1, 2, 0, 0> yzxx, gbrr;
        SwizzledComponents<1, 2, 0, 1> yzxy, gbrg;
        SwizzledComponents<1, 2, 0, 2> yzxz, gbrb;
        SwizzledComponents<1, 2, 1, 0> yzyx, gbgr;
        SwizzledComponents<1, 2, 1, 1> yzyy, gbgg;
        SwizzledComponents<1, 2, 1, 2> yzyz, gbgb;
        SwizzledComponents<1, 2, 2, 0> yzzx, gbbr;
        SwizzledComponents<1, 2, 2, 1> yzzy, gbbg;

        SwizzledComponents<1, 2, 2, 2> yzzz, gbbb;
        SwizzledComponents<2, 0, 0, 0> zxxx, brrr;
        SwizzledComponents<2, 0, 0, 1> zxxy, brrg;
        SwizzledComponents<2, 0, 0, 2> zxxz, brrb;
        SwizzledComponents<2, 0, 1, 0> zxyx, brgr;
        SwizzledComponents<2, 0, 1, 1> zxyy, brgg;
        SwizzledComponents<2, 0, 1, 2> zxyz, brgb;
        SwizzledComponents<2, 0, 2, 0> zxzx, brbr;

        SwizzledComponents<2, 0, 2, 1> zxzy, brbg;
        SwizzledComponents<2, 0, 2, 2> zxzz, brbb;
        SwizzledComponents<2, 1, 0, 0> zyxx, bgrr;
        SwizzledComponents<2, 1, 0, 1> zyxy, bgrg;
        SwizzledComponents<2, 1, 0, 2> zyxz, bgrb;
        SwizzledComponents<2, 1, 1, 0> zyyx, bggr;
        SwizzledComponents<2, 1, 1, 1> zyyy, bggg;
        SwizzledComponents<2, 1, 1, 2> zyyz, bggb;

        SwizzledComponents<2, 1, 2, 0> zyzx, bgbr;
        SwizzledComponents<2, 1, 2, 1> zyzy, bgbg;
        SwizzledComponents<2, 1, 2, 2> zyzz, bgbb;
        SwizzledComponents<2, 2, 0, 0> zzxx, bbrr;
        SwizzledComponents<2, 2, 0, 1> zzxy, bbrg;
        SwizzledComponents<2, 2, 0, 2> zzxz, bbrb;
        SwizzledComponents<2, 2, 1, 0> zzyx, bbgr;
        SwizzledComponents<2, 2, 1, 1> zzyy, bbgg;

        SwizzledComponents<2, 2, 1, 2> zzyz, bbgb;
        SwizzledComponents<2, 2, 2, 0> zzzx, bbbr;
        SwizzledComponents<2, 2, 2, 1> zzzy, bbbg;
        SwizzledComponents<2, 2, 2, 2> zzzz, bbbb;
        /// @endcond
        /// @}
    };
};

/**
 * @brief Specialized 4D vector base class.
 *
 * Takes a Vec type that accepts size_t and type as parameters. This is
 * passed along to the Swizzled class.
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
    /**
     * @name default constructors
     * @{
     */
    constexpr VecBase() = default;
    constexpr VecBase(const VecBase&) = default;
    constexpr VecBase(VecBase&&) = default;
    ///@}

    union
    {
        std::array<T, N> m_components = {}; ///< array of components

        /**
         * @name Swizzled permutatons of {x, y, z, w} or {r, g, b, a} of max
         * size 4.
         * @{
         */
        /// @cond DO_NOT_DOCUMENT
        SingleComponent<0> x, r;
        SingleComponent<1> y, g;
        SingleComponent<2> z, b;
        SingleComponent<3> w, a;

        SwizzledComponents<0, 0> xx, rr;
        SwizzledComponents<0, 1> xy, rg;
        SwizzledComponents<0, 2> xz, rb;
        SwizzledComponents<0, 3> xw, ra;
        SwizzledComponents<1, 0> yx, gr;
        SwizzledComponents<1, 1> yy, gg;
        SwizzledComponents<1, 2> yz, gb;
        SwizzledComponents<1, 3> yw, ga;

        SwizzledComponents<2, 0> zx, br;
        SwizzledComponents<2, 1> zy, bg;
        SwizzledComponents<2, 2> zz, bb;
        SwizzledComponents<2, 3> zw, ba;
        SwizzledComponents<3, 0> wx, ar;
        SwizzledComponents<3, 1> wy, ag;
        SwizzledComponents<3, 2> wz, ab;
        SwizzledComponents<3, 3> ww, aa;

        SwizzledComponents<0, 0, 0> xxx, rrr;
        SwizzledComponents<0, 0, 1> xxy, rrg;
        SwizzledComponents<0, 0, 2> xxz, rrb;
        SwizzledComponents<0, 0, 3> xxw, rra;
        SwizzledComponents<0, 1, 0> xyx, rgr;
        SwizzledComponents<0, 1, 1> xyy, rgg;
        SwizzledComponents<0, 1, 2> xyz, rgb;
        SwizzledComponents<0, 1, 3> xyw, rga;

        SwizzledComponents<0, 2, 0> xzx, rbr;
        SwizzledComponents<0, 2, 1> xzy, rbg;
        SwizzledComponents<0, 2, 2> xzz, rbb;
        SwizzledComponents<0, 2, 3> xzw, rba;
        SwizzledComponents<0, 3, 0> xwx, rar;
        SwizzledComponents<0, 3, 1> xwy, rag;
        SwizzledComponents<0, 3, 2> xwz, rab;
        SwizzledComponents<0, 3, 3> xww, raa;

        SwizzledComponents<1, 0, 0> yxx, grr;
        SwizzledComponents<1, 0, 1> yxy, grg;
        SwizzledComponents<1, 0, 2> yxz, grb;
        SwizzledComponents<1, 0, 3> yxw, gra;
        SwizzledComponents<1, 1, 0> yyx, ggr;
        SwizzledComponents<1, 1, 1> yyy, ggg;
        SwizzledComponents<1, 1, 2> yyz, ggb;
        SwizzledComponents<1, 1, 3> yyw, gga;

        SwizzledComponents<1, 2, 0> yzx, gbr;
        SwizzledComponents<1, 2, 1> yzy, gbg;
        SwizzledComponents<1, 2, 2> yzz, gbb;
        SwizzledComponents<1, 2, 3> yzw, gba;
        SwizzledComponents<1, 3, 0> ywx, gar;
        SwizzledComponents<1, 3, 1> ywy, gag;
        SwizzledComponents<1, 3, 2> ywz, gab;
        SwizzledComponents<1, 3, 3> yww, gaa;

        SwizzledComponents<2, 0, 0> zxx, brr;
        SwizzledComponents<2, 0, 1> zxy, brg;
        SwizzledComponents<2, 0, 2> zxz, brb;
        SwizzledComponents<2, 0, 3> zxw, bra;
        SwizzledComponents<2, 1, 0> zyx, bgr;
        SwizzledComponents<2, 1, 1> zyy, bgg;
        SwizzledComponents<2, 1, 2> zyz, bgb;
        SwizzledComponents<2, 1, 3> zyw, bga;

        SwizzledComponents<2, 2, 0> zzx, bbr;
        SwizzledComponents<2, 2, 1> zzy, bbg;
        SwizzledComponents<2, 2, 2> zzz, bbb;
        SwizzledComponents<2, 2, 3> zzw, bba;
        SwizzledComponents<2, 3, 0> zwx, bar;
        SwizzledComponents<2, 3, 1> zwy, bag;
        SwizzledComponents<2, 3, 2> zwz, bab;
        SwizzledComponents<2, 3, 3> zww, baa;

        SwizzledComponents<3, 0, 0> wxx, arr;
        SwizzledComponents<3, 0, 1> wxy, arg;
        SwizzledComponents<3, 0, 2> wxz, arb;
        SwizzledComponents<3, 0, 3> wxw, ara;
        SwizzledComponents<3, 1, 0> wyx, agr;
        SwizzledComponents<3, 1, 1> wyy, agg;
        SwizzledComponents<3, 1, 2> wyz, agb;
        SwizzledComponents<3, 1, 3> wyw, aga;

        SwizzledComponents<3, 2, 0> wzx, abr;
        SwizzledComponents<3, 2, 1> wzy, abg;
        SwizzledComponents<3, 2, 2> wzz, abb;
        SwizzledComponents<3, 2, 3> wzw, aba;
        SwizzledComponents<3, 3, 0> wwx, aar;
        SwizzledComponents<3, 3, 1> wwy, aag;
        SwizzledComponents<3, 3, 2> wwz, aab;
        SwizzledComponents<3, 3, 3> www, aaa;

        SwizzledComponents<0, 0, 0, 0> xxxx, rrrr;
        SwizzledComponents<0, 0, 0, 1> xxxy, rrrg;
        SwizzledComponents<0, 0, 0, 2> xxxz, rrrb;
        SwizzledComponents<0, 0, 0, 3> xxxw, rrra;
        SwizzledComponents<0, 0, 1, 0> xxyx, rrgr;
        SwizzledComponents<0, 0, 1, 1> xxyy, rrgg;
        SwizzledComponents<0, 0, 1, 2> xxyz, rrgb;
        SwizzledComponents<0, 0, 1, 3> xxyw, rrga;

        SwizzledComponents<0, 0, 2, 0> xxzx, rrbr;
        SwizzledComponents<0, 0, 2, 1> xxzy, rrbg;
        SwizzledComponents<0, 0, 2, 2> xxzz, rrbb;
        SwizzledComponents<0, 0, 2, 3> xxzw, rrba;
        SwizzledComponents<0, 0, 3, 0> xxwx, rrar;
        SwizzledComponents<0, 0, 3, 1> xxwy, rrag;
        SwizzledComponents<0, 0, 3, 2> xxwz, rrab;
        SwizzledComponents<0, 0, 3, 3> xxww, rraa;

        SwizzledComponents<0, 1, 0, 0> xyxx, rgrr;
        SwizzledComponents<0, 1, 0, 1> xyxy, rgrg;
        SwizzledComponents<0, 1, 0, 2> xyxz, rgrb;
        SwizzledComponents<0, 1, 0, 3> xyxw, rgra;
        SwizzledComponents<0, 1, 1, 0> xyyx, rggr;
        SwizzledComponents<0, 1, 1, 1> xyyy, rggg;
        SwizzledComponents<0, 1, 1, 2> xyyz, rggb;
        SwizzledComponents<0, 1, 1, 3> xyyw, rgga;

        SwizzledComponents<0, 1, 2, 0> xyzx, rgbr;
        SwizzledComponents<0, 1, 2, 1> xyzy, rgbg;
        SwizzledComponents<0, 1, 2, 2> xyzz, rgbb;
        SwizzledComponents<0, 1, 2, 3> xyzw, rgba;
        SwizzledComponents<0, 1, 3, 0> xywx, rgar;
        SwizzledComponents<0, 1, 3, 1> xywy, rgag;
        SwizzledComponents<0, 1, 3, 2> xywz, rgab;
        SwizzledComponents<0, 1, 3, 3> xyww, rgaa;

        SwizzledComponents<0, 2, 0, 0> xzxx, rbrr;
        SwizzledComponents<0, 2, 0, 1> xzxy, rbrg;
        SwizzledComponents<0, 2, 0, 2> xzxz, rbrb;
        SwizzledComponents<0, 2, 0, 3> xzxw, rbra;
        SwizzledComponents<0, 2, 1, 0> xzyx, rbgr;
        SwizzledComponents<0, 2, 1, 1> xzyy, rbgg;
        SwizzledComponents<0, 2, 1, 2> xzyz, rbgb;
        SwizzledComponents<0, 2, 1, 3> xzyw, rbga;

        SwizzledComponents<0, 2, 2, 0> xzzx, rbbr;
        SwizzledComponents<0, 2, 2, 1> xzzy, rbbg;
        SwizzledComponents<0, 2, 2, 2> xzzz, rbbb;
        SwizzledComponents<0, 2, 2, 3> xzzw, rbba;
        SwizzledComponents<0, 2, 3, 0> xzwx, rbar;
        SwizzledComponents<0, 2, 3, 1> xzwy, rbag;
        SwizzledComponents<0, 2, 3, 2> xzwz, rbab;
        SwizzledComponents<0, 2, 3, 3> xzww, rbaa;

        SwizzledComponents<0, 3, 0, 0> xwxx, rarr;
        SwizzledComponents<0, 3, 0, 1> xwxy, rarg;
        SwizzledComponents<0, 3, 0, 2> xwxz, rarb;
        SwizzledComponents<0, 3, 0, 3> xwxw, rara;
        SwizzledComponents<0, 3, 1, 0> xwyx, ragr;
        SwizzledComponents<0, 3, 1, 1> xwyy, ragg;
        SwizzledComponents<0, 3, 1, 2> xwyz, ragb;
        SwizzledComponents<0, 3, 1, 3> xwyw, raga;

        SwizzledComponents<0, 3, 2, 0> xwzx, rabr;
        SwizzledComponents<0, 3, 2, 1> xwzy, rabg;
        SwizzledComponents<0, 3, 2, 2> xwzz, rabb;
        SwizzledComponents<0, 3, 2, 3> xwzw, raba;
        SwizzledComponents<0, 3, 3, 0> xwwx, raar;
        SwizzledComponents<0, 3, 3, 1> xwwy, raag;
        SwizzledComponents<0, 3, 3, 2> xwwz, raab;
        SwizzledComponents<0, 3, 3, 3> xwww, raaa;

        SwizzledComponents<1, 0, 0, 0> yxxx, grrr;
        SwizzledComponents<1, 0, 0, 1> yxxy, grrg;
        SwizzledComponents<1, 0, 0, 2> yxxz, grrb;
        SwizzledComponents<1, 0, 0, 3> yxxw, grra;
        SwizzledComponents<1, 0, 1, 0> yxyx, grgr;
        SwizzledComponents<1, 0, 1, 1> yxyy, grgg;
        SwizzledComponents<1, 0, 1, 2> yxyz, grgb;
        SwizzledComponents<1, 0, 1, 3> yxyw, grga;

        SwizzledComponents<1, 0, 2, 0> yxzx, grbr;
        SwizzledComponents<1, 0, 2, 1> yxzy, grbg;
        SwizzledComponents<1, 0, 2, 2> yxzz, grbb;
        SwizzledComponents<1, 0, 2, 3> yxzw, grba;
        SwizzledComponents<1, 0, 3, 0> yxwx, grar;
        SwizzledComponents<1, 0, 3, 1> yxwy, grag;
        SwizzledComponents<1, 0, 3, 2> yxwz, grab;
        SwizzledComponents<1, 0, 3, 3> yxww, graa;

        SwizzledComponents<1, 1, 0, 0> yyxx, ggrr;
        SwizzledComponents<1, 1, 0, 1> yyxy, ggrg;
        SwizzledComponents<1, 1, 0, 2> yyxz, ggrb;
        SwizzledComponents<1, 1, 0, 3> yyxw, ggra;
        SwizzledComponents<1, 1, 1, 0> yyyx, gggr;
        SwizzledComponents<1, 1, 1, 1> yyyy, gggg;
        SwizzledComponents<1, 1, 1, 2> yyyz, gggb;
        SwizzledComponents<1, 1, 1, 3> yyyw, ggga;

        SwizzledComponents<1, 1, 2, 0> yyzx, ggbr;
        SwizzledComponents<1, 1, 2, 1> yyzy, ggbg;
        SwizzledComponents<1, 1, 2, 2> yyzz, ggbb;
        SwizzledComponents<1, 1, 2, 3> yyzw, ggba;
        SwizzledComponents<1, 1, 3, 0> yywx, ggar;
        SwizzledComponents<1, 1, 3, 1> yywy, ggag;
        SwizzledComponents<1, 1, 3, 2> yywz, ggab;
        SwizzledComponents<1, 1, 3, 3> yyww, ggaa;

        SwizzledComponents<1, 2, 0, 0> yzxx, gbrr;
        SwizzledComponents<1, 2, 0, 1> yzxy, gbrg;
        SwizzledComponents<1, 2, 0, 2> yzxz, gbrb;
        SwizzledComponents<1, 2, 0, 3> yzxw, gbra;
        SwizzledComponents<1, 2, 1, 0> yzyx, gbgr;
        SwizzledComponents<1, 2, 1, 1> yzyy, gbgg;
        SwizzledComponents<1, 2, 1, 2> yzyz, gbgb;
        SwizzledComponents<1, 2, 1, 3> yzyw, gbga;

        SwizzledComponents<1, 2, 2, 0> yzzx, gbbr;
        SwizzledComponents<1, 2, 2, 1> yzzy, gbbg;
        SwizzledComponents<1, 2, 2, 2> yzzz, gbbb;
        SwizzledComponents<1, 2, 2, 3> yzzw, gbba;
        SwizzledComponents<1, 2, 3, 0> yzwx, gbar;
        SwizzledComponents<1, 2, 3, 1> yzwy, gbag;
        SwizzledComponents<1, 2, 3, 2> yzwz, gbab;
        SwizzledComponents<1, 2, 3, 3> yzww, gbaa;

        SwizzledComponents<1, 3, 0, 0> ywxx, garr;
        SwizzledComponents<1, 3, 0, 1> ywxy, garg;
        SwizzledComponents<1, 3, 0, 2> ywxz, garb;
        SwizzledComponents<1, 3, 0, 3> ywxw, gara;
        SwizzledComponents<1, 3, 1, 0> ywyx, gagr;
        SwizzledComponents<1, 3, 1, 1> ywyy, gagg;
        SwizzledComponents<1, 3, 1, 2> ywyz, gagb;
        SwizzledComponents<1, 3, 1, 3> ywyw, gaga;

        SwizzledComponents<1, 3, 2, 0> ywzx, gabr;
        SwizzledComponents<1, 3, 2, 1> ywzy, gabg;
        SwizzledComponents<1, 3, 2, 2> ywzz, gabb;
        SwizzledComponents<1, 3, 2, 3> ywzw, gaba;
        SwizzledComponents<1, 3, 3, 0> ywwx, gaar;
        SwizzledComponents<1, 3, 3, 1> ywwy, gaag;
        SwizzledComponents<1, 3, 3, 2> ywwz, gaab;
        SwizzledComponents<1, 3, 3, 3> ywww, gaaa;

        SwizzledComponents<2, 0, 0, 0> zxxx, brrr;
        SwizzledComponents<2, 0, 0, 1> zxxy, brrg;
        SwizzledComponents<2, 0, 0, 2> zxxz, brrb;
        SwizzledComponents<2, 0, 0, 3> zxxw, brra;
        SwizzledComponents<2, 0, 1, 0> zxyx, brgr;
        SwizzledComponents<2, 0, 1, 1> zxyy, brgg;
        SwizzledComponents<2, 0, 1, 2> zxyz, brgb;
        SwizzledComponents<2, 0, 1, 3> zxyw, brga;

        SwizzledComponents<2, 0, 2, 0> zxzx, brbr;
        SwizzledComponents<2, 0, 2, 1> zxzy, brbg;
        SwizzledComponents<2, 0, 2, 2> zxzz, brbb;
        SwizzledComponents<2, 0, 2, 3> zxzw, brba;
        SwizzledComponents<2, 0, 3, 0> zxwx, brar;
        SwizzledComponents<2, 0, 3, 1> zxwy, brag;
        SwizzledComponents<2, 0, 3, 2> zxwz, brab;
        SwizzledComponents<2, 0, 3, 3> zxww, braa;

        SwizzledComponents<2, 1, 0, 0> zyxx, bgrr;
        SwizzledComponents<2, 1, 0, 1> zyxy, bgrg;
        SwizzledComponents<2, 1, 0, 2> zyxz, bgrb;
        SwizzledComponents<2, 1, 0, 3> zyxw, bgra;
        SwizzledComponents<2, 1, 1, 0> zyyx, bggr;
        SwizzledComponents<2, 1, 1, 1> zyyy, bggg;
        SwizzledComponents<2, 1, 1, 2> zyyz, bggb;
        SwizzledComponents<2, 1, 1, 3> zyyw, bgga;

        SwizzledComponents<2, 1, 2, 0> zyzx, bgbr;
        SwizzledComponents<2, 1, 2, 1> zyzy, bgbg;
        SwizzledComponents<2, 1, 2, 2> zyzz, bgbb;
        SwizzledComponents<2, 1, 2, 3> zyzw, bgba;
        SwizzledComponents<2, 1, 3, 0> zywx, bgar;
        SwizzledComponents<2, 1, 3, 1> zywy, bgag;
        SwizzledComponents<2, 1, 3, 2> zywz, bgab;
        SwizzledComponents<2, 1, 3, 3> zyww, bgaa;

        SwizzledComponents<2, 2, 0, 0> zzxx, bbrr;
        SwizzledComponents<2, 2, 0, 1> zzxy, bbrg;
        SwizzledComponents<2, 2, 0, 2> zzxz, bbrb;
        SwizzledComponents<2, 2, 0, 3> zzxw, bbra;
        SwizzledComponents<2, 2, 1, 0> zzyx, bbgr;
        SwizzledComponents<2, 2, 1, 1> zzyy, bbgg;
        SwizzledComponents<2, 2, 1, 2> zzyz, bbgb;
        SwizzledComponents<2, 2, 1, 3> zzyw, bbga;

        SwizzledComponents<2, 2, 2, 0> zzzx, bbbr;
        SwizzledComponents<2, 2, 2, 1> zzzy, bbbg;
        SwizzledComponents<2, 2, 2, 2> zzzz, bbbb;
        SwizzledComponents<2, 2, 2, 3> zzzw, bbba;
        SwizzledComponents<2, 2, 3, 0> zzwx, bbar;
        SwizzledComponents<2, 2, 3, 1> zzwy, bbag;
        SwizzledComponents<2, 2, 3, 2> zzwz, bbab;
        SwizzledComponents<2, 2, 3, 3> zzww, bbaa;

        SwizzledComponents<2, 3, 0, 0> zwxx, barr;
        SwizzledComponents<2, 3, 0, 1> zwxy, barg;
        SwizzledComponents<2, 3, 0, 2> zwxz, barb;
        SwizzledComponents<2, 3, 0, 3> zwxw, bara;
        SwizzledComponents<2, 3, 1, 0> zwyx, bagr;
        SwizzledComponents<2, 3, 1, 1> zwyy, bagg;
        SwizzledComponents<2, 3, 1, 2> zwyz, bagb;
        SwizzledComponents<2, 3, 1, 3> zwyw, baga;

        SwizzledComponents<2, 3, 2, 0> zwzx, babr;
        SwizzledComponents<2, 3, 2, 1> zwzy, babg;
        SwizzledComponents<2, 3, 2, 2> zwzz, babb;
        SwizzledComponents<2, 3, 2, 3> zwzw, baba;
        SwizzledComponents<2, 3, 3, 0> zwwx, baar;
        SwizzledComponents<2, 3, 3, 1> zwwy, baag;
        SwizzledComponents<2, 3, 3, 2> zwwz, baab;
        SwizzledComponents<2, 3, 3, 3> zwww, baaa;

        SwizzledComponents<3, 0, 0, 0> wxxx, arrr;
        SwizzledComponents<3, 0, 0, 1> wxxy, arrg;
        SwizzledComponents<3, 0, 0, 2> wxxz, arrb;
        SwizzledComponents<3, 0, 0, 3> wxxw, arra;
        SwizzledComponents<3, 0, 1, 0> wxyx, argr;
        SwizzledComponents<3, 0, 1, 1> wxyy, argg;
        SwizzledComponents<3, 0, 1, 2> wxyz, argb;
        SwizzledComponents<3, 0, 1, 3> wxyw, arga;

        SwizzledComponents<3, 0, 2, 0> wxzx, arbr;
        SwizzledComponents<3, 0, 2, 1> wxzy, arbg;
        SwizzledComponents<3, 0, 2, 2> wxzz, arbb;
        SwizzledComponents<3, 0, 2, 3> wxzw, arba;
        SwizzledComponents<3, 0, 3, 0> wxwx, arar;
        SwizzledComponents<3, 0, 3, 1> wxwy, arag;
        SwizzledComponents<3, 0, 3, 2> wxwz, arab;
        SwizzledComponents<3, 0, 3, 3> wxww, araa;

        SwizzledComponents<3, 1, 0, 0> wyxx, agrr;
        SwizzledComponents<3, 1, 0, 1> wyxy, agrg;
        SwizzledComponents<3, 1, 0, 2> wyxz, agrb;
        SwizzledComponents<3, 1, 0, 3> wyxw, agra;
        SwizzledComponents<3, 1, 1, 0> wyyx, aggr;
        SwizzledComponents<3, 1, 1, 1> wyyy, aggg;
        SwizzledComponents<3, 1, 1, 2> wyyz, aggb;
        SwizzledComponents<3, 1, 1, 3> wyyw, agga;

        SwizzledComponents<3, 1, 2, 0> wyzx, agbr;
        SwizzledComponents<3, 1, 2, 1> wyzy, agbg;
        SwizzledComponents<3, 1, 2, 2> wyzz, agbb;
        SwizzledComponents<3, 1, 2, 3> wyzw, agba;
        SwizzledComponents<3, 1, 3, 0> wywx, agar;
        SwizzledComponents<3, 1, 3, 1> wywy, agag;
        SwizzledComponents<3, 1, 3, 2> wywz, agab;
        SwizzledComponents<3, 1, 3, 3> wyww, agaa;

        SwizzledComponents<3, 2, 0, 0> wzxx, abrr;
        SwizzledComponents<3, 2, 0, 1> wzxy, abrg;
        SwizzledComponents<3, 2, 0, 2> wzxz, abrb;
        SwizzledComponents<3, 2, 0, 3> wzxw, abra;
        SwizzledComponents<3, 2, 1, 0> wzyx, abgr;
        SwizzledComponents<3, 2, 1, 1> wzyy, abgg;
        SwizzledComponents<3, 2, 1, 2> wzyz, abgb;
        SwizzledComponents<3, 2, 1, 3> wzyw, abga;

        SwizzledComponents<3, 2, 2, 0> wzzx, abbr;
        SwizzledComponents<3, 2, 2, 1> wzzy, abbg;
        SwizzledComponents<3, 2, 2, 2> wzzz, abbb;
        SwizzledComponents<3, 2, 2, 3> wzzw, abba;
        SwizzledComponents<3, 2, 3, 0> wzwx, abar;
        SwizzledComponents<3, 2, 3, 1> wzwy, abag;
        SwizzledComponents<3, 2, 3, 2> wzwz, abab;
        SwizzledComponents<3, 2, 3, 3> wzww, abaa;

        SwizzledComponents<3, 3, 0, 0> wwxx, aarr;
        SwizzledComponents<3, 3, 0, 1> wwxy, aarg;
        SwizzledComponents<3, 3, 0, 2> wwxz, aarb;
        SwizzledComponents<3, 3, 0, 3> wwxw, aara;
        SwizzledComponents<3, 3, 1, 0> wwyx, aagr;
        SwizzledComponents<3, 3, 1, 1> wwyy, aagg;
        SwizzledComponents<3, 3, 1, 2> wwyz, aagb;
        SwizzledComponents<3, 3, 1, 3> wwyw, aaga;

        SwizzledComponents<3, 3, 2, 0> wwzx, aabr;
        SwizzledComponents<3, 3, 2, 1> wwzy, aabg;
        SwizzledComponents<3, 3, 2, 2> wwzz, aabb;
        SwizzledComponents<3, 3, 2, 3> wwzw, aaba;
        SwizzledComponents<3, 3, 3, 0> wwwx, aaar;
        SwizzledComponents<3, 3, 3, 1> wwwy, aaag;
        SwizzledComponents<3, 3, 3, 2> wwwz, aaab;
        SwizzledComponents<3, 3, 3, 3> wwww, aaaa;
        /// @endcond
        /// @}
    };
};

} // namespace asciirast::math
