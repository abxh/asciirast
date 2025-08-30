/**
 * @file VecBase.h
 * @brief File with definitions of the generic and specialized vector
 *        base classes
 */

#pragma once

#include <array>

#include "./Swizzled.hpp"

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
    requires(N > 1)
class VecBase
{
public:
    std::array<T, N> m_components = {}; ///< array of components

    /**
     * @name Default constructors / destructors
     * @{
     */
    constexpr ~VecBase() = default;
    constexpr VecBase() = default;
    constexpr VecBase(const VecBase&) = default;
    constexpr VecBase(VecBase&&) = default;
    /// @}
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
    using Component = SwizzledSingle<N, T, Index>;

    template<std::size_t... Indicies>
    using Components = Swizzled<Vec<sizeof...(Indicies), T>, N, T, Indicies...>;

public:
    /**
     * @name Default constructors / destructors
     * @{
     */
    constexpr ~VecBase() = default;
    constexpr VecBase() = default;
    constexpr VecBase(const VecBase&) = default;
    constexpr VecBase(VecBase&&) = default;
    /// @}

    union
    {
        std::array<T, N> m_components = {}; ///< array of components

        /**
         * @name Swizzled permutatons of {x, y}, {r, g} of max size 4 as
         * members.
         * @{
         */
        /// @cond DO_NOT_DOCUMENT
        Component<0> x, r;
        Component<1> y, g;

        Components<0, 0> xx, rr;
        Components<0, 1> xy, rg;
        Components<1, 0> yx, gr;
        Components<1, 1> yy, gg;

        Components<0, 0, 0> xxx, rrr;
        Components<0, 0, 1> xxy, rrg;
        Components<0, 1, 0> xyx, rgr;
        Components<0, 1, 1> xyy, rgg;
        Components<1, 0, 0> yxx, grr;
        Components<1, 0, 1> yxy, grg;
        Components<1, 1, 0> yyx, ggr;
        Components<1, 1, 1> yyy, ggg;

        Components<0, 0, 0, 0> xxxx, rrrr;
        Components<0, 0, 0, 1> xxxy, rrrg;
        Components<0, 0, 1, 0> xxyx, rrgr;
        Components<0, 0, 1, 1> xxyy, rrgg;
        Components<0, 1, 0, 0> xyxx, rgrr;
        Components<0, 1, 0, 1> xyxy, rgrg;
        Components<0, 1, 1, 0> xyyx, rggr;
        Components<0, 1, 1, 1> xyyy, rggg;

        Components<1, 0, 0, 0> yxxx, grrr;
        Components<1, 0, 0, 1> yxxy, grrg;
        Components<1, 0, 1, 0> yxyx, grgr;
        Components<1, 0, 1, 1> yxyy, grgg;
        Components<1, 1, 0, 0> yyxx, ggrr;
        Components<1, 1, 0, 1> yyxy, ggrg;
        Components<1, 1, 1, 0> yyyx, gggr;
        Components<1, 1, 1, 1> yyyy, gggg;
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
    using Component = SwizzledSingle<N, T, Index>;

    template<std::size_t... Indicies>
    using Components = Swizzled<Vec<sizeof...(Indicies), T>, N, T, Indicies...>;

public:
    /**
     * @name Default constructors / destructors
     * @{
     */
    constexpr ~VecBase() = default;
    constexpr VecBase() = default;
    constexpr VecBase(const VecBase&) = default;
    constexpr VecBase(VecBase&&) = default;
    /// @}

    union
    {
        std::array<T, N> m_components = {}; ///< array of components

        /**
         * @name Swizzled permutatons of {x, y, z} or {r, g, b} of max size
         * 4 as members.
         * @{
         */
        /// @cond DO_NOT_DOCUMENT
        Component<0> x, r;
        Component<1> y, g;
        Component<2> z, b;

