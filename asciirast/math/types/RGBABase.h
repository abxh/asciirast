/**
 * @file RGBA.h
 * @brief RGB-Color base class with support for swizzled components.
 */

#pragma once

#include <array>

#include "VecBase.h"

namespace asciirast::math {

/**
 * @brief Four-dimensional RGB-Color base class with support for swizzled {r, g,
 * b, a} components.
 *
 * @tparam T Value type.
 */
template <typename T>
class RGBABase {
private:
    template <size_t... Indicies>
    using Component = SwizzledComponents<4, T, Indicies...>;

public:
    /**
     * @brief Value type
     */
    using value_type = T;

    /**
     * @brief Number of components
     */
    static constexpr size_t size() { return 4; }

public:
    /**
     * @brief Anonymous union between an array of components and swizzled
     * components.
     */
    union {
        std::array<T, 4> m_components;  ///< component array

        /**
         * @name Swizzled components
         * @{
         * Combinations of {r, g, b, a} with a maximum size of 4 as class
         * members.
         */
        Component<0> r;
        Component<1> g;
        Component<2> b;
        Component<3> a;

        // the rest are not documented to spare doxygen of generating redundant
        // documentation
        /// @cond DO_NO_DOCUMENT
        Component<0, 0> rr;
        Component<0, 1> rg;
        Component<0, 2> rb;
        Component<0, 3> ra;
        Component<1, 0> gr;
        Component<1, 1> gg;
        Component<1, 2> gb;
        Component<1, 3> ga;
        Component<2, 0> br;
        Component<2, 1> bg;
        Component<2, 2> bb;
        Component<2, 3> ba;
        Component<3, 0> ar;
        Component<3, 1> ag;
        Component<3, 2> ab;
        Component<3, 3> aa;

        Component<0, 0, 0> rrr;
        Component<0, 0, 1> rrg;
        Component<0, 0, 2> rrb;
        Component<0, 0, 3> rra;
        Component<0, 1, 0> rgr;
        Component<0, 1, 1> rgg;
        Component<0, 1, 2> rgb;
        Component<0, 1, 3> rga;
        Component<0, 2, 0> rbr;
        Component<0, 2, 1> rbg;
        Component<0, 2, 2> rbb;
        Component<0, 2, 3> rba;
        Component<0, 3, 0> rar;
        Component<0, 3, 1> rag;
        Component<0, 3, 2> rab;
        Component<0, 3, 3> raa;
        Component<1, 0, 0> grr;
        Component<1, 0, 1> grg;
        Component<1, 0, 2> grb;
        Component<1, 0, 3> gra;
        Component<1, 1, 0> ggr;
        Component<1, 1, 1> ggg;
        Component<1, 1, 2> ggb;
        Component<1, 1, 3> gga;
        Component<1, 2, 0> gbr;
        Component<1, 2, 1> gbg;
        Component<1, 2, 2> gbb;
        Component<1, 2, 3> gba;
        Component<1, 3, 0> gar;
        Component<1, 3, 1> gag;
        Component<1, 3, 2> gab;
        Component<1, 3, 3> gaa;
        Component<2, 0, 0> brr;
        Component<2, 0, 1> brg;
        Component<2, 0, 2> brb;
        Component<2, 0, 3> bra;
        Component<2, 1, 0> bgr;
        Component<2, 1, 1> bgg;
        Component<2, 1, 2> bgb;
        Component<2, 1, 3> bga;
        Component<2, 2, 0> bbr;
        Component<2, 2, 1> bbg;
        Component<2, 2, 2> bbb;
        Component<2, 2, 3> bba;
        Component<2, 3, 0> bar;
        Component<2, 3, 1> bag;
        Component<2, 3, 2> bab;
        Component<2, 3, 3> baa;
        Component<3, 0, 0> arr;
        Component<3, 0, 1> arg;
        Component<3, 0, 2> arb;
        Component<3, 0, 3> ara;
        Component<3, 1, 0> agr;
        Component<3, 1, 1> agg;
        Component<3, 1, 2> agb;
        Component<3, 1, 3> aga;
        Component<3, 2, 0> abr;
        Component<3, 2, 1> abg;
        Component<3, 2, 2> abb;
        Component<3, 2, 3> aba;
        Component<3, 3, 0> aar;
        Component<3, 3, 1> aag;
        Component<3, 3, 2> aab;
        Component<3, 3, 3> aaa;

