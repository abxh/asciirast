/**
 * @file VecBase.h
 * @brief Vector base class with support for swizzled components for 1,2,3 and 4
 * dimensional vectors.
 */

#pragma once

#include <array>

#include "Swizzled.h"

namespace asciirast::math {

/**
 * @brief Vector base class.
 */
template <template <int, typename> typename VecClass, int N, typename T>
    requires(N > 0)
class VecBase {
public:
    std::array<T, N> m_components;  ///< vector component array
};

/**
 * @brief One-dimensional specialization of vector base class with support for
 * {x} component.
 */
template <template <int, typename> typename VecClass, typename T>
class VecBase<VecClass, 1, T> {
private:
    static constexpr int N = 1;

    template <int... Indicies>
    using Component = Swizzled<VecClass, N, T, Indicies...>;

public:
    /**
     * @brief Anomymous union between an array of components and swizzled
     * components.
     */
    union {
        std::array<T, N> m_components;  ///< vector component array

        /**
         * @name Swizzled components
         * @{
         * Combinations of {x} with a maximum size of 4 as class members.
         */
        Component<0> x;
        Component<0, 0> xx;
        Component<0, 0, 0> xxx;
        Component<0, 0, 0, 0> xxxx;
        /// @}
    };
};

/**
 * @brief Two-dimensional specialization of vector base class with support for
 * swizzled {x, y} components.
 */
template <template <int, typename> typename VecClass, typename T>
class VecBase<VecClass, 2, T> {
private:
    static constexpr int N = 2;

    template <int... Indicies>
    using Component = Swizzled<VecClass, N, T, Indicies...>;

public:
    /**
     * @brief Anonymous union between an array of components and swizzled
     * components.
     */
    union {
        std::array<T, N> m_components;  ///< Vector component array

        /**
         * @name Swizzled components
         * @{
         * Combinations of {x, y} with a maximum size of 4 as class members.
         */
        Component<0> x;
        Component<1> y;

        // the rest are not documented to spare doxygen of generating redundant
        // documentation
        /// @cond DO_NO_DOCUMENT
        Component<0, 0> xx;
        Component<0, 1> xy;
        Component<1, 0> yx;
        Component<1, 1> yy;

        Component<0, 0, 0> xxx;
        Component<0, 0, 1> xxy;
        Component<0, 1, 0> xyx;
        Component<0, 1, 1> xyy;
        Component<1, 0, 0> yxx;
        Component<1, 0, 1> yxy;
        Component<1, 1, 0> yyx;
        Component<1, 1, 1> yyy;

        Component<0, 0, 0, 0> xxxx;
        Component<0, 0, 0, 1> xxxy;
        Component<0, 0, 1, 0> xxyx;
        Component<0, 0, 1, 1> xxyy;
        Component<0, 1, 0, 0> xyxx;
        Component<0, 1, 0, 1> xyxy;
        Component<0, 1, 1, 0> xyyx;
        Component<0, 1, 1, 1> xyyy;

        // copy of above with slight change:
        Component<1, 0, 0, 0> yxxx;
        Component<1, 0, 0, 1> yxxy;
        Component<1, 0, 1, 0> yxyx;
        Component<1, 0, 1, 1> yxyy;
        Component<1, 1, 0, 0> yyxx;
        Component<1, 1, 0, 1> yyxy;
        Component<1, 1, 1, 0> yyyx;
        Component<1, 1, 1, 1> yyyy;
        /// @endcond
        /// @}
    };
};

/**
 * @brief Three-dimensional specialization of  vector base class with support
 * for swizzled {x, y, z} components.
 */
template <template <int, typename> typename VecClass, typename T>
class VecBase<VecClass, 3, T> {
private:
    static constexpr int N = 3;

    template <int... Indicies>
    using Component = Swizzled<VecClass, N, T, Indicies...>;

public:
    /**
     * @brief Anonymous union between an array of components and swizzled
     * components
     */
    union {
        std::array<T, N> m_components;  ///< vector component array