        Components<0, 0> xx, rr;
        Components<0, 1> xy, rb;
        Components<0, 2> xz, rg;
        Components<1, 0> yx, gr;
        Components<1, 1> yy, gg;
        Components<1, 2> yz, gb;
        Components<2, 0> zx, br;
        Components<2, 1> zy, bg;
        Components<2, 2> zz, bb;

        Components<0, 0, 0> xxx, rrr;
        Components<0, 0, 1> xxy, rrg;
        Components<0, 0, 2> xxz, rrb;
        Components<0, 1, 0> xyx, rgr;
        Components<0, 1, 1> xyy, rgg;
        Components<0, 1, 2> xyz, rgb;
        Components<0, 2, 0> xzx, rbr;
        Components<0, 2, 1> xzy, rbg;

        Components<0, 2, 2> xzz, rbb;
        Components<1, 0, 0> yxx, grr;
        Components<1, 0, 1> yxy, grg;
        Components<1, 0, 2> yxz, grb;
        Components<1, 1, 0> yyx, ggr;
        Components<1, 1, 1> yyy, ggg;
        Components<1, 1, 2> yyz, ggb;
        Components<1, 2, 0> yzx, gbr;

        Components<1, 2, 1> yzy, gbg;
        Components<1, 2, 2> yzz, gbb;
        Components<2, 0, 0> zxx, brr;
        Components<2, 0, 1> zxy, brg;
        Components<2, 0, 2> zxz, brb;
        Components<2, 1, 0> zyx, bgr;
        Components<2, 1, 1> zyy, bgg;
        Components<2, 1, 2> zyz, bgb;

        Components<2, 2, 0> zzx, bbr;
        Components<2, 2, 1> zzy, bbg;
        Components<2, 2, 2> zzz, bbb;
        Components<0, 0, 0, 0> xxxx, rrrr;
        Components<0, 0, 0, 1> xxxy, rrrg;
        Components<0, 0, 0, 2> xxxz, rrrb;
        Components<0, 0, 1, 0> xxyx, rrgr;
        Components<0, 0, 1, 1> xxyy, rrgg;

        Components<0, 0, 1, 2> xxyz, rrgb;
        Components<0, 0, 2, 0> xxzx, rrbr;
        Components<0, 0, 2, 1> xxzy, rrbg;
        Components<0, 0, 2, 2> xxzz, rrbb;
        Components<0, 1, 0, 0> xyxx, rgrr;
        Components<0, 1, 0, 1> xyxy, rgrg;
        Components<0, 1, 0, 2> xyxz, rgrb;
        Components<0, 1, 1, 0> xyyx, rggr;

        Components<0, 1, 1, 1> xyyy, rggg;
        Components<0, 1, 1, 2> xyyz, rggb;
        Components<0, 1, 2, 0> xyzx, rgbr;
        Components<0, 1, 2, 1> xyzy, rgbg;
        Components<0, 1, 2, 2> xyzz, rgbb;
        Components<0, 2, 0, 0> xzxx, rbrr;
        Components<0, 2, 0, 1> xzxy, rbrg;
        Components<0, 2, 0, 2> xzxz, rbrb;

        Components<0, 2, 1, 0> xzyx, rbgr;
        Components<0, 2, 1, 1> xzyy, rbgg;
        Components<0, 2, 1, 2> xzyz, rbgb;
        Components<0, 2, 2, 0> xzzx, rbbr;
        Components<0, 2, 2, 1> xzzy, rbbg;
        Components<0, 2, 2, 2> xzzz, rbbb;
        Components<1, 0, 0, 0> yxxx, grrr;
        Components<1, 0, 0, 1> yxxy, grrg;

        Components<1, 0, 0, 2> yxxz, grrb;
        Components<1, 0, 1, 0> yxyx, grgr;
        Components<1, 0, 1, 1> yxyy, grgg;
        Components<1, 0, 1, 2> yxyz, grgb;
        Components<1, 0, 2, 0> yxzx, grbr;
        Components<1, 0, 2, 1> yxzy, grbg;
        Components<1, 0, 2, 2> yxzz, grbb;
        Components<1, 1, 0, 0> yyxx, ggrr;