        Component<0, 0, 0, 0> rrrr;
        Component<0, 0, 0, 1> rrrg;
        Component<0, 0, 0, 2> rrrb;
        Component<0, 0, 0, 3> rrra;
        Component<0, 0, 1, 0> rrgr;
        Component<0, 0, 1, 1> rrgg;
        Component<0, 0, 1, 2> rrgb;
        Component<0, 0, 1, 3> rrga;
        Component<0, 0, 2, 0> rrbr;
        Component<0, 0, 2, 1> rrbg;
        Component<0, 0, 2, 2> rrbb;
        Component<0, 0, 2, 3> rrba;
        Component<0, 0, 3, 0> rrar;
        Component<0, 0, 3, 1> rrag;
        Component<0, 0, 3, 2> rrab;
        Component<0, 0, 3, 3> rraa;
        Component<0, 1, 0, 0> rgrr;
        Component<0, 1, 0, 1> rgrg;
        Component<0, 1, 0, 2> rgrb;
        Component<0, 1, 0, 3> rgra;
        Component<0, 1, 1, 0> rggr;
        Component<0, 1, 1, 1> rggg;
        Component<0, 1, 1, 2> rggb;
        Component<0, 1, 1, 3> rgga;
        Component<0, 1, 2, 0> rgbr;
        Component<0, 1, 2, 1> rgbg;
        Component<0, 1, 2, 2> rgbb;
        Component<0, 1, 2, 3> rgba;
        Component<0, 1, 3, 0> rgar;
        Component<0, 1, 3, 1> rgag;
        Component<0, 1, 3, 2> rgab;
        Component<0, 1, 3, 3> rgaa;
        Component<0, 2, 0, 0> rbrr;
        Component<0, 2, 0, 1> rbrg;
        Component<0, 2, 0, 2> rbrb;
        Component<0, 2, 0, 3> rbra;
        Component<0, 2, 1, 0> rbgr;
        Component<0, 2, 1, 1> rbgg;
        Component<0, 2, 1, 2> rbgb;
        Component<0, 2, 1, 3> rbga;
        Component<0, 2, 2, 0> rbbr;
        Component<0, 2, 2, 1> rbbg;
        Component<0, 2, 2, 2> rbbb;
        Component<0, 2, 2, 3> rbba;
        Component<0, 2, 3, 0> rbar;
        Component<0, 2, 3, 1> rbag;
        Component<0, 2, 3, 2> rbab;
        Component<0, 2, 3, 3> rbaa;
        Component<0, 3, 0, 0> rarr;
        Component<0, 3, 0, 1> rarg;
        Component<0, 3, 0, 2> rarb;
        Component<0, 3, 0, 3> rara;
        Component<0, 3, 1, 0> ragr;
        Component<0, 3, 1, 1> ragg;
        Component<0, 3, 1, 2> ragb;
        Component<0, 3, 1, 3> raga;
        Component<0, 3, 2, 0> rabr;
        Component<0, 3, 2, 1> rabg;
        Component<0, 3, 2, 2> rabb;
        Component<0, 3, 2, 3> raba;
        Component<0, 3, 3, 0> raar;
        Component<0, 3, 3, 1> raag;
        Component<0, 3, 3, 2> raab;
        Component<0, 3, 3, 3> raaa;