        /**
         * @name Swizzled components
         * @{
         * Combinations of {x, y, z} with a maximum size of 4 as class members.
         */
        Component<0> x;
        Component<1> y;
        Component<2> z;

        // the rest are not documented to spare doxygen of generating redundant
        // documentation
        /// @cond DO_NO_DOCUMENT
        Component<0, 0> xx;
        Component<0, 1> xy;
        Component<0, 2> xz;
        Component<1, 0> yx;
        Component<1, 1> yy;
        Component<1, 2> yz;
        Component<2, 0> zx;
        Component<2, 1> zy;
        Component<2, 2> zz;

        Component<0, 0, 0> xxx;
        Component<0, 0, 1> xxy;
        Component<0, 0, 2> xxz;
        Component<0, 1, 0> xyx;
        Component<0, 1, 1> xyy;
        Component<0, 1, 2> xyz;
        Component<0, 2, 0> xzx;
        Component<0, 2, 1> xzy;
        Component<0, 2, 2> xzz;
        Component<1, 0, 0> yxx;
        Component<1, 0, 1> yxy;
        Component<1, 0, 2> yxz;
        Component<1, 1, 0> yyx;
        Component<1, 1, 1> yyy;
        Component<1, 1, 2> yyz;
        Component<1, 2, 0> yzx;
        Component<1, 2, 1> yzy;
        Component<1, 2, 2> yzz;
        Component<2, 0, 0> zxx;
        Component<2, 0, 1> zxy;
        Component<2, 0, 2> zxz;
        Component<2, 1, 0> zyx;
        Component<2, 1, 1> zyy;
        Component<2, 1, 2> zyz;
        Component<2, 2, 0> zzx;
        Component<2, 2, 1> zzy;
        Component<2, 2, 2> zzz;

        // copy of above with slight change:
        Component<0, 0, 0, 0> xxxx;
        Component<0, 0, 0, 1> xxxy;
        Component<0, 0, 0, 2> xxxz;
        Component<0, 0, 1, 0> xxyx;
        Component<0, 0, 1, 1> xxyy;
        Component<0, 0, 1, 2> xxyz;
        Component<0, 0, 2, 0> xxzx;
        Component<0, 0, 2, 1> xxzy;
        Component<0, 0, 2, 2> xxzz;
        Component<0, 1, 0, 0> xyxx;
        Component<0, 1, 0, 1> xyxy;
        Component<0, 1, 0, 2> xyxz;
        Component<0, 1, 1, 0> xyyx;
        Component<0, 1, 1, 1> xyyy;
        Component<0, 1, 1, 2> xyyz;
        Component<0, 1, 2, 0> xyzx;
        Component<0, 1, 2, 1> xyzy;
        Component<0, 1, 2, 2> xyzz;
        Component<0, 2, 0, 0> xzxx;
        Component<0, 2, 0, 1> xzxy;
        Component<0, 2, 0, 2> xzxz;
        Component<0, 2, 1, 0> xzyx;
        Component<0, 2, 1, 1> xzyy;
        Component<0, 2, 1, 2> xzyz;
        Component<0, 2, 2, 0> xzzx;
        Component<0, 2, 2, 1> xzzy;
        Component<0, 2, 2, 2> xzzz;

        // copy of above with slight change:
        Component<1, 0, 0, 0> yxxx;
        Component<1, 0, 0, 1> yxxy;
        Component<1, 0, 0, 2> yxxz;
        Component<1, 0, 1, 0> yxyx;
        Component<1, 0, 1, 1> yxyy;
        Component<1, 0, 1, 2> yxyz;
        Component<1, 0, 2, 0> yxzx;
        Component<1, 0, 2, 1> yxzy;
        Component<1, 0, 2, 2> yxzz;
        Component<1, 1, 0, 0> yyxx;
        Component<1, 1, 0, 1> yyxy;
        Component<1, 1, 0, 2> yyxz;
        Component<1, 1, 1, 0> yyyx;
        Component<1, 1, 1, 1> yyyy;
        Component<1, 1, 1, 2> yyyz;
        Component<1, 1, 2, 0> yyzx;
        Component<1, 1, 2, 1> yyzy;
        Component<1, 1, 2, 2> yyzz;
        Component<1, 2, 0, 0> yzxx;
        Component<1, 2, 0, 1> yzxy;
        Component<1, 2, 0, 2> yzxz;
        Component<1, 2, 1, 0> yzyx;
        Component<1, 2, 1, 1> yzyy;
        Component<1, 2, 1, 2> yzyz;
        Component<1, 2, 2, 0> yzzx;
        Component<1, 2, 2, 1> yzzy;
        Component<1, 2, 2, 2> yzzz;