        Components<1, 1, 0, 1> yyxy, ggrg;
        Components<1, 1, 0, 2> yyxz, ggrb;
        Components<1, 1, 1, 0> yyyx, gggr;
        Components<1, 1, 1, 1> yyyy, gggg;
        Components<1, 1, 1, 2> yyyz, gggb;
        Components<1, 1, 2, 0> yyzx, ggbr;
        Components<1, 1, 2, 1> yyzy, ggbg;
        Components<1, 1, 2, 2> yyzz, ggbb;

        Components<1, 2, 0, 0> yzxx, gbrr;
        Components<1, 2, 0, 1> yzxy, gbrg;
        Components<1, 2, 0, 2> yzxz, gbrb;
        Components<1, 2, 1, 0> yzyx, gbgr;
        Components<1, 2, 1, 1> yzyy, gbgg;
        Components<1, 2, 1, 2> yzyz, gbgb;
        Components<1, 2, 2, 0> yzzx, gbbr;
        Components<1, 2, 2, 1> yzzy, gbbg;

        Components<1, 2, 2, 2> yzzz, gbbb;
        Components<2, 0, 0, 0> zxxx, brrr;
        Components<2, 0, 0, 1> zxxy, brrg;
        Components<2, 0, 0, 2> zxxz, brrb;
        Components<2, 0, 1, 0> zxyx, brgr;
        Components<2, 0, 1, 1> zxyy, brgg;
        Components<2, 0, 1, 2> zxyz, brgb;
        Components<2, 0, 2, 0> zxzx, brbr;

        Components<2, 0, 2, 1> zxzy, brbg;
        Components<2, 0, 2, 2> zxzz, brbb;
        Components<2, 1, 0, 0> zyxx, bgrr;
        Components<2, 1, 0, 1> zyxy, bgrg;
        Components<2, 1, 0, 2> zyxz, bgrb;
        Components<2, 1, 1, 0> zyyx, bggr;
        Components<2, 1, 1, 1> zyyy, bggg;
        Components<2, 1, 1, 2> zyyz, bggb;

        Components<2, 1, 2, 0> zyzx, bgbr;
        Components<2, 1, 2, 1> zyzy, bgbg;
        Components<2, 1, 2, 2> zyzz, bgbb;
        Components<2, 2, 0, 0> zzxx, bbrr;
        Components<2, 2, 0, 1> zzxy, bbrg;
        Components<2, 2, 0, 2> zzxz, bbrb;
        Components<2, 2, 1, 0> zzyx, bbgr;
        Components<2, 2, 1, 1> zzyy, bbgg;

        Components<2, 2, 1, 2> zzyz, bbgb;
        Components<2, 2, 2, 0> zzzx, bbbr;
        Components<2, 2, 2, 1> zzzy, bbbg;
        Components<2, 2, 2, 2> zzzz, bbbb;
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
    using Component = SwizzledSingle<N, T, Index>;

    template<std::size_t... Indicies>
    using Components = Swizzled<Vec<sizeof...(Indicies), T>, N, T, Indicies...>;

public:
    /**
     * @name Default constructors / destructors
     * @{
     */
    constexpr ~VecBase() = default;
    constexpr VecBase() = default;
    constexpr VecBase(const VecBase&) = default;
    constexpr VecBase(VecBase&&) = default;
    /// @}

    union
    {
        std::array<T, N> m_components = {}; ///< array of components

        /**
         * @name Swizzled permutatons of {x, y, z, w} or {r, g, b, a} of max
         * size 4.
         * @{
         */
        /// @cond DO_NOT_DOCUMENT
        Component<0> x, r;
        Component<1> y, g;
        Component<2> z, b;
        Component<3> w, a;

        Components<0, 0> xx, rr;
        Components<0, 1> xy, rg;
        Components<0, 2> xz, rb;
        Components<0, 3> xw, ra;
        Components<1, 0> yx, gr;
        Components<1, 1> yy, gg;
        Components<1, 2> yz, gb;
        Components<1, 3> yw, ga;

