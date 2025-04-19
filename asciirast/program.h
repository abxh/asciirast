/**
 * @file program.h
 * @brief Definition of program interface and other related types
 */

#pragma once

#include <cassert>
#include <concepts>
#include <generator>

#include "./fragment.h"
#include "./framebuffer.h"

namespace asciirast {

/**
 * @brief Abstract (shader) program that can be used to follow the
 *        program interface
 *
 * @tparam Uniform_ Arbitary type which contain constant values
 * @tparam Vertex_ Vertex type to be used to output a fragment
 * @tparam Varying_ Fragment type that follows the Varying interface
 * @tparam FrameBuffer Framebuffer type that follows the Framebuffer
 *                     interface
 * @tparam FragmentContextValues The types of values to prepare for
                                 in fragment context
 */
template<class Uniform_,
         class Vertex_,
         VaryingInterface Varying_,
         FrameBufferInterface FrameBuffer,
         typename... FragmentContextValues>
class AbstractProgram
{
public:
    using Uniform = Uniform_;             ///< uniform(s) type
    using Vertex = Vertex_;               ///< vertex type
    using Varying = Varying_;             ///< varying type
    using Targets = FrameBuffer::Targets; ///< user framebuffer targets

    using FragmentContext = FragmentContextType<FragmentContextValues...>; ///< fragment context

    /**
     * @brief Default virtual destructor
     */
    virtual ~AbstractProgram() = default;

    /**
     * @brief Function run on every vertex
     */
    virtual auto on_vertex(const Uniform&,
                           const Vertex&) const //
            -> Fragment<Varying> = 0;

    /**
     * @brief Function run on every fragment
     */
    virtual auto on_fragment(FragmentContext&,
                             const Uniform&,
                             const ProjectedFragment<Varying>&) const //
            -> std::generator<FragmentResult<Targets>> = 0;
};

/**
 * @brief Concept to follow the (shader) program interface
 *
 * @tparam T The type to check
 */
template<class T>
concept ProgramInterface = requires(const T t) {
    typename T::Uniform;
    typename T::Vertex;
    typename T::Varying;
    typename T::Targets;
    typename T::FragmentContext;
    {
        t.on_vertex(std::declval<const typename T::Uniform&>(), //
                    std::declval<const typename T::Vertex&>())  //
    } -> std::same_as<Fragment<typename T::Varying>>;
    {
        t.on_fragment(std::declval<typename T::FragmentContext&>(),                  //
                      std::declval<const typename T::Uniform&>(),                    //
                      std::declval<const ProjectedFragment<typename T::Varying>&>()) //
    } -> std::same_as<std::generator<FragmentResult<typename T::Targets>>>;
};

namespace detail {

/// @cond DO_NOT_DOCUMENT

template<ProgramInterface Program, class Uniform, class Vertex, FrameBufferInterface FrameBuffer>
struct can_use_program_with
{
    static constexpr bool value = std::is_same_v<typename Program::Uniform, Uniform> &&
                                  std::is_same_v<typename Program::Vertex, Vertex> &&
                                  std::is_same_v<typename Program::Targets, typename FrameBuffer::Targets>;
};

/// @endcond

}; // namespace detail

}; // namespace asciirast