        // copy of above with slight change:
        Component<2, 0, 0, 0> zxxx;
        Component<2, 0, 0, 1> zxxy;
        Component<2, 0, 0, 2> zxxz;
        Component<2, 0, 1, 0> zxyx;
        Component<2, 0, 1, 1> zxyy;
        Component<2, 0, 1, 2> zxyz;
        Component<2, 0, 2, 0> zxzx;
        Component<2, 0, 2, 1> zxzy;
        Component<2, 0, 2, 2> zxzz;
        Component<2, 1, 0, 0> zyxx;
        Component<2, 1, 0, 1> zyxy;
        Component<2, 1, 0, 2> zyxz;
        Component<2, 1, 1, 0> zyyx;
        Component<2, 1, 1, 1> zyyy;
        Component<2, 1, 1, 2> zyyz;
        Component<2, 1, 2, 0> zyzx;
        Component<2, 1, 2, 1> zyzy;
        Component<2, 1, 2, 2> zyzz;
        Component<2, 2, 0, 0> zzxx;
        Component<2, 2, 0, 1> zzxy;
        Component<2, 2, 0, 2> zzxz;
        Component<2, 2, 1, 0> zzyx;
        Component<2, 2, 1, 1> zzyy;
        Component<2, 2, 1, 2> zzyz;
        Component<2, 2, 2, 0> zzzx;
        Component<2, 2, 2, 1> zzzy;
        Component<2, 2, 2, 2> zzzz;
        /// @endcond
        ///@}
    };
};

/**
 * @brief Four-dimensional vector base class with support for swizzled {x, y, z,
 * w} components.
 */
template <template <int, typename> typename VecClass, typename T>
class VecBase<VecClass, 4, T> {
private:
    static constexpr int N = 4;

    template <int... Indicies>
    using Component = Swizzled<VecClass, N, T, Indicies...>;

public:
    /**
     * @brief Anonymous union between an array of components and swizzled
     * components.
     */
    union {
        std::array<T, N> m_components;  ///< vector component array

        /**
         * @name Swizzled components
         * @{
         * Combinations of {x, y, z, w} with a maximum size of 4 as class
         * members.
         */
        Component<0> x;
        Component<1> y;
        Component<2> z;
        Component<3> w;

        // the rest are not documented to spare doxygen of generating redundant
        // documentation
        /// @cond DO_NO_DOCUMENT
        Component<0, 0> xx;
        Component<0, 1> xy;
        Component<0, 2> xz;
        Component<0, 3> xw;
        Component<1, 0> yx;
        Component<1, 1> yy;
        Component<1, 2> yz;
        Component<1, 3> yw;
        Component<2, 0> zx;
        Component<2, 1> zy;
        Component<2, 2> zz;
        Component<2, 3> zw;
        Component<3, 0> wx;
        Component<3, 1> wy;
        Component<3, 2> wz;
        Component<3, 3> ww;