        Components<2, 0> zx, br;
        Components<2, 1> zy, bg;
        Components<2, 2> zz, bb;
        Components<2, 3> zw, ba;
        Components<3, 0> wx, ar;
        Components<3, 1> wy, ag;
        Components<3, 2> wz, ab;
        Components<3, 3> ww, aa;

        Components<0, 0, 0> xxx, rrr;
        Components<0, 0, 1> xxy, rrg;
        Components<0, 0, 2> xxz, rrb;
        Components<0, 0, 3> xxw, rra;
        Components<0, 1, 0> xyx, rgr;
        Components<0, 1, 1> xyy, rgg;
        Components<0, 1, 2> xyz, rgb;
        Components<0, 1, 3> xyw, rga;

        Components<0, 2, 0> xzx, rbr;
        Components<0, 2, 1> xzy, rbg;
        Components<0, 2, 2> xzz, rbb;
        Components<0, 2, 3> xzw, rba;
        Components<0, 3, 0> xwx, rar;
        Components<0, 3, 1> xwy, rag;
        Components<0, 3, 2> xwz, rab;
        Components<0, 3, 3> xww, raa;

        Components<1, 0, 0> yxx, grr;
        Components<1, 0, 1> yxy, grg;
        Components<1, 0, 2> yxz, grb;
        Components<1, 0, 3> yxw, gra;
        Components<1, 1, 0> yyx, ggr;
        Components<1, 1, 1> yyy, ggg;
        Components<1, 1, 2> yyz, ggb;
        Components<1, 1, 3> yyw, gga;

        Components<1, 2, 0> yzx, gbr;
        Components<1, 2, 1> yzy, gbg;
        Components<1, 2, 2> yzz, gbb;
        Components<1, 2, 3> yzw, gba;
        Components<1, 3, 0> ywx, gar;
        Components<1, 3, 1> ywy, gag;
        Components<1, 3, 2> ywz, gab;
        Components<1, 3, 3> yww, gaa;

        Components<2, 0, 0> zxx, brr;
        Components<2, 0, 1> zxy, brg;
        Components<2, 0, 2> zxz, brb;
        Components<2, 0, 3> zxw, bra;
        Components<2, 1, 0> zyx, bgr;
        Components<2, 1, 1> zyy, bgg;
        Components<2, 1, 2> zyz, bgb;
        Components<2, 1, 3> zyw, bga;

        Components<2, 2, 0> zzx, bbr;
        Components<2, 2, 1> zzy, bbg;
        Components<2, 2, 2> zzz, bbb;
        Components<2, 2, 3> zzw, bba;
        Components<2, 3, 0> zwx, bar;
        Components<2, 3, 1> zwy, bag;
        Components<2, 3, 2> zwz, bab;
        Components<2, 3, 3> zww, baa;

        Components<3, 0, 0> wxx, arr;
        Components<3, 0, 1> wxy, arg;
        Components<3, 0, 2> wxz, arb;
        Components<3, 0, 3> wxw, ara;
        Components<3, 1, 0> wyx, agr;
        Components<3, 1, 1> wyy, agg;
        Components<3, 1, 2> wyz, agb;
        Components<3, 1, 3> wyw, aga;

        Components<3, 2, 0> wzx, abr;
        Components<3, 2, 1> wzy, abg;
        Components<3, 2, 2> wzz, abb;
        Components<3, 2, 3> wzw, aba;
        Components<3, 3, 0> wwx, aar;
        Components<3, 3, 1> wwy, aag;
        Components<3, 3, 2> wwz, aab;
        Components<3, 3, 3> www, aaa;

        Components<0, 0, 0, 0> xxxx, rrrr;
        Components<0, 0, 0, 1> xxxy, rrrg;
        Components<0, 0, 0, 2> xxxz, rrrb;
        Components<0, 0, 0, 3> xxxw, rrra;
        Components<0, 0, 1, 0> xxyx, rrgr;
        Components<0, 0, 1, 1> xxyy, rrgg;
        Components<0, 0, 1, 2> xxyz, rrgb;
        Components<0, 0, 1, 3> xxyw, rrga;

