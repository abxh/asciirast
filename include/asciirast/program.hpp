/**
 * @file program.hpp
 * @brief Definition of program interface
 */

#pragma once

#include <concepts>

#include "./detail/GeneratorInterface.hpp"
#include "./fragment.hpp"
#include "./program_token.hpp"

namespace asciirast {

/**
 * @brief uniform requirements, to discourage using references and use them as they are usually used
 */
template<class T>
concept ProgramInterface_UniformWithNoReferences = requires {
    typename T::Uniform;
} && std::is_copy_assignable_v<typename T::Uniform> && std::is_move_assignable_v<typename T::Uniform>;

/**
 * @brief on_vertex() function requirements
 */
template<class T>
concept ProgramInterface_VertexSupport = requires(const T t) {
    typename T::Uniform;
    typename T::Vertex;
    typename T::Varying;
    requires std::semiregular<typename T::Targets>;
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
    typename T::Uniform;
    typename T::Varying;
    typename T::Targets;
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
    typename T::FragmentContext;
    typename T::Uniform;
    typename T::Varying;
    typename T::Targets;
    []<typename... ValueTypes>(const FragmentContextGeneric<ValueTypes...>&) {
    }(std::declval<typename T::FragmentContext>());
    {
        t.on_fragment(std::declval<typename T::FragmentContext&>(),
                      std::declval<const typename T::Uniform&>(), //
                      std::declval<const ProjectedFragment<typename T::Varying>&>(),
                      std::declval<typename T::Targets&>()) //
    } -> detail::GeneratorInterface<FragmentToken>;
};

/**
 * @brief Concept to follow the (shader) program interface
 */
template<class T>
concept ProgramInterface = requires(const T t) {
    requires ProgramInterface_UniformWithNoReferences<T>;
    requires ProgramInterface_VertexSupport<T>;
    requires ProgramInterface_FragRegularSupport<T>;
} || requires(const T t) {
    requires ProgramInterface_UniformWithNoReferences<T>;
    requires ProgramInterface_VertexSupport<T>;
    requires ProgramInterface_FragCoroutineSupport<T>;
};

}; // namespace asciirast