        Component<0, 0, 0> xxx;
        Component<0, 0, 1> xxy;
        Component<0, 0, 2> xxz;
        Component<0, 0, 3> xxw;
        Component<0, 1, 0> xyx;
        Component<0, 1, 1> xyy;
        Component<0, 1, 2> xyz;
        Component<0, 1, 3> xyw;
        Component<0, 2, 0> xzx;
        Component<0, 2, 1> xzy;
        Component<0, 2, 2> xzz;
        Component<0, 2, 3> xzw;
        Component<0, 3, 0> xwx;
        Component<0, 3, 1> xwy;
        Component<0, 3, 2> xwz;
        Component<0, 3, 3> xww;
        Component<1, 0, 0> yxx;
        Component<1, 0, 1> yxy;
        Component<1, 0, 2> yxz;
        Component<1, 0, 3> yxw;
        Component<1, 1, 0> yyx;
        Component<1, 1, 1> yyy;
        Component<1, 1, 2> yyz;
        Component<1, 1, 3> yyw;
        Component<1, 2, 0> yzx;
        Component<1, 2, 1> yzy;
        Component<1, 2, 2> yzz;
        Component<1, 2, 3> yzw;
        Component<1, 3, 0> ywx;
        Component<1, 3, 1> ywy;
        Component<1, 3, 2> ywz;
        Component<1, 3, 3> yww;
        Component<2, 0, 0> zxx;
        Component<2, 0, 1> zxy;
        Component<2, 0, 2> zxz;
        Component<2, 0, 3> zxw;
        Component<2, 1, 0> zyx;
        Component<2, 1, 1> zyy;
        Component<2, 1, 2> zyz;
        Component<2, 1, 3> zyw;
        Component<2, 2, 0> zzx;
        Component<2, 2, 1> zzy;
        Component<2, 2, 2> zzz;
        Component<2, 2, 3> zzw;
        Component<2, 3, 0> zwx;
        Component<2, 3, 1> zwy;
        Component<2, 3, 2> zwz;
        Component<2, 3, 3> zww;
        Component<3, 0, 0> wxx;
        Component<3, 0, 1> wxy;
        Component<3, 0, 2> wxz;
        Component<3, 0, 3> wxw;
        Component<3, 1, 0> wyx;
        Component<3, 1, 1> wyy;
        Component<3, 1, 2> wyz;
        Component<3, 1, 3> wyw;
        Component<3, 2, 0> wzx;
        Component<3, 2, 1> wzy;
        Component<3, 2, 2> wzz;
        Component<3, 2, 3> wzw;
        Component<3, 3, 0> wwx;
        Component<3, 3, 1> wwy;
        Component<3, 3, 2> wwz;
        Component<3, 3, 3> www;

