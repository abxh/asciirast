/**
 * @file program.h
 * @brief Definition of program interface
 */

#pragma once

#include <cassert>
#include <concepts>

#include "./fragment.h"
#include "./framebuffer.h"
#include "./program_token.h"

namespace asciirast {

/// @cond DO_NOT_DOCUMENT
namespace detail {

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

template<class T>
concept ProgramInterface_FragRegularSupport = requires(const T t) {
    requires ProgramInterface_MinimalSupport<T>;
    {
        t.on_fragment(std::declval<const typename T::Uniform&>(), //
                      std::declval<const ProjectedFragment<typename T::Varying>&>(),
                      std::declval<typename T::Targets&>()) //
    } -> std::same_as<void>;
};

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

};
/// @endcond

/**
 * @brief Concept to follow the (shader) program interface
 *
 * @tparam T The type to check
 */
template<class T>
concept ProgramInterface = requires(const T t) {
    requires detail::ProgramInterface_MinimalSupport<T>;
    requires detail::ProgramInterface_FragRegularSupport<T>;
} || requires(const T t) {
    requires detail::ProgramInterface_MinimalSupport<T>;
    requires detail::ProgramInterface_FragCoroutineSupport<T>;
};

/// @cond DO_NOT_DOCUMENT
namespace detail {

template<ProgramInterface Program, class Uniform, class Vertex, FrameBufferInterface FrameBuffer>
struct can_use_program_with
{
    static constexpr bool value = std::is_same_v<typename Program::Uniform, Uniform> &&
                                  std::is_same_v<typename Program::Vertex, Vertex> &&
                                  std::is_same_v<typename Program::Targets, typename FrameBuffer::Targets>;
};

};
/// @endcond

}; // namespace asciirast
