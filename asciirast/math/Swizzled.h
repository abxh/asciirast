/**
 * @file Swizzled.h
 * @brief File with definition of the swizzled class
 *
 * This file contains the Swizzled class. With a single index provided,
 * it is specialized to behave as an number implicitly. With multiple
 * indicies provided, it supports in-place operations with other
 * Swizzled and vectors of same size, and can be explictly converted to
 * a vector temporary copy with the
 * `.to_vec()` method.
 *
 * Inspiration:
 * - https://kiorisyshen.github.io/2018/08/27/Vector%20Swizzling%20and%20Parameter%20Pack%20in%20C++/
 * - https://jojendersie.de/performance-optimal-vector-swizzling-in-c/
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
 * This class takes the vector type as template parameter input, and
 * defines in-place operators and `.to_vec()` with it. The number of
 * indicies given is used to determine the size of this swizzled
 * component.
 *
 * @tparam Vec      Vector type instansiated with the correct size and
 * type.
 * @tparam N        Number of components in the vector
 * @tparam T        Type of components
 * @tparam Indicies The indicies
 */
template<class Vec, std::size_t N, typename T, std::size_t... Indicies>
    requires(sizeof...(Indicies) > 1U && ((Indicies < N) && ...))
class Swizzled
{
    static constexpr std::array IndiciesArray = { Indicies... };
    std::array<T, N> m_components;

    template<std::size_t... ThisIndicies>
    struct non_duplicate_indicies;

    static constexpr bool lvalue_has_non_duplicate_indicies = non_duplicate_indicies<Indicies...>::value;

    template<std::size_t M, std::size_t... OtherIndicies>
    bool does_not_overlap(const Swizzled<Vec, M, T, OtherIndicies...>& that) const
    {
        constexpr bool indicies_do_not_overlap =
                !overlapping_indicies(std::index_sequence<Indicies...>{}, std::index_sequence<OtherIndicies...>{});

        return (this->data() != that.data()) || (this->data() == that.data() && indicies_do_not_overlap);
    }

public:
    using value_type = T; ///@< value type

    /**
     * @brief Underlying data pointer
     */
    const T* data() const { return &m_components[0]; }

    /**
     * @brief Size of swizzled component
     */
    static constexpr std::size_t size() { return IndiciesArray.size(); }

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