        Component<0, 0, 0, 0> xxxx;
        Component<0, 0, 0, 1> xxxy;
        Component<0, 0, 0, 2> xxxz;
        Component<0, 0, 0, 3> xxxw;
        Component<0, 0, 1, 0> xxyx;
        Component<0, 0, 1, 1> xxyy;
        Component<0, 0, 1, 2> xxyz;
        Component<0, 0, 1, 3> xxyw;
        Component<0, 0, 2, 0> xxzx;
        Component<0, 0, 2, 1> xxzy;
        Component<0, 0, 2, 2> xxzz;
        Component<0, 0, 2, 3> xxzw;
        Component<0, 0, 3, 0> xxwx;
        Component<0, 0, 3, 1> xxwy;
        Component<0, 0, 3, 2> xxwz;
        Component<0, 0, 3, 3> xxww;
        Component<0, 1, 0, 0> xyxx;
        Component<0, 1, 0, 1> xyxy;
        Component<0, 1, 0, 2> xyxz;
        Component<0, 1, 0, 3> xyxw;
        Component<0, 1, 1, 0> xyyx;
        Component<0, 1, 1, 1> xyyy;
        Component<0, 1, 1, 2> xyyz;
        Component<0, 1, 1, 3> xyyw;
        Component<0, 1, 2, 0> xyzx;
        Component<0, 1, 2, 1> xyzy;
        Component<0, 1, 2, 2> xyzz;
        Component<0, 1, 2, 3> xyzw;
        Component<0, 1, 3, 0> xywx;
        Component<0, 1, 3, 1> xywy;
        Component<0, 1, 3, 2> xywz;
        Component<0, 1, 3, 3> xyww;
        Component<0, 2, 0, 0> xzxx;
        Component<0, 2, 0, 1> xzxy;
        Component<0, 2, 0, 2> xzxz;
        Component<0, 2, 0, 3> xzxw;
        Component<0, 2, 1, 0> xzyx;
        Component<0, 2, 1, 1> xzyy;
        Component<0, 2, 1, 2> xzyz;
        Component<0, 2, 1, 3> xzyw;
        Component<0, 2, 2, 0> xzzx;
        Component<0, 2, 2, 1> xzzy;
        Component<0, 2, 2, 2> xzzz;
        Component<0, 2, 2, 3> xzzw;
        Component<0, 2, 3, 0> xzwx;
        Component<0, 2, 3, 1> xzwy;
        Component<0, 2, 3, 2> xzwz;
        Component<0, 2, 3, 3> xzww;
        Component<0, 3, 0, 0> xwxx;
        Component<0, 3, 0, 1> xwxy;
        Component<0, 3, 0, 2> xwxz;
        Component<0, 3, 0, 3> xwxw;
        Component<0, 3, 1, 0> xwyx;
        Component<0, 3, 1, 1> xwyy;
        Component<0, 3, 1, 2> xwyz;
        Component<0, 3, 1, 3> xwyw;
        Component<0, 3, 2, 0> xwzx;
        Component<0, 3, 2, 1> xwzy;
        Component<0, 3, 2, 2> xwzz;
        Component<0, 3, 2, 3> xwzw;
        Component<0, 3, 3, 0> xwwx;
        Component<0, 3, 3, 1> xwwy;
        Component<0, 3, 3, 2> xwwz;
        Component<0, 3, 3, 3> xwww;

        // copy of above with slight change:
        Component<1, 0, 0, 0> yxxx;
        Component<1, 0, 0, 1> yxxy;
        Component<1, 0, 0, 2> yxxz;
        Component<1, 0, 0, 3> yxxw;
        Component<1, 0, 1, 0> yxyx;
        Component<1, 0, 1, 1> yxyy;
        Component<1, 0, 1, 2> yxyz;
        Component<1, 0, 1, 3> yxyw;
        Component<1, 0, 2, 0> yxzx;
        Component<1, 0, 2, 1> yxzy;
        Component<1, 0, 2, 2> yxzz;
        Component<1, 0, 2, 3> yxzw;
        Component<1, 0, 3, 0> yxwx;
        Component<1, 0, 3, 1> yxwy;
        Component<1, 0, 3, 2> yxwz;
        Component<1, 0, 3, 3> yxww;
        Component<1, 1, 0, 0> yyxx;
        Component<1, 1, 0, 1> yyxy;
        Component<1, 1, 0, 2> yyxz;
        Component<1, 1, 0, 3> yyxw;
        Component<1, 1, 1, 0> yyyx;
        Component<1, 1, 1, 1> yyyy;
        Component<1, 1, 1, 2> yyyz;
        Component<1, 1, 1, 3> yyyw;
        Component<1, 1, 2, 0> yyzx;
        Component<1, 1, 2, 1> yyzy;
        Component<1, 1, 2, 2> yyzz;
        Component<1, 1, 2, 3> yyzw;
        Component<1, 1, 3, 0> yywx;
        Component<1, 1, 3, 1> yywy;
        Component<1, 1, 3, 2> yywz;
        Component<1, 1, 3, 3> yyww;
        Component<1, 2, 0, 0> yzxx;
        Component<1, 2, 0, 1> yzxy;
        Component<1, 2, 0, 2> yzxz;
        Component<1, 2, 0, 3> yzxw;
        Component<1, 2, 1, 0> yzyx;
        Component<1, 2, 1, 1> yzyy;
        Component<1, 2, 1, 2> yzyz;
        Component<1, 2, 1, 3> yzyw;
        Component<1, 2, 2, 0> yzzx;
        Component<1, 2, 2, 1> yzzy;
        Component<1, 2, 2, 2> yzzz;
        Component<1, 2, 2, 3> yzzw;
        Component<1, 2, 3, 0> yzwx;
        Component<1, 2, 3, 1> yzwy;
        Component<1, 2, 3, 2> yzwz;
        Component<1, 2, 3, 3> yzww;
        Component<1, 3, 0, 0> ywxx;
        Component<1, 3, 0, 1> ywxy;
        Component<1, 3, 0, 2> ywxz;
        Component<1, 3, 0, 3> ywxw;
        Component<1, 3, 1, 0> ywyx;
        Component<1, 3, 1, 1> ywyy;
        Component<1, 3, 1, 2> ywyz;
        Component<1, 3, 1, 3> ywyw;
        Component<1, 3, 2, 0> ywzx;
        Component<1, 3, 2, 1> ywzy;
        Component<1, 3, 2, 2> ywzz;
        Component<1, 3, 2, 3> ywzw;
        Component<1, 3, 3, 0> ywwx;
        Component<1, 3, 3, 1> ywwy;
        Component<1, 3, 3, 2> ywwz;
        Component<1, 3, 3, 3> ywww;