        // copy of above with slight change:
        Component<1, 0, 0, 0> grrr;
        Component<1, 0, 0, 1> grrg;
        Component<1, 0, 0, 2> grrb;
        Component<1, 0, 0, 3> grra;
        Component<1, 0, 1, 0> grgr;
        Component<1, 0, 1, 1> grgg;
        Component<1, 0, 1, 2> grgb;
        Component<1, 0, 1, 3> grga;
        Component<1, 0, 2, 0> grbr;
        Component<1, 0, 2, 1> grbg;
        Component<1, 0, 2, 2> grbb;
        Component<1, 0, 2, 3> grba;
        Component<1, 0, 3, 0> grar;
        Component<1, 0, 3, 1> grag;
        Component<1, 0, 3, 2> grab;
        Component<1, 0, 3, 3> graa;
        Component<1, 1, 0, 0> ggrr;
        Component<1, 1, 0, 1> ggrg;
        Component<1, 1, 0, 2> ggrb;
        Component<1, 1, 0, 3> ggra;
        Component<1, 1, 1, 0> gggr;
        Component<1, 1, 1, 1> gggg;
        Component<1, 1, 1, 2> gggb;
        Component<1, 1, 1, 3> ggga;
        Component<1, 1, 2, 0> ggbr;
        Component<1, 1, 2, 1> ggbg;
        Component<1, 1, 2, 2> ggbb;
        Component<1, 1, 2, 3> ggba;
        Component<1, 1, 3, 0> ggar;
        Component<1, 1, 3, 1> ggag;
        Component<1, 1, 3, 2> ggab;
        Component<1, 1, 3, 3> ggaa;
        Component<1, 2, 0, 0> gbrr;
        Component<1, 2, 0, 1> gbrg;
        Component<1, 2, 0, 2> gbrb;
        Component<1, 2, 0, 3> gbra;
        Component<1, 2, 1, 0> gbgr;
        Component<1, 2, 1, 1> gbgg;
        Component<1, 2, 1, 2> gbgb;
        Component<1, 2, 1, 3> gbga;
        Component<1, 2, 2, 0> gbbr;
        Component<1, 2, 2, 1> gbbg;
        Component<1, 2, 2, 2> gbbb;
        Component<1, 2, 2, 3> gbba;
        Component<1, 2, 3, 0> gbar;
        Component<1, 2, 3, 1> gbag;
        Component<1, 2, 3, 2> gbab;
        Component<1, 2, 3, 3> gbaa;
        Component<1, 3, 0, 0> garr;
        Component<1, 3, 0, 1> garg;
        Component<1, 3, 0, 2> garb;
        Component<1, 3, 0, 3> gara;
        Component<1, 3, 1, 0> gagr;
        Component<1, 3, 1, 1> gagg;
        Component<1, 3, 1, 2> gagb;
        Component<1, 3, 1, 3> gaga;
        Component<1, 3, 2, 0> gabr;
        Component<1, 3, 2, 1> gabg;
        Component<1, 3, 2, 2> gabb;
        Component<1, 3, 2, 3> gaba;
        Component<1, 3, 3, 0> gaar;
        Component<1, 3, 3, 1> gaag;
        Component<1, 3, 3, 2> gaab;
        Component<1, 3, 3, 3> gaaa;

        // copy of above with slight change:
        Component<2, 0, 0, 0> brrr;
        Component<2, 0, 0, 1> brrg;
        Component<2, 0, 0, 2> brrb;
        Component<2, 0, 0, 3> brra;
        Component<2, 0, 1, 0> brgr;
        Component<2, 0, 1, 1> brgg;
        Component<2, 0, 1, 2> brgb;
        Component<2, 0, 1, 3> brga;
        Component<2, 0, 2, 0> brbr;
        Component<2, 0, 2, 1> brbg;
        Component<2, 0, 2, 2> brbb;
        Component<2, 0, 2, 3> brba;
        Component<2, 0, 3, 0> brar;
        Component<2, 0, 3, 1> brag;
        Component<2, 0, 3, 2> brab;
        Component<2, 0, 3, 3> braa;
        Component<2, 1, 0, 0> bgrr;
        Component<2, 1, 0, 1> bgrg;
        Component<2, 1, 0, 2> bgrb;
        Component<2, 1, 0, 3> bgra;
        Component<2, 1, 1, 0> bggr;
        Component<2, 1, 1, 1> bggg;
        Component<2, 1, 1, 2> bggb;
        Component<2, 1, 1, 3> bgga;
        Component<2, 1, 2, 0> bgbr;
        Component<2, 1, 2, 1> bgbg;
        Component<2, 1, 2, 2> bgbb;
        Component<2, 1, 2, 3> bgba;
        Component<2, 1, 3, 0> bgar;
        Component<2, 1, 3, 1> bgag;
        Component<2, 1, 3, 2> bgab;
        Component<2, 1, 3, 3> bgaa;
        Component<2, 2, 0, 0> bbrr;
        Component<2, 2, 0, 1> bbrg;
        Component<2, 2, 0, 2> bbrb;
        Component<2, 2, 0, 3> bbra;
        Component<2, 2, 1, 0> bbgr;
        Component<2, 2, 1, 1> bbgg;
        Component<2, 2, 1, 2> bbgb;
        Component<2, 2, 1, 3> bbga;
        Component<2, 2, 2, 0> bbbr;
        Component<2, 2, 2, 1> bbbg;
        Component<2, 2, 2, 2> bbbb;
        Component<2, 2, 2, 3> bbba;
        Component<2, 2, 3, 0> bbar;
        Component<2, 2, 3, 1> bbag;
        Component<2, 2, 3, 2> bbab;
        Component<2, 2, 3, 3> bbaa;
        Component<2, 3, 0, 0> barr;
        Component<2, 3, 0, 1> barg;
        Component<2, 3, 0, 2> barb;
        Component<2, 3, 0, 3> bara;
        Component<2, 3, 1, 0> bagr;
        Component<2, 3, 1, 1> bagg;
        Component<2, 3, 1, 2> bagb;
        Component<2, 3, 1, 3> baga;
        Component<2, 3, 2, 0> babr;
        Component<2, 3, 2, 1> babg;
        Component<2, 3, 2, 2> babb;
        Component<2, 3, 2, 3> baba;
        Component<2, 3, 3, 0> baar;
        Component<2, 3, 3, 1> baag;
        Component<2, 3, 3, 2> baab;
        Component<2, 3, 3, 3> baaa;

