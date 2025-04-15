/**
 * @file Swizzled.h
 * @brief File with definition of the swizzled class
 *
 * This file contains the Swizzled class. It supports in-place operations
 * with other Swizzled and vectors of same size, and can be explictly converted
 * to a vector temporary copy with the `.to_vec()` method.
 *
 * Inspiration:
 * https://kiorisyshen.github.io/2018/08/27/Vector%20Swizzling%20and%20Parameter%20Pack%20in%20C++/
 * https://jojendersie.de/performance-optimal-vector-swizzling-in-c/
 */

#pragma once

#include <array>
#include <cassert>

namespace asciirast::math {

/**
 * @brief Class for swizzled vector components
 *
 * This class takes the vector type as template parameter input.
 *
 * The number of indicies given is used to determine the size of
 * the resulting swizzled component.
 *
 * @tparam Vec      Vec assumed to be instantiated with the correct size and type.
 * @tparam N        Number of components in the vector
 * @tparam T        Type of components
 * @tparam Indicies The indicies
 */
template<class Vec, std::size_t N, typename T, std::size_t... Indicies>
    requires(sizeof...(Indicies) > 1U && ((Indicies < N) && ...))
class Swizzled
{
    static constexpr std::array IndiciesArray = { Indicies... };

private:
    template<std::size_t... Is>
    struct non_duplicate_indicies;

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

private:
    static consteval bool overlapping_indicies(const std::size_t i, const std::size_t j) { return i == j; }

    template<std::size_t... Js>
    static consteval bool overlapping_indicies(const std::size_t i, const std::index_sequence<Js...>& s2)
    {
        (void)(s2);
        return (overlapping_indicies(i, Js) || ...);
    }

    template<std::size_t... Is, std::size_t... Js>
    static consteval bool overlapping_indicies(const std::index_sequence<Is...>& s1,
                                               const std::index_sequence<Js...>& s2)
    {
        (void)(s1);
        return (overlapping_indicies(Is, s2) || ...);
    }

private:
    std::array<T, N> m_components;

public:
    /// value type
    using value_type = T;

    /// Whether, in case this is used as a lvalue, it has doesn't have duplicate indicies
    static constexpr bool lvalue_has_non_duplicate_indicies = non_duplicate_indicies<Indicies...>::value;

    /**
     * @brief Default constructor
     */
    constexpr Swizzled() = default;

    /**
     * @brief Default copy constructor
     */
    constexpr Swizzled(const Swizzled&) = default;

    /**
     * @brief Default move constructor
     */
    constexpr Swizzled(Swizzled&&) = default;

    /**
     * @brief Size of swizzled component
     *
     * @return number of indicies as size_t
     */
    [[nodiscard]] static constexpr std::size_t size() { return IndiciesArray.size(); }

    /**
     * @brief Underlying data pointer of the vector
     *
     * @return Pointer to the first element of the vector
     */
    [[nodiscard]] constexpr const T* data() const { return &m_components[0]; }

    /**
     * @brief Check if this does not overlap with another swizzled,
     * when both are part of the same vector
     *
     * @param that The other Swizzled
     * @return Whether the Swizzled objects overlap
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    [[nodiscard]] constexpr bool does_not_overlap(const Swizzled<Vec, M, T, OtherIndicies...>& that) const
    {
        constexpr bool indicies_do_not_overlap =
                !overlapping_indicies(std::index_sequence<Indicies...>{}, std::index_sequence<OtherIndicies...>{});

        return (this->data() != that.data()) || (this->data() == that.data() && indicies_do_not_overlap);
    }

    /**
     * @brief Convert this to a temporary vector copy
     *
     * @return A vector of size equal to the number of indicies
     */
    [[nodiscard]] constexpr Vec to_vec() const { return Vec{ (*this) }; }

    /**
     * @brief Index the swizzled component.
     *
     * @param i The index
     * @return A reference to the value at the index swizzled
     */
    [[nodiscard]] constexpr T& operator[](const std::size_t i)
    {
        assert(i < this->size() && "index is inside bounds");

        return m_components[IndiciesArray[i]];
    }

    /**
     * @brief Index the swizzled component.
     *
     * @param i The index
     * @return Value at the index swizzled
     */
    [[nodiscard]] constexpr T operator[](const std::size_t i) const
    {
        assert(i < this->size() && "index is inside bounds");

        return m_components[IndiciesArray[i]];
    }

    /**
     * @brief Unary minus vector operator
     *
     * @return The resulting vector of size equal to the number of indicies
     */
    [[nodiscard]] constexpr Vec operator+() const { return +to_vec(); }

    /**
     * @brief Unary minus vector operator
     *
     * @return The resulting vector of size equal to the number of indicies
     */
    [[nodiscard]] constexpr Vec operator-() const { return -to_vec(); }