        Components<0, 0, 2, 0> xxzx, rrbr;
        Components<0, 0, 2, 1> xxzy, rrbg;
        Components<0, 0, 2, 2> xxzz, rrbb;
        Components<0, 0, 2, 3> xxzw, rrba;
        Components<0, 0, 3, 0> xxwx, rrar;
        Components<0, 0, 3, 1> xxwy, rrag;
        Components<0, 0, 3, 2> xxwz, rrab;
        Components<0, 0, 3, 3> xxww, rraa;

        Components<0, 1, 0, 0> xyxx, rgrr;
        Components<0, 1, 0, 1> xyxy, rgrg;
        Components<0, 1, 0, 2> xyxz, rgrb;
        Components<0, 1, 0, 3> xyxw, rgra;
        Components<0, 1, 1, 0> xyyx, rggr;
        Components<0, 1, 1, 1> xyyy, rggg;
        Components<0, 1, 1, 2> xyyz, rggb;
        Components<0, 1, 1, 3> xyyw, rgga;

        Components<0, 1, 2, 0> xyzx, rgbr;
        Components<0, 1, 2, 1> xyzy, rgbg;
        Components<0, 1, 2, 2> xyzz, rgbb;
        Components<0, 1, 2, 3> xyzw, rgba;
        Components<0, 1, 3, 0> xywx, rgar;
        Components<0, 1, 3, 1> xywy, rgag;
        Components<0, 1, 3, 2> xywz, rgab;
        Components<0, 1, 3, 3> xyww, rgaa;

        Components<0, 2, 0, 0> xzxx, rbrr;
        Components<0, 2, 0, 1> xzxy, rbrg;
        Components<0, 2, 0, 2> xzxz, rbrb;
        Components<0, 2, 0, 3> xzxw, rbra;
        Components<0, 2, 1, 0> xzyx, rbgr;
        Components<0, 2, 1, 1> xzyy, rbgg;
        Components<0, 2, 1, 2> xzyz, rbgb;
        Components<0, 2, 1, 3> xzyw, rbga;

        Components<0, 2, 2, 0> xzzx, rbbr;
        Components<0, 2, 2, 1> xzzy, rbbg;
        Components<0, 2, 2, 2> xzzz, rbbb;
        Components<0, 2, 2, 3> xzzw, rbba;
        Components<0, 2, 3, 0> xzwx, rbar;
        Components<0, 2, 3, 1> xzwy, rbag;
        Components<0, 2, 3, 2> xzwz, rbab;
        Components<0, 2, 3, 3> xzww, rbaa;

        Components<0, 3, 0, 0> xwxx, rarr;
        Components<0, 3, 0, 1> xwxy, rarg;
        Components<0, 3, 0, 2> xwxz, rarb;
        Components<0, 3, 0, 3> xwxw, rara;
        Components<0, 3, 1, 0> xwyx, ragr;
        Components<0, 3, 1, 1> xwyy, ragg;
        Components<0, 3, 1, 2> xwyz, ragb;
        Components<0, 3, 1, 3> xwyw, raga;

        Components<0, 3, 2, 0> xwzx, rabr;
        Components<0, 3, 2, 1> xwzy, rabg;
        Components<0, 3, 2, 2> xwzz, rabb;
        Components<0, 3, 2, 3> xwzw, raba;
        Components<0, 3, 3, 0> xwwx, raar;
        Components<0, 3, 3, 1> xwwy, raag;
        Components<0, 3, 3, 2> xwwz, raab;
        Components<0, 3, 3, 3> xwww, raaa;

