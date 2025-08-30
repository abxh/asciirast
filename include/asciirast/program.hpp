/**
 * @file program.hpp
 * @brief Definition of program interface
 */

#pragma once

#include <concepts>

#include "./fragment.hpp"
#include "./program_token.hpp"

namespace asciirast {

/**
 * @brief Minimal Program requirements
 */
template<class T>
concept ProgramInterface_MinimalSupport = requires(const T t) {
    typename T::Uniform;
    typename T::Vertex;
    typename T::Varying;
    typename T::Targets;
    requires std::semiregular<typename T::Targets>;
    requires std::is_default_constructible_v<typename T::Uniform>;
    {
        t.on_vertex(std::declval<const typename T::Uniform&>(),     //
                    std::declval<const typename T::Vertex&>(),      //
                    std::declval<Fragment<typename T::Varying>&>()) //
    } -> std::same_as<void>;
};

/**
 * @brief Regular on_fragment() function requirements
 */
template<class T>
concept ProgramInterface_FragRegularSupport = requires(const T t) {
    requires ProgramInterface_MinimalSupport<T>;
    {
        t.on_fragment(std::declval<const typename T::Uniform&>(), //
                      std::declval<const ProjectedFragment<typename T::Varying>&>(),
                      std::declval<typename T::Targets&>()) //
    } -> std::same_as<void>;
};

/**
 * @brief Coroutine on_fragment() function requirements
 */
template<class T>
concept ProgramInterface_FragCoroutineSupport = requires(const T t) {
    requires ProgramInterface_MinimalSupport<T>;
    typename T::FragmentContext;
    []<typename... ValueTypes>(const FragmentContextType<ValueTypes...>&) {
    }(std::declval<typename T::FragmentContext>());
    {
        t.on_fragment(std::declval<typename T::FragmentContext&>(),
                      std::declval<const typename T::Uniform&>(), //
                      std::declval<const ProjectedFragment<typename T::Varying>&>(),
                      std::declval<typename T::Targets&>()) //
    } -> std::same_as<ProgramTokenGenerator>;
};

/**
 * @brief Concept to follow the (shader) program interface
 */
template<class T>
concept ProgramInterface = requires(const T t) {
    requires ProgramInterface_MinimalSupport<T>;
    requires ProgramInterface_FragRegularSupport<T>;
} || requires(const T t) {
    requires ProgramInterface_MinimalSupport<T>;
    requires ProgramInterface_FragCoroutineSupport<T>;
};

}; // namespace asciirast