    /**
     * @brief In-place assignment with vector
     *
     * @param that The vector
     * @return This
     */
    constexpr Swizzled& operator=(const Vec& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] = that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise addition with vector
     *
     * @param that The vector
     * @return This
     */
    constexpr Swizzled& operator+=(const Vec& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] += that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with vector
     *
     * @param that The vector
     * @return This
     */
    constexpr Swizzled& operator-=(const Vec& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] -= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with vector
     *
     * @param that The vector
     * @return This
     */
    constexpr Swizzled& operator*=(const Vec& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] *= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise assignment with other Swizzled of
     * same kind
     *
     * @note this is neccessary to define since c++ defaults to a
     * implicit non-templated assignment operator
     *
     * @param that Other swizzled object
     * @return This
     */
    constexpr Swizzled& operator=(const Swizzled& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        assert(this->does_not_overlap(that) &&
               "inplace operations with no overlapping indicies. use .to_vec() if needed");

        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] = that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise assignment with other Swizzled
     *
     * @param that Other swizzled object
     * @return This
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    constexpr Swizzled& operator=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        assert(this->does_not_overlap(that) &&
               "inplace operations with no overlapping indicies. use .to_vec() if needed");

        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] = that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise addition with other Swizzled
     *
     * @param that Other swizzled object
     * @return This
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    constexpr Swizzled& operator+=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        assert(this->does_not_overlap(that) &&
               "inplace operations with no overlapping indicies. use .to_vec() if needed");

        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] += that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with other Swizzled
     *
     * @param that Other swizzled object
     * @return This
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    constexpr Swizzled& operator-=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        assert(this->does_not_overlap(that) &&
               "inplace operations with no overlapping indicies. use .to_vec() if needed");

        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] -= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with other Swizzled
     *
     * @param that Other swizzled object
     * @return This
     */
    template<std::size_t M, std::size_t... OtherIndicies>
    constexpr Swizzled& operator*=(const Swizzled<Vec, M, T, OtherIndicies...>& that)
        requires(lvalue_has_non_duplicate_indicies)
    {
        assert(this->does_not_overlap(that) &&
               "inplace operations with no overlapping indicies. use .to_vec() if needed");

        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] *= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place scalar multiplication
     *
     * @param scalar Scalar value
     * @return This
     */
    constexpr Swizzled& operator*=(const T scalar)
        requires(lvalue_has_non_duplicate_indicies)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] *= scalar;
        }
        return *this;
    }

    /**
     * @brief In-place scalar division
     *
     * @param scalar Scalar value
     * @return This
     */
    constexpr Swizzled& operator/=(const T scalar)
        requires(lvalue_has_non_duplicate_indicies)
    {
        assert(scalar != 0 && "non-zero division");

        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] /= scalar;
        }
        return *this;
    }
};

/**
 * @brief Class for single vector component
 *
 * @tparam N     Number of components in the vector
 * @tparam T     Type of components
 * @tparam Index The index
 */
template<std::size_t N, typename T, std::size_t Index>
    requires(Index < N)
class SwizzledSingle
{
private:
    std::array<T, N> m_components;

public:
    /// value type
    using value_type = T;

    /**
     * @brief Default constructor
     */
    constexpr SwizzledSingle() = default;

    /**
     * @brief Default copy constructor
     */
    constexpr SwizzledSingle(const SwizzledSingle&) = default;

    /**
     * @brief Default move constructor
     */
    constexpr SwizzledSingle(SwizzledSingle&&) = default;

    /**
     * @brief Assignment from value
     *
     * @param value The new value at the swizzled index
     * @return This
     */
    constexpr SwizzledSingle& operator=(const T value)
    {
        m_components[Index] = value;
        return *this;
    }

    /**
     * @brief Assignment from other SwizzledSingle of same kind
     *
     * @note this is neccessary to define since c++ defaults to a
     * implicit non-templated assignment operator
     *
     * @param that Other SwizzledSingle object
     * @return This
     */
    constexpr SwizzledSingle& operator=(const SwizzledSingle& that)
    {
        m_components[Index] = T{ that };
        return *this;
    }

    /**
     * @brief Assignment from other SwizzledSingle component
     *
     * @param that Other SwizzledSingle object
     * @return This
     */
    template<std::size_t M, std::size_t OtherIndex>
    constexpr auto& operator=(const SwizzledSingle<M, T, OtherIndex>& that)
    {
        m_components[Index] = T{ that };
        return *this;
    }

    /**
     * @brief Implicit conversion to number
     *
     * @return The swizzled component
     */
    [[nodiscard]] constexpr operator T() const { return m_components[Index]; }

    /**
     * @brief Implicit conversion to number reference
     *
     * @return The reference to the swizzled component
     */
    [[nodiscard]] constexpr operator T&() { return m_components[Index]; }
};

} // namespace asciirast::math