        Components<1, 0, 0, 0> yxxx, grrr;
        Components<1, 0, 0, 1> yxxy, grrg;
        Components<1, 0, 0, 2> yxxz, grrb;
        Components<1, 0, 0, 3> yxxw, grra;
        Components<1, 0, 1, 0> yxyx, grgr;
        Components<1, 0, 1, 1> yxyy, grgg;
        Components<1, 0, 1, 2> yxyz, grgb;
        Components<1, 0, 1, 3> yxyw, grga;

        Components<1, 0, 2, 0> yxzx, grbr;
        Components<1, 0, 2, 1> yxzy, grbg;
        Components<1, 0, 2, 2> yxzz, grbb;
        Components<1, 0, 2, 3> yxzw, grba;
        Components<1, 0, 3, 0> yxwx, grar;
        Components<1, 0, 3, 1> yxwy, grag;
        Components<1, 0, 3, 2> yxwz, grab;
        Components<1, 0, 3, 3> yxww, graa;

        Components<1, 1, 0, 0> yyxx, ggrr;
        Components<1, 1, 0, 1> yyxy, ggrg;
        Components<1, 1, 0, 2> yyxz, ggrb;
        Components<1, 1, 0, 3> yyxw, ggra;
        Components<1, 1, 1, 0> yyyx, gggr;
        Components<1, 1, 1, 1> yyyy, gggg;
        Components<1, 1, 1, 2> yyyz, gggb;
        Components<1, 1, 1, 3> yyyw, ggga;

        Components<1, 1, 2, 0> yyzx, ggbr;
        Components<1, 1, 2, 1> yyzy, ggbg;
        Components<1, 1, 2, 2> yyzz, ggbb;
        Components<1, 1, 2, 3> yyzw, ggba;
        Components<1, 1, 3, 0> yywx, ggar;
        Components<1, 1, 3, 1> yywy, ggag;
        Components<1, 1, 3, 2> yywz, ggab;
        Components<1, 1, 3, 3> yyww, ggaa;

        Components<1, 2, 0, 0> yzxx, gbrr;
        Components<1, 2, 0, 1> yzxy, gbrg;
        Components<1, 2, 0, 2> yzxz, gbrb;
        Components<1, 2, 0, 3> yzxw, gbra;
        Components<1, 2, 1, 0> yzyx, gbgr;
        Components<1, 2, 1, 1> yzyy, gbgg;
        Components<1, 2, 1, 2> yzyz, gbgb;
        Components<1, 2, 1, 3> yzyw, gbga;

        Components<1, 2, 2, 0> yzzx, gbbr;
        Components<1, 2, 2, 1> yzzy, gbbg;
        Components<1, 2, 2, 2> yzzz, gbbb;
        Components<1, 2, 2, 3> yzzw, gbba;
        Components<1, 2, 3, 0> yzwx, gbar;
        Components<1, 2, 3, 1> yzwy, gbag;
        Components<1, 2, 3, 2> yzwz, gbab;
        Components<1, 2, 3, 3> yzww, gbaa;

        Components<1, 3, 0, 0> ywxx, garr;
        Components<1, 3, 0, 1> ywxy, garg;
        Components<1, 3, 0, 2> ywxz, garb;
        Components<1, 3, 0, 3> ywxw, gara;
        Components<1, 3, 1, 0> ywyx, gagr;
        Components<1, 3, 1, 1> ywyy, gagg;
        Components<1, 3, 1, 2> ywyz, gagb;
        Components<1, 3, 1, 3> ywyw, gaga;

        Components<1, 3, 2, 0> ywzx, gabr;
        Components<1, 3, 2, 1> ywzy, gabg;
        Components<1, 3, 2, 2> ywzz, gabb;
        Components<1, 3, 2, 3> ywzw, gaba;
        Components<1, 3, 3, 0> ywwx, gaar;
        Components<1, 3, 3, 1> ywwy, gaag;
        Components<1, 3, 3, 2> ywwz, gaab;
        Components<1, 3, 3, 3> ywww, gaaa;

