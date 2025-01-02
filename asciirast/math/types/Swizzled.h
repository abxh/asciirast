/**
 * @file Swizzled.h
 * @brief Swizzled class to swizzle components.
 *
 * Stumbled upon the core idea in:
 * https://kiorisyshen.github.io/2018/08/27/Vector%20Swizzling%20and%20Parameter%20Pack%20in%20C++/
 */

#pragma once

#include <array>

#include "VecLike.h"

namespace asciirast::math {

/**
 * @brief Constraint to check uniqueness among indicies.
 */
template <std::size_t I, std::size_t... Js>
struct unique_index {
    static constexpr bool value = ((I != Js) && ...);
};

/**
 * @brief Swizzled class to swizzle components
 */
template <template <int, typename> typename V,
          int N,
          typename T,
          std::size_t... Is>
    requires(N > 0)
class Swizzled : public VecLike<Swizzled<V, N, T, Is...>,
                                V<sizeof...(Is), T>,
                                sizeof...(Is),
                                T> {
private:
    std::array<T, N> m_components;

private:
    static constexpr const std::array indicies = {Is...};

    template <template <int, typename> typename W,
              int M,
              typename U,
              std::size_t... Js>
        requires(M > 0)
    friend class Swizzled;

    template <typename W, typename Res, int M, typename U>
        requires(M > 0)
    friend class VecLike;

    constexpr T index(int i) const { return this->m_components[indicies[i]]; }
    constexpr T& index(int i) { return this->m_components[indicies[i]]; }

public:
    /**
     * @brief Assignment from another swizzled
     */
    template <template <int, typename> typename W, int M, std::size_t... Js>
        requires(sizeof...(Is) == sizeof...(Js))
    auto operator=(const Swizzled<W, M, T, Js...>& that)
        requires(unique_index<Is, Js...>::value && ...)
    {
        for (int i = 0; i < sizeof...(Is); i++) {
            this->index(i) = that.index(i);
        }
        return *this;
    }

    /**
     * @brief Assignment from another vector.
     */
    auto operator=(const V<sizeof...(Is), T>& that) {
        for (int i = 0; i < sizeof...(Is); i++) {
            this->index(i) = that[i];
        }
        return *this;
    }

    /**
     * @brief Explicit conversion to vector copy (allowing code like v.xyz =
     * v.zyx.to_vec())
     */
    V<sizeof...(Is), T> to_vec() const { return V<sizeof...(Is), T>{*this}; }

    /**
     * @brief Explicit conversion to vector copy
     */
    explicit operator V<sizeof...(Is), T>() const {
        V<sizeof...(Is), T> res{};
        for (int i = 0; i < sizeof...(Is); i++) {
            res[i] = this->index(i);
        }
        return res;
    }
};

/**
 * @brief Single component specialization of Swizzled
 */
template <template <int, typename> typename VecClass,
          int N,
          typename T,
          std::size_t index>
    requires(N > 0)
class Swizzled<VecClass, N, T, index> {
private:
    std::array<T, N> m_components;

public:
    /**
     * @brief Implicit conversion to value type.
     */
    operator T() const { return m_components[index]; }

    /**
     * @brief Implicit conversion to value type.
     */
    operator T&() { return m_components[index]; }

    /**
     * @brief Implicit assignment from other value.
     */
    auto& operator=(const T value) { return (m_components[index] = value); }
};

}  // namespace asciirast::math