        // copy of above with slight change:
        Component<3, 0, 0, 0> arrr;
        Component<3, 0, 0, 1> arrg;
        Component<3, 0, 0, 2> arrb;
        Component<3, 0, 0, 3> arra;
        Component<3, 0, 1, 0> argr;
        Component<3, 0, 1, 1> argg;
        Component<3, 0, 1, 2> argb;
        Component<3, 0, 1, 3> arga;
        Component<3, 0, 2, 0> arbr;
        Component<3, 0, 2, 1> arbg;
        Component<3, 0, 2, 2> arbb;
        Component<3, 0, 2, 3> arba;
        Component<3, 0, 3, 0> arar;
        Component<3, 0, 3, 1> arag;
        Component<3, 0, 3, 2> arab;
        Component<3, 0, 3, 3> araa;
        Component<3, 1, 0, 0> agrr;
        Component<3, 1, 0, 1> agrg;
        Component<3, 1, 0, 2> agrb;
        Component<3, 1, 0, 3> agra;
        Component<3, 1, 1, 0> aggr;
        Component<3, 1, 1, 1> aggg;
        Component<3, 1, 1, 2> aggb;
        Component<3, 1, 1, 3> agga;
        Component<3, 1, 2, 0> agbr;
        Component<3, 1, 2, 1> agbg;
        Component<3, 1, 2, 2> agbb;
        Component<3, 1, 2, 3> agba;
        Component<3, 1, 3, 0> agar;
        Component<3, 1, 3, 1> agag;
        Component<3, 1, 3, 2> agab;
        Component<3, 1, 3, 3> agaa;
        Component<3, 2, 0, 0> abrr;
        Component<3, 2, 0, 1> abrg;
        Component<3, 2, 0, 2> abrb;
        Component<3, 2, 0, 3> abra;
        Component<3, 2, 1, 0> abgr;
        Component<3, 2, 1, 1> abgg;
        Component<3, 2, 1, 2> abgb;
        Component<3, 2, 1, 3> abga;
        Component<3, 2, 2, 0> abbr;
        Component<3, 2, 2, 1> abbg;
        Component<3, 2, 2, 2> abbb;
        Component<3, 2, 2, 3> abba;
        Component<3, 2, 3, 0> abar;
        Component<3, 2, 3, 1> abag;
        Component<3, 2, 3, 2> abab;
        Component<3, 2, 3, 3> abaa;
        Component<3, 3, 0, 0> aarr;
        Component<3, 3, 0, 1> aarg;
        Component<3, 3, 0, 2> aarb;
        Component<3, 3, 0, 3> aara;
        Component<3, 3, 1, 0> aagr;
        Component<3, 3, 1, 1> aagg;
        Component<3, 3, 1, 2> aagb;
        Component<3, 3, 1, 3> aaga;
        Component<3, 3, 2, 0> aabr;
        Component<3, 3, 2, 1> aabg;
        Component<3, 3, 2, 2> aabb;
        Component<3, 3, 2, 3> aaba;
        Component<3, 3, 3, 0> aaar;
        Component<3, 3, 3, 1> aaag;
        Component<3, 3, 3, 2> aaab;
        Component<3, 3, 3, 3> aaaa;
        /// @endcond
        ///@}
    };

public:
    /**
     * @brief Copy constructor from vector supporting implicit conversions
     */
    RGBABase(const Vec<4, T>& v) {
        for (size_t i = 0; i < 4; i++) {
            this->m_components[i] = v[i];
        }
    }