        return m_components[IndiciesArray[i]];
    }

    /**
     * @brief Index the swizzled component.
     */
    T operator[](const std::size_t i) const
    {
        assert(i < this->size() && "index is inside bounds");

        return m_components[IndiciesArray[i]];
    }

    /**
     * @brief Range of swizzled components
     */
    std::ranges::view auto range()
    {
        const auto func = [this](const std::size_t i) -> T& { return m_components[IndiciesArray[i]]; };

        return std::views::iota(0U, IndiciesArray.size()) | std::views::transform(func);
    }

    /**
     * @brief Range of swizzled components
     */
    std::ranges::view auto range() const
    {
        const auto func = [this](const std::size_t i) -> T { return m_components[IndiciesArray[i]]; };

        return std::views::iota(0U, IndiciesArray.size()) | std::views::transform(func);
    }

    /**
     * @brief Unary minus vector operator
     */
    Vec operator-() const { return -to_vec(); }

    /**
     * @brief In-place assignment with vector
     */
    Swizzled& operator=(const Vec& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        for (const std::size_t i : IndiciesArray) {
            m_components[i] = that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise addition with vector
     */
    Swizzled& operator+=(const Vec& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        for (const std::size_t i : IndiciesArray) {
            m_components[i] += that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with vector
     */
    Swizzled& operator-=(const Vec& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        for (const std::size_t i : IndiciesArray) {
            m_components[i] -= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with vector
     */
    Swizzled& operator*=(const Vec& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        for (const std::size_t i : IndiciesArray) {
            m_components[i] *= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise assignment with other Swizzled of
     * same kind
     */
    Swizzled& operator=(const Swizzled& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        assert(this->does_not_overlap(that) && "inplace operations with no overlapping indicies. use "
                                               ".to_vec() if needed");

        const auto tups = std::views::zip(this->range(), that.range());

        for (const std::tuple<T&, const T> tup : tups) {
            auto [dest, src] = tup;

            dest = src;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise assignment with other Swizzled
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    Swizzled& operator=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        assert(this->does_not_overlap(that) && "inplace operations with no overlapping indicies. use "
                                               ".to_vec() if needed");

        const auto tups = std::views::zip(this->range(), that.range());

        for (const std::tuple<T&, const T> tup : tups) {
            auto [dest, src] = tup;

            dest = src;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise addition with other Swizzled
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    Swizzled& operator+=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        assert(this->does_not_overlap(that) && "inplace operations with no overlapping indicies. use "
                                               ".to_vec() if needed");

        const auto tups = std::views::zip(this->range(), that.range());

        for (const std::tuple<T&, const T> tup : tups) {
            auto [dest, src] = tup;
            dest += src;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with other Swizzled
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    Swizzled& operator-=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        assert(this->does_not_overlap(that) && "inplace operations with no overlapping indicies. use "
                                               ".to_vec() if needed");

        const auto tups = std::views::zip(this->range(), that.range());

        for (const std::tuple<T&, const T> tup : tups) {
            auto [dest, src] = tup;
            dest -= src;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with other Swizzled
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    Swizzled& operator*=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        assert(this->does_not_overlap(that) && "inplace operations with no overlapping indicies. use "
                                               ".to_vec() if needed");

        const auto tups = std::views::zip(this->range(), that.range());

        for (const std::tuple<T&, const T> tup : tups) {
            auto [dest, src] = tup;
            dest *= src;
        }
        return *this;
    }

    /**
     * @brief In-place scalar multiplication
     */
    Swizzled& operator*=(const T scalar)
        requires(lvalue_has_non_duplicate_indicies)
    {
        for (T& x : this->range()) {
            x *= scalar;
        }
        return *this;
    }

    /**
     * @brief In-place scalar division
     */
    Swizzled& operator/=(const T scalar)
        requires(lvalue_has_non_duplicate_indicies)
    {
        if constexpr (std::is_integral_v<T>) {
            assert(scalar != T{ 0 } && "non-zero division");
        }
        for (T& x : this->range()) {
            x /= scalar;
        }
        return *this;
    }

private:
    template<std::size_t First, std::size_t... Rest>
    struct non_duplicate_indicies<First, Rest...>
    {
        static constexpr bool value = (!((First == Rest) || ...)) && (non_duplicate_indicies<Rest...>::value);
    };

    template<std::size_t Last>
    struct non_duplicate_indicies<Last>
    {
        static constexpr bool value = true;
    };

    static consteval bool overlapping_indicies(const std::size_t i, const std::size_t j) { return i == j; }

    template<std::size_t... OtherIndicies>
    static consteval bool overlapping_indicies(const std::size_t i, const std::index_sequence<OtherIndicies...>& s2)
    {
        (void)(s2);
        return (overlapping_indicies(i, OtherIndicies) || ...);
    }

    template<std::size_t... ThisIndicies, std::size_t... OtherIndicies>
    static consteval bool overlapping_indicies(const std::index_sequence<ThisIndicies...>& s1,
                                               const std::index_sequence<OtherIndicies...>& s2)
    {
        (void)(s1);
        return (overlapping_indicies(ThisIndicies, s2) || ...);
    }
};

/**
 * @brief Class for single vector components
 *
 * @tparam N     Number of components in the vector
 * @tparam T     Type of components
 * @tparam Index The index
 */
template<std::size_t N, typename T, std::size_t Index>
    requires(Index < N)
class SingleVectorComponent
{
    std::array<T, N> m_components;

public:
    using value_type = T; ///@< value type

    /**
     * @brief Assignment from value
     */
    SingleVectorComponent& operator=(const T value)
    {
        m_components[Index] = value;
        return *this;
    }

    /**
     * @brief Assignment from other single swizzled component of same kind
     */
    SingleVectorComponent& operator=(const SingleVectorComponent& that)
    {
        m_components[Index] = T{ that };
        return *this;
    }

    /**
     * @brief Assignment from other single swizzled component
     */
    template<std::size_t M, std::size_t OtherIndex>
    auto& operator=(const SingleVectorComponent<M, T, OtherIndex>& that)
    {
        m_components[Index] = T{ that };
        return *this;
    }

    /**
     * @brief Implicit conversion to number
     */
    operator T() const { return m_components[Index]; }

    /**
     * @brief Implicit conversion to number reference
     */
    operator T&() { return m_components[Index]; }
};

} // namespace asciirast::math
