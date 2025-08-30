/**
 * @file Swizzled.hpp
 * @brief File with definition of the swizzled class
 *
 * This file contains the Swizzled class. It can be explictly converted
 * to a vector temporary copy with the `.to_vec()` method.
 *
 * Inspiration:
 * https://kiorisyshen.github.io/2018/08/27/Vector%20Swizzling%20and%20Parameter%20Pack%20in%20C++/
 * https://jojendersie.de/performance-optimal-vector-swizzling-in-c/
 */

#pragma once

#include <array>
#include <type_traits>

#include "../detail/assert.hpp"

namespace asciirast::math {

/// @cond DO_NOT_DOCUMENT
namespace detail {

template<std::size_t... Is>
struct non_duplicate_indicies : std::true_type
{};

template<std::size_t First, std::size_t... Rest>
struct non_duplicate_indicies<First, Rest...>
    : std::bool_constant<((First != Rest) && ...) && non_duplicate_indicies<Rest...>::value>
{};

template<std::size_t Last>
struct non_duplicate_indicies<Last> : std::true_type
{};

template<std::size_t... Is>
static constexpr bool non_duplicate_indicies_v = non_duplicate_indicies<Is...>::value;

} // namespace detail
/// @endcond

/**
 * @brief Class for swizzled vector components
 *
 * This class takes the vector type as template parameter input.
 *
 * The number of indicies given is used to determine the size of
 * the resulting swizzled component.
 *
 * @tparam Vec      Vec assumed to be instantiated with the correct size
 * and type.
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

public:
    /// value type
    using value_type = T;

    /**
     * @name Default constructors / destructors
     * @{
     */
    constexpr ~Swizzled() = default;
    constexpr Swizzled() = default;
    constexpr Swizzled(const Swizzled&) = default;
    constexpr Swizzled(Swizzled&&) = default;
    /// @}

    /**
     * @brief In-place component-wise assignment with other Swizzled of
     * same kind
     */
    constexpr Swizzled& operator=(const Swizzled& that)
        requires(detail::non_duplicate_indicies_v<Indicies...>)
    {
        return (*this = Vec{ that });
    }

    /**
     * @brief In-place component-wise assignment with other Swizzled of
     * same kind
     */
    constexpr Swizzled& operator=(const Swizzled&& that)
        requires(detail::non_duplicate_indicies_v<Indicies...>)
    {
        return (*this = Vec{ that });
    }

    /**
     * @brief Size of swizzled component
     *
     * @return number of indicies as size_t
     */
    [[nodiscard]] static constexpr std::size_t size() { return IndiciesArray.size(); }

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
        ASCIIRAST_ASSERT(i < this->size(), "index is inside bounds", this->to_vec());

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
        ASCIIRAST_ASSERT(i < this->size(), "index is inside bounds", this->to_vec());

        return m_components[IndiciesArray[i]];
    }

    /**
     * @brief Unary minus vector operator
     *
     * @return The resulting vector of size equal to the number of
     * indicies
     */
    [[nodiscard]] constexpr Vec operator+() const { return +to_vec(); }

    /**
     * @brief Unary minus vector operator
     *
     * @return The resulting vector of size equal to the number of
     * indicies
     */
    [[nodiscard]] constexpr Vec operator-() const { return -to_vec(); }

    /**
     * @brief In-place assignment with vector
     *
     * @param that The vector
     * @return This
     */
    constexpr Swizzled& operator=(const Vec& that)
        requires(detail::non_duplicate_indicies_v<Indicies...>)
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
        requires(detail::non_duplicate_indicies_v<Indicies...>)
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
        requires(detail::non_duplicate_indicies_v<Indicies...>)
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
        requires(detail::non_duplicate_indicies_v<Indicies...>)
    {
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
        requires(detail::non_duplicate_indicies_v<Indicies...>)
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
        requires(detail::non_duplicate_indicies_v<Indicies...>)
    {
        ASCIIRAST_ASSERT(scalar != 0, "non-zero division", this->to_vec());

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
    std::array<T, N> m_components;

public:
    /// value type
    using value_type = T;

    /**
     * @name Default constructors / destructors
     * @{
     */
    constexpr ~SwizzledSingle() = default;
    constexpr SwizzledSingle() = default;
    constexpr SwizzledSingle(const SwizzledSingle&) = default;
    constexpr SwizzledSingle(SwizzledSingle&&) = default;
    /// @}

    /**
     * @brief Size of swizzled component
     *
     * @return number of indicies as size_t
     */
    [[nodiscard]] static constexpr std::size_t size() { return 1U; }

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