    /**
     * @brief Explicit conversion to vector
     * @returns A copy of the vector corresponding to the rgba components.
     */
    Vec<4, T> to_vec() const { return Vec<4, T>{this->m_components.begin()}; }

    /**
     * @brief Implicit conversion to vector
     * @returns A copy of the vector corresponding to the rgba components.
     */
    operator Vec<4, T>() const { return this->to_vec(); }

    /**
     * @brief Assignment from a vector.
     * @param rhs Vector rvalue from which to set the rgba components.
     * @returns this as reference
     */
    RGBABase& operator=(const Vec<4, T>& rhs) {
        for (size_t i = 0; i < 4; i++) {
            this->m_components[i] = rhs[i];
        }
        return *this;
    }

    /**
     * @name vec like operator support
     * @{
     */
    friend std::ostream& operator<<(std::ostream& out, const RGBABase& v) {
        return out << Vec<4, T>{v};
    }
    T& operator[](size_t i) { return m_components[i]; }
    const T& operator[](size_t i) const { return m_components[i]; }
    auto& operator=(const RGBABase& rhs) {
        for (size_t i = 0; i < 4; i++) {
            this->m_components[i] = rhs[i];
        }
        return *this;
    }
    auto& operator+=(const RGBABase& rhs) {
        for (size_t i = 0; i < 4; i++) {
            this->m_components[i] += rhs[i];
        }
        return *this;
    }
    auto& operator-=(const RGBABase& rhs) {
        for (size_t i = 0; i < 4; i++) {
            this->m_components[i] -= rhs[i];
        }
        return *this;
    }
    auto& operator*=(const RGBABase& rhs) {
        for (size_t i = 0; i < 4; i++) {
            this->m_components[i] -= rhs[i];
        }
        return *this;
    }
    // template <std::size_t M, std::size_t... Js>
    //     requires(sizeof...(Indicies) == sizeof...(Js))
    // auto& operator*=(const Other<M, Js...>& rhs) {
    //     auto other_indicies = Other<M, Js...>::indicies;
    //     for (auto [l, r] : std::views::zip(indicies, other_indicies)) {
    //         this->m_components[l] *= rhs.m_components[r];
    //     }
    //     return *this;
    // }
    // template <typename U>
    //     requires(non_narrowing_conv<T, U>)
    // auto& operator*=(const U& rhs) {
    //     auto scalar = T{rhs};
    //     for (auto i : indicies) {
    //         this->m_components[i] *= scalar;
    //     }
    //     return *this;
    // }
    // template <typename U>
    //     requires(non_narrowing_conv<T, U>)
    // auto& operator/=(const U& rhs) {
    //     auto scalar = T{rhs};
    //     for (auto i : indicies) {
    //         this->m_components[i] /= scalar;
    //     }
    //     return *this;
    // }
    // // clang-format off
    // template <typename U> requires(non_narrowing_conv<U, T>) friend VecRes
    // operator*(const U& scalar, const This& v) { return scalar * VecRes{v}; }
    // template <typename U> requires(non_narrowing_conv<U, T>) friend VecRes
    // operator*(const This& v, const U& scalar) { return VecRes{v} * scalar; }
    // template <typename U> requires(non_narrowing_conv<U, T>) friend VecRes
    // operator/(const This& v, const U& scalar) { return VecRes{v} / scalar; }
    // template <std::size_t M, std::size_t... Js> requires(sizeof...(Indicies)
    // == sizeof...(Js)) friend VecRes operator+(const This& lhs, const Other<M,
    // Js...>& rhs) { return VecRes{lhs} + VecRes{rhs}; } template <std::size_t
    // M, std::size_t... Js> requires(sizeof...(Indicies) == sizeof...(Js))
    // friend VecRes operator-(const This& lhs, const Other<M, Js...>& rhs) {
    // return VecRes{lhs} - VecRes{rhs}; } template <std::size_t M,
    // std::size_t... Js> requires(sizeof...(Indicies) == sizeof...(Js)) friend
    // VecRes operator*(const This& lhs, const Other<M, Js...>& rhs) { return
    // VecRes{lhs} * VecRes{rhs}; } template <std::size_t M, std::size_t... Js>
    // requires(sizeof...(Indicies) == sizeof...(Js)) friend VecRes
    // operator==(const This& lhs, const Other<M, Js...>& rhs)
    // requires(std::is_integral_v<T>) { return VecRes{lhs} == VecRes{rhs}; }
    // // clang-format on
    // /// @}
};

}  // namespace asciirast::math
