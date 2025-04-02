/**
 * @file Swizzled.h
 * @brief File with definition of the swizzled class
 *
 * This file contains the Swizzled class. With a single index provided, it is
 * specialized to behave as an number implicitly. With multiple indicies
 * provided, it supports in-place operations with other Swizzled and vectors of
 * same size, and can be explictly converted to a vector temporary copy with the
 * `.to_vec()` method.
 *
 * Inspiration:
 * https://kiorisyshen.github.io/2018/08/27/Vector%20Swizzling%20and%20Parameter%20Pack%20in%20C++/
 */

#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <ranges>

namespace asciirast::math {

/**
 * @brief Class to swizzle vector components
 *
 * This class takes the vector type as template parameter input, and defines
 * in-place operators and `.to_vec()` with it. The number of indicies given is
 * used to determine the size of this swizzled component.
 *
 * @tparam Vec      Vector type instansiated with the correct size and type.
 * @tparam N        Number of components in the vector
 * @tparam T        Type of components
 * @tparam Indicies The indicies
 */
template<class Vec, std::size_t N, typename T, std::size_t... Indicies>
    requires(sizeof...(Indicies) > 0 && ((Indicies < N) && ...))
class Swizzled
{
    static constexpr std::array INDICIES = { Indicies... };
    std::array<T, N> m_components;

    static consteval bool overlapping_indicies(const std::size_t i, const std::size_t j) { return i == j; }

    template<std::size_t... OtherIndicies>
    static consteval bool overlapping_indicies(const std::size_t i, const std::index_sequence<OtherIndicies...>& s2)
    {
        return (overlapping_indicies(i, OtherIndicies) || ...);
    }

    template<std::size_t... ThisIndicies, std::size_t... OtherIndicies>
    static consteval bool overlapping_indicies(const std::index_sequence<ThisIndicies...>& s1,
                                               const std::index_sequence<OtherIndicies...>& s2)
    {
        return (overlapping_indicies(ThisIndicies, s2) || ...);
    }

    T* data() { return &m_components[0]; }

    const T* data() const { return &m_components[0]; }

    template<std::size_t M, std::size_t... OtherIndicies>
    bool does_not_overlap(const Swizzled<Vec, M, T, OtherIndicies...>& that) const
    {
        return this->data() != that.data() ||
               (this->data() == that.data() &&
                !overlapping_indicies(std::index_sequence<Indicies...>{}, std::index_sequence<OtherIndicies...>{}));
    }

public:
    using value_type = T; ///@< value type

    /**
     * @brief Size of swizzled component
     */
    static constexpr std::size_t size() { return INDICIES.size(); }

    /**
     * @brief Convert this to a temporary vector copy
     */
    Vec to_vec() const { return Vec{ (*this) }; }

    /**
     * @brief Index the swizzled component.
     */
    T& operator[](const std::size_t i)
    {
        assert(i < this->size() && "index is inside bounds");

        return m_components[INDICIES[i]];
    }

    /**
     * @brief Index the swizzled component.
     */
    T operator[](const std::size_t i) const
    {
        assert(i < this->size() && "index is inside bounds");

        return m_components[INDICIES[i]];
    }

    /**
     * @brief Range of swizzled components
     */
    std::ranges::view auto range()
    {
        const auto func = [this](const std::size_t i) -> T& { return m_components[INDICIES[i]]; };

        return std::views::iota(0U, INDICIES.size()) | std::views::transform(func);
    }

    /**
     * @brief Range of swizzled components
     */
    std::ranges::view auto range() const
    {
        const auto func = [this](const std::size_t i) -> T { return m_components[INDICIES[i]]; };

        return std::views::iota(0U, INDICIES.size()) | std::views::transform(func);
    }

    /**
     * @brief Unary minus vector operator
     */
    Vec operator-() const { return -to_vec(); }

