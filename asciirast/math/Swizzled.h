/**
 * @file Swizzled.h
 * @brief File with definition of the swizzled class
 *
 * This file contains the Swizzled class. With a single index provided, it is
 * specialized to behave as an number implicitly. With multiple indicies
 * provided, it supports in-place operations with other Swizzled and vectors of
 * same size, and can be explictly converted to a vector temporary copy with the
 * `.vec()` method.
 *
 * In-place operations with other swizzled with overlapping memory regions
 * (overlapping indicies) should be seen as UB. Use `.vec()` as needed.
 *
 * Assumes the vector type can copy construct the Swizzled type, and have a
 * indexing operator.
 */

#pragma once

#include <array>
#include <cassert>
#include <ranges>

#include "non_narrowing.h"

namespace asciirast::math {

/**
 * @brief Class to swizzle vector components
 *
 * This class takes the vector type as template parameter input, and defines
 * in-place operators and .vec() with it. The number of indicies given is used
 * to determine the size of this swizzled component.
 *
 * @tparam V    The vector type, instansiated with the correct size and type.
 * @tparam N    Number of components in the vector
 * @tparam T    Type of components
 * @tparam Is   The indicies
 */
template <class V, std::size_t N, typename T, std::size_t... Is>
    requires(N > 0 && std::is_arithmetic_v<T> && ((Is < N) && ...) &&
             sizeof...(Is) > 0)
class Swizzled {
private:
    static constexpr std::array indicies = {Is...};

private:
    std::array<T, N> m_components;

public:
    /**
     * @brief Convert this to a temporary vector copy
     */
    V vec() const { return V{*this}; }

    /**
     * @brief Index the swizzled component.
     */
    T& operator[](std::size_t i) noexcept { return m_components[indicies[i]]; }

    /**
     * @brief Index the swizzled component.
     */
    T operator[](std::size_t i) const noexcept {
        return m_components[indicies[i]];
    }

    /**
     * @brief Range of swizzled components
     */
    std::ranges::view auto range() {
        return std::views::iota(0U, indicies.size()) |
               std::views::transform([this](std::size_t i) {
                   return m_components[indicies[i]];
               });
    }

    /**
     * @brief Range of swizzled components
     */
    std::ranges::view auto range() const {
        return std::views::iota(0U, indicies.size()) |
               std::views::transform([this](std::size_t i) {
                   return m_components[indicies[i]];
               });
    }

    /**
     * @brief In-place assignment with vector
     */
    Swizzled operator=(const V& v) {
        for (auto i : indicies) {
            m_components[i] = v[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise addition with vector
     */
    Swizzled operator+=(const V& that) {
        for (auto i : indicies) {
            m_components[i] += that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with vector
     */
    Swizzled operator-=(const V& that) {
        for (auto i : indicies) {
            m_components[i] -= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with vector
     */
    Swizzled operator*=(const V& that) {
        for (auto i : indicies) {
            m_components[i] *= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise assignment with other Swizzled
     */
    template <class W, std::size_t M, std::size_t... Js>
    Swizzled operator=(const Swizzled<W, M, T, Js...>& that) {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x = y;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise addition with other Swizzled
     */
    template <class W, std::size_t M, std::size_t... Js>
    Swizzled operator+=(const Swizzled<W, M, T, Js...>& that) {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x += y;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with other Swizzled
     */
    template <class W, std::size_t M, std::size_t... Js>
    Swizzled operator-=(const Swizzled<W, M, T, Js...>& that) {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x -= y;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with other Swizzled
     */
    template <class W, std::size_t M, std::size_t... Js>
    Swizzled operator*=(const Swizzled<W, M, T, Js...>& that) {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x *= y;
        }
        return *this;
    }

    /**
     * @brief In-place scalar multiplication
     */
    template <typename U>
        requires(non_narrowing_v<U, T>)
    Swizzled operator*=(const U scalar) {
        for (auto x : this->range()) {
            x *= T{scalar};
        }
        return *this;
    }

    /**
     * @brief In-place scalar division
     */
    template <typename U>
        requires(non_narrowing_v<U, T>)
    Swizzled operator/=(const U scalar) {
        assert(T{scalar} != T{0} && "non-zero division");
        for (auto x : this->range()) {
            x /= T{scalar};
        }
        return *this;
    }
};

/**
 * @brief Specialized Swizzled class for single components, given a single
 * index.
 *
 * @tparam V     The vector type, instansiated with the correct size and type.
 * @tparam N     Number of components in the vector
 * @tparam T     Type of components
 * @tparam index The index
 */
template <class V, std::size_t N, typename T, std::size_t index>
    requires(N > 0 && std::is_arithmetic_v<T> && index < N)
class Swizzled<V, N, T, index> {
private:
    std::array<T, N> m_components;

public:
    /**
     * @brief Implicit conversion to number
     */
    constexpr operator T() const { return m_components[index]; }

    /**
     * @brief Implicit conversion to number reference
     */
    operator T&() { return m_components[index]; }

    /**
     * @brief Assignment from number
     */
    template <typename U>
        requires(non_narrowing_v<U, T>)
    T& operator=(const U value) {
        return (m_components[index] = T{value});
    }
};

}  // namespace asciirast::math