        // copy of above with slight change:
        Component<2, 0, 0, 0> zxxx;
        Component<2, 0, 0, 1> zxxy;
        Component<2, 0, 0, 2> zxxz;
        Component<2, 0, 0, 3> zxxw;
        Component<2, 0, 1, 0> zxyx;
        Component<2, 0, 1, 1> zxyy;
        Component<2, 0, 1, 2> zxyz;
        Component<2, 0, 1, 3> zxyw;
        Component<2, 0, 2, 0> zxzx;
        Component<2, 0, 2, 1> zxzy;
        Component<2, 0, 2, 2> zxzz;
        Component<2, 0, 2, 3> zxzw;
        Component<2, 0, 3, 0> zxwx;
        Component<2, 0, 3, 1> zxwy;
        Component<2, 0, 3, 2> zxwz;
        Component<2, 0, 3, 3> zxww;
        Component<2, 1, 0, 0> zyxx;
        Component<2, 1, 0, 1> zyxy;
        Component<2, 1, 0, 2> zyxz;
        Component<2, 1, 0, 3> zyxw;
        Component<2, 1, 1, 0> zyyx;
        Component<2, 1, 1, 1> zyyy;
        Component<2, 1, 1, 2> zyyz;
        Component<2, 1, 1, 3> zyyw;
        Component<2, 1, 2, 0> zyzx;
        Component<2, 1, 2, 1> zyzy;
        Component<2, 1, 2, 2> zyzz;
        Component<2, 1, 2, 3> zyzw;
        Component<2, 1, 3, 0> zywx;
        Component<2, 1, 3, 1> zywy;
        Component<2, 1, 3, 2> zywz;
        Component<2, 1, 3, 3> zyww;
        Component<2, 2, 0, 0> zzxx;
        Component<2, 2, 0, 1> zzxy;
        Component<2, 2, 0, 2> zzxz;
        Component<2, 2, 0, 3> zzxw;
        Component<2, 2, 1, 0> zzyx;
        Component<2, 2, 1, 1> zzyy;
        Component<2, 2, 1, 2> zzyz;
        Component<2, 2, 1, 3> zzyw;
        Component<2, 2, 2, 0> zzzx;
        Component<2, 2, 2, 1> zzzy;
        Component<2, 2, 2, 2> zzzz;
        Component<2, 2, 2, 3> zzzw;
        Component<2, 2, 3, 0> zzwx;
        Component<2, 2, 3, 1> zzwy;
        Component<2, 2, 3, 2> zzwz;
        Component<2, 2, 3, 3> zzww;
        Component<2, 3, 0, 0> zwxx;
        Component<2, 3, 0, 1> zwxy;
        Component<2, 3, 0, 2> zwxz;
        Component<2, 3, 0, 3> zwxw;
        Component<2, 3, 1, 0> zwyx;
        Component<2, 3, 1, 1> zwyy;
        Component<2, 3, 1, 2> zwyz;
        Component<2, 3, 1, 3> zwyw;
        Component<2, 3, 2, 0> zwzx;
        Component<2, 3, 2, 1> zwzy;
        Component<2, 3, 2, 2> zwzz;
        Component<2, 3, 2, 3> zwzw;
        Component<2, 3, 3, 0> zwwx;
        Component<2, 3, 3, 1> zwwy;
        Component<2, 3, 3, 2> zwwz;
        Component<2, 3, 3, 3> zwww;