        Components<2, 0, 0, 0> zxxx, brrr;
        Components<2, 0, 0, 1> zxxy, brrg;
        Components<2, 0, 0, 2> zxxz, brrb;
        Components<2, 0, 0, 3> zxxw, brra;
        Components<2, 0, 1, 0> zxyx, brgr;
        Components<2, 0, 1, 1> zxyy, brgg;
        Components<2, 0, 1, 2> zxyz, brgb;
        Components<2, 0, 1, 3> zxyw, brga;

        Components<2, 0, 2, 0> zxzx, brbr;
        Components<2, 0, 2, 1> zxzy, brbg;
        Components<2, 0, 2, 2> zxzz, brbb;
        Components<2, 0, 2, 3> zxzw, brba;
        Components<2, 0, 3, 0> zxwx, brar;
        Components<2, 0, 3, 1> zxwy, brag;
        Components<2, 0, 3, 2> zxwz, brab;
        Components<2, 0, 3, 3> zxww, braa;

        Components<2, 1, 0, 0> zyxx, bgrr;
        Components<2, 1, 0, 1> zyxy, bgrg;
        Components<2, 1, 0, 2> zyxz, bgrb;
        Components<2, 1, 0, 3> zyxw, bgra;
        Components<2, 1, 1, 0> zyyx, bggr;
        Components<2, 1, 1, 1> zyyy, bggg;
        Components<2, 1, 1, 2> zyyz, bggb;
        Components<2, 1, 1, 3> zyyw, bgga;

        Components<2, 1, 2, 0> zyzx, bgbr;
        Components<2, 1, 2, 1> zyzy, bgbg;
        Components<2, 1, 2, 2> zyzz, bgbb;
        Components<2, 1, 2, 3> zyzw, bgba;
        Components<2, 1, 3, 0> zywx, bgar;
        Components<2, 1, 3, 1> zywy, bgag;
        Components<2, 1, 3, 2> zywz, bgab;
        Components<2, 1, 3, 3> zyww, bgaa;

        Components<2, 2, 0, 0> zzxx, bbrr;
        Components<2, 2, 0, 1> zzxy, bbrg;
        Components<2, 2, 0, 2> zzxz, bbrb;
        Components<2, 2, 0, 3> zzxw, bbra;
        Components<2, 2, 1, 0> zzyx, bbgr;
        Components<2, 2, 1, 1> zzyy, bbgg;
        Components<2, 2, 1, 2> zzyz, bbgb;
        Components<2, 2, 1, 3> zzyw, bbga;

        Components<2, 2, 2, 0> zzzx, bbbr;
        Components<2, 2, 2, 1> zzzy, bbbg;
        Components<2, 2, 2, 2> zzzz, bbbb;
        Components<2, 2, 2, 3> zzzw, bbba;
        Components<2, 2, 3, 0> zzwx, bbar;
        Components<2, 2, 3, 1> zzwy, bbag;
        Components<2, 2, 3, 2> zzwz, bbab;
        Components<2, 2, 3, 3> zzww, bbaa;

        Components<2, 3, 0, 0> zwxx, barr;
        Components<2, 3, 0, 1> zwxy, barg;
        Components<2, 3, 0, 2> zwxz, barb;
        Components<2, 3, 0, 3> zwxw, bara;
        Components<2, 3, 1, 0> zwyx, bagr;
        Components<2, 3, 1, 1> zwyy, bagg;
        Components<2, 3, 1, 2> zwyz, bagb;
        Components<2, 3, 1, 3> zwyw, baga;

        Components<2, 3, 2, 0> zwzx, babr;
        Components<2, 3, 2, 1> zwzy, babg;
        Components<2, 3, 2, 2> zwzz, babb;
        Components<2, 3, 2, 3> zwzw, baba;
        Components<2, 3, 3, 0> zwwx, baar;
        Components<2, 3, 3, 1> zwwy, baag;
        Components<2, 3, 3, 2> zwwz, baab;
        Components<2, 3, 3, 3> zwww, baaa;

