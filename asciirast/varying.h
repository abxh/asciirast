/**
 * @file varying.h
 * @brief Definition of Varying and related things
 *
 * on perspective corrected interpolation:
 * @li https://www.youtube.com/watch?v=1Dv2-cLAJXw (ChilliTomatoNoodle)
 * @li https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
 * @li https://www.cs.cornell.edu/courses/cs4620/2015fa/lectures/PerspectiveCorrectZU.pdf
 */

#pragma once

#include "../external/boost_pfr/include/boost/pfr.hpp"

#include "./math/types.h"

#include <type_traits>

namespace asciirast {

/**
 * @brief Empty varying type
 */
struct EmptyVarying
{};

/**
 * @brief Concept to follow the varying interface
 *
 * Varying are the interpolated attributes of verticies.
 */
template<typename T>
concept VaryingInterface = std::same_as<T, EmptyVarying> || requires(const T x) {
    { x + x } -> std::same_as<T>;
    { x * math::Float{ -1 } } -> std::same_as<T>;
    requires std::semiregular<T>;
};

/**
 * @brief Tag struct to derive varying operations if every member themselves fullfills VaryingInterface
 */
template<typename T>
    requires(std::is_aggregate_v<T> &&
             []<std::size_t... I>(const std::index_sequence<I...>&) {
                 return (VaryingInterface<boost::pfr::tuple_element_t<I, T>> && ...);
             }(std::make_index_sequence<boost::pfr::tuple_size_v<T>>()))
struct DeriveVaryingOps : std::false_type
{};

/**
 * @brief Helper macro to tag type for derivation of varying operations
 */
#define DERIVE_VARYING_OPS(T)                                                                                          \
    template<>                                                                                                         \
    struct asciirast::DeriveVaryingOps<T> : std::true_type                                                             \
    {};                                                                                                                \
    using asciirast::operator+;                                                                                        \
    using asciirast::operator*;

/**
 * @brief Linear interpolation of varying types
 */
template<VaryingInterface Varying>
static Varying
lerp_varying(const Varying& lhs, const Varying& rhs, const math::Float t)
{
    if constexpr (std::is_same_v<Varying, EmptyVarying>) {
        return EmptyVarying();
    } else {
        return lhs * (1 - t) + rhs * t;
    }
}

/**
 * @brief Linear interpolation of varying perspective corrected
 */
template<VaryingInterface Varying>
[[maybe_unused]]
static auto
lerp_projected_varying(const Varying& lhs,
                       const Varying& rhs,
                       const math::Float t,
                       const math::Float Z_inv0,
                       const math::Float Z_inv1,
                       const math::Float acc_Z_inv) -> Varying
{
    if constexpr (std::is_same_v<Varying, EmptyVarying>) {
        return EmptyVarying();
    } else {
        if (t == 0) return lhs;
        if (t == 1) return rhs;

        const auto l = lhs * Z_inv0;
        const auto r = rhs * Z_inv1;

        return (l * (1 - t) + r * t) * (1 / acc_Z_inv);
    }
}

/**
 * @brief Derived varying-varying addition operator
 */
template<typename Varying, std::enable_if_t<std::is_class_v<Varying> && DeriveVaryingOps<Varying>::value, int> = 0>
auto
operator+(const Varying& lhs, const Varying& rhs) -> Varying
{
    return [&]<std::size_t... I>(const std::index_sequence<I...>&) {
        return Varying{ (boost::pfr::get<I>(lhs) + boost::pfr::get<I>(rhs))... };
    }(std::make_index_sequence<boost::pfr::tuple_size_v<Varying>>());
}

/**
 * @brief Derived varying-scalar multiplication operator
 */
template<typename Varying, std::enable_if_t<std::is_class_v<Varying> && DeriveVaryingOps<Varying>::value, int> = 0>
auto
operator*(const Varying& lhs, const math::Float& scalar) -> Varying
{
    return [&]<std::size_t... I>(const std::index_sequence<I...>&) {
        return Varying{ (boost::pfr::get<I>(lhs) * scalar)... };
    }(std::make_index_sequence<boost::pfr::tuple_size_v<Varying>>());
}

};