    /**
     * @brief In-place assignment with vector
     */
    Swizzled operator=(const Vec& that)
    {
        for (const std::size_t i : INDICIES) {
            m_components[i] = that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise addition with vector
     */
    Swizzled operator+=(const Vec& that)
    {
        for (const std::size_t i : INDICIES) {
            m_components[i] += that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with vector
     */
    Swizzled operator-=(const Vec& that)
    {
        for (const std::size_t i : INDICIES) {
            m_components[i] -= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with vector
     */
    Swizzled operator*=(const Vec& that)
    {
        for (const std::size_t i : INDICIES) {
            m_components[i] *= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise assignment with other Swizzled
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    Swizzled operator=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
    {
        assert(this->does_not_overlap(that) &&
               "inplace operations with no overlapping indicies. use .to_vec() if needed");

        for (const std::tuple<T&, const T> t : std::views::zip(this->range(), that.range())) {
            auto [dest, src] = t;

            dest = src;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise addition with other Swizzled
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    Swizzled operator+=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
    {
        assert(this->does_not_overlap(that) &&
               "inplace operations with no overlapping indicies. use .to_vec() if needed");

        for (const std::tuple<T&, const T> t : std::views::zip(this->range(), that.range())) {
            auto [dest, src] = t;

            dest += src;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with other Swizzled
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    Swizzled operator-=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
    {
        assert(this->does_not_overlap(that) &&
               "inplace operations with no overlapping indicies. use .to_vec() if needed");

        for (const std::tuple<T&, const T> t : std::views::zip(this->range(), that.range())) {
            auto [dest, src] = t;

            dest -= src;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with other Swizzled
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    Swizzled operator*=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
    {
        assert(this->does_not_overlap(that) &&
               "inplace operations with no overlapping indicies. use .to_vec() if needed");

        for (const std::tuple<T&, const T> t : std::views::zip(this->range(), that.range())) {
            auto [dest, src] = t;

            dest *= src;
        }
        return *this;
    }

    /**
     * @brief In-place scalar multiplication
     */
    Swizzled operator*=(const T scalar)
    {
        for (T& x : this->range()) {
            x *= scalar;
        }
        return *this;
    }

    /**
     * @brief In-place scalar division
     */
    Swizzled operator/=(const T scalar)
    {
        if constexpr (std::is_integral_v<T>) {
            assert(scalar != T{ 0 } && "non-zero division");
        }

        for (T& x : this->range()) {
            x /= scalar;
        }
        return *this;
    }
};

/**
 * @brief Specialized Swizzled class for single components, given a single
 * index.
 *
 * @tparam Vec   Vector type instansiated with the correct size and type.
 * @tparam N     Number of components in the vector
 * @tparam T     Type of components
 * @tparam INDEX The index
 */
template<class Vec, std::size_t N, typename T, std::size_t INDEX>
    requires(INDEX < N)
class Swizzled<Vec, N, T, INDEX>
{
    std::array<T, N> m_components;

public:
    using value_type = T; ///@< value type

    /**
     * @brief Size of swizzled component
     */
    static constexpr std::size_t size() { return 1UL; }

    /**
     * @brief Implicit conversion to number
     */
    operator T() const { return m_components[INDEX]; }

    /**
     * @brief Implicit conversion to number reference
     */
    operator T&() { return m_components[INDEX]; }

    /**
     * @brief Assignment from number
     */
    T operator=(const T value) { return (m_components[INDEX] = value); }

    /**
     * @brief Range of swizzled component
     */
    std::ranges::view auto range() { return std::ranges::views::single(m_components[INDEX]); }

    /**
     * @brief Range of swizzled component
     */
    std::ranges::view auto range() const { return std::ranges::views::single(m_components[INDEX]); }

    /**
     * @brief Convert this to a temporary vector copy
     */
    Vec to_vec() const { return Vec{ (*this) }; }

    /**
     * @brief Index the swizzled component.
     */
    T& operator[](const std::size_t i)
    {
        assert(i == 0UL && "index is inside bounds");

        return m_components[INDEX];
    }

    /**
     * @brief Index the swizzled component.
     */
    T operator[](const std::size_t i) const
    {
        assert(i == 0UL && "index is inside bounds");

        return m_components[INDEX];
    }
};

} // namespace asciirast::math