        // copy of above with slight change:
        Component<3, 0, 0, 0> wxxx;
        Component<3, 0, 0, 1> wxxy;
        Component<3, 0, 0, 2> wxxz;
        Component<3, 0, 0, 3> wxxw;
        Component<3, 0, 1, 0> wxyx;
        Component<3, 0, 1, 1> wxyy;
        Component<3, 0, 1, 2> wxyz;
        Component<3, 0, 1, 3> wxyw;
        Component<3, 0, 2, 0> wxzx;
        Component<3, 0, 2, 1> wxzy;
        Component<3, 0, 2, 2> wxzz;
        Component<3, 0, 2, 3> wxzw;
        Component<3, 0, 3, 0> wxwx;
        Component<3, 0, 3, 1> wxwy;
        Component<3, 0, 3, 2> wxwz;
        Component<3, 0, 3, 3> wxww;
        Component<3, 1, 0, 0> wyxx;
        Component<3, 1, 0, 1> wyxy;
        Component<3, 1, 0, 2> wyxz;
        Component<3, 1, 0, 3> wyxw;
        Component<3, 1, 1, 0> wyyx;
        Component<3, 1, 1, 1> wyyy;
        Component<3, 1, 1, 2> wyyz;
        Component<3, 1, 1, 3> wyyw;
        Component<3, 1, 2, 0> wyzx;
        Component<3, 1, 2, 1> wyzy;
        Component<3, 1, 2, 2> wyzz;
        Component<3, 1, 2, 3> wyzw;
        Component<3, 1, 3, 0> wywx;
        Component<3, 1, 3, 1> wywy;
        Component<3, 1, 3, 2> wywz;
        Component<3, 1, 3, 3> wyww;
        Component<3, 2, 0, 0> wzxx;
        Component<3, 2, 0, 1> wzxy;
        Component<3, 2, 0, 2> wzxz;
        Component<3, 2, 0, 3> wzxw;
        Component<3, 2, 1, 0> wzyx;
        Component<3, 2, 1, 1> wzyy;
        Component<3, 2, 1, 2> wzyz;
        Component<3, 2, 1, 3> wzyw;
        Component<3, 2, 2, 0> wzzx;
        Component<3, 2, 2, 1> wzzy;
        Component<3, 2, 2, 2> wzzz;
        Component<3, 2, 2, 3> wzzw;
        Component<3, 2, 3, 0> wzwx;
        Component<3, 2, 3, 1> wzwy;
        Component<3, 2, 3, 2> wzwz;
        Component<3, 2, 3, 3> wzww;
        Component<3, 3, 0, 0> wwxx;
        Component<3, 3, 0, 1> wwxy;
        Component<3, 3, 0, 2> wwxz;
        Component<3, 3, 0, 3> wwxw;
        Component<3, 3, 1, 0> wwyx;
        Component<3, 3, 1, 1> wwyy;
        Component<3, 3, 1, 2> wwyz;
        Component<3, 3, 1, 3> wwyw;
        Component<3, 3, 2, 0> wwzx;
        Component<3, 3, 2, 1> wwzy;
        Component<3, 3, 2, 2> wwzz;
        Component<3, 3, 2, 3> wwzw;
        Component<3, 3, 3, 0> wwwx;
        Component<3, 3, 3, 1> wwwy;
        Component<3, 3, 3, 2> wwwz;
        Component<3, 3, 3, 3> wwww;
        /// @endcond
        ///@}
    };
};

}  // namespace asciirast::math