        Components<3, 0, 0, 0> wxxx, arrr;
        Components<3, 0, 0, 1> wxxy, arrg;
        Components<3, 0, 0, 2> wxxz, arrb;
        Components<3, 0, 0, 3> wxxw, arra;
        Components<3, 0, 1, 0> wxyx, argr;
        Components<3, 0, 1, 1> wxyy, argg;
        Components<3, 0, 1, 2> wxyz, argb;
        Components<3, 0, 1, 3> wxyw, arga;

        Components<3, 0, 2, 0> wxzx, arbr;
        Components<3, 0, 2, 1> wxzy, arbg;
        Components<3, 0, 2, 2> wxzz, arbb;
        Components<3, 0, 2, 3> wxzw, arba;
        Components<3, 0, 3, 0> wxwx, arar;
        Components<3, 0, 3, 1> wxwy, arag;
        Components<3, 0, 3, 2> wxwz, arab;
        Components<3, 0, 3, 3> wxww, araa;

        Components<3, 1, 0, 0> wyxx, agrr;
        Components<3, 1, 0, 1> wyxy, agrg;
        Components<3, 1, 0, 2> wyxz, agrb;
        Components<3, 1, 0, 3> wyxw, agra;
        Components<3, 1, 1, 0> wyyx, aggr;
        Components<3, 1, 1, 1> wyyy, aggg;
        Components<3, 1, 1, 2> wyyz, aggb;
        Components<3, 1, 1, 3> wyyw, agga;

        Components<3, 1, 2, 0> wyzx, agbr;
        Components<3, 1, 2, 1> wyzy, agbg;
        Components<3, 1, 2, 2> wyzz, agbb;
        Components<3, 1, 2, 3> wyzw, agba;
        Components<3, 1, 3, 0> wywx, agar;
        Components<3, 1, 3, 1> wywy, agag;
        Components<3, 1, 3, 2> wywz, agab;
        Components<3, 1, 3, 3> wyww, agaa;

        Components<3, 2, 0, 0> wzxx, abrr;
        Components<3, 2, 0, 1> wzxy, abrg;
        Components<3, 2, 0, 2> wzxz, abrb;
        Components<3, 2, 0, 3> wzxw, abra;
        Components<3, 2, 1, 0> wzyx, abgr;
        Components<3, 2, 1, 1> wzyy, abgg;
        Components<3, 2, 1, 2> wzyz, abgb;
        Components<3, 2, 1, 3> wzyw, abga;

        Components<3, 2, 2, 0> wzzx, abbr;
        Components<3, 2, 2, 1> wzzy, abbg;
        Components<3, 2, 2, 2> wzzz, abbb;
        Components<3, 2, 2, 3> wzzw, abba;
        Components<3, 2, 3, 0> wzwx, abar;
        Components<3, 2, 3, 1> wzwy, abag;
        Components<3, 2, 3, 2> wzwz, abab;
        Components<3, 2, 3, 3> wzww, abaa;

        Components<3, 3, 0, 0> wwxx, aarr;
        Components<3, 3, 0, 1> wwxy, aarg;
        Components<3, 3, 0, 2> wwxz, aarb;
        Components<3, 3, 0, 3> wwxw, aara;
        Components<3, 3, 1, 0> wwyx, aagr;
        Components<3, 3, 1, 1> wwyy, aagg;
        Components<3, 3, 1, 2> wwyz, aagb;
        Components<3, 3, 1, 3> wwyw, aaga;

        Components<3, 3, 2, 0> wwzx, aabr;
        Components<3, 3, 2, 1> wwzy, aabg;
        Components<3, 3, 2, 2> wwzz, aabb;
        Components<3, 3, 2, 3> wwzw, aaba;
        Components<3, 3, 3, 0> wwwx, aaar;
        Components<3, 3, 3, 1> wwwy, aaag;
        Components<3, 3, 3, 2> wwwz, aaab;
        Components<3, 3, 3, 3> wwww, aaaa;
        /// @endcond
        /// @}
    };
};

} // namespace asciirast::math
