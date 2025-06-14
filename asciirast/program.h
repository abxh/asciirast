/**
 * @file program.h
 * @brief Definition of program interface
 */

#pragma once

#include <cassert>
#include <concepts>
#include <generator>

#include "./fragment.h"
#include "./framebuffer.h"
#include "./program_token.h"

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
 * @tparam ProgramTokenAllocator Program token allocator
 * @tparam FragmentContextValues Types to prepare FragmentContext for
 */
template<class Uniform_,
         class Vertex_,
         VaryingInterface Varying_,
         FrameBufferInterface FrameBuffer,
         typename ProgramTokenAllocator = std::allocator<ProgramToken>,
         typename... FragmentContextValues>
class AbstractProgram
{
public:
    using Uniform = Uniform_;                                              ///< uniform(s) type
    using Vertex = Vertex_;                                                ///< vertex type
    using Varying = Varying_;                                              ///< varying type
    using Targets = FrameBuffer::Targets;                                  ///< user framebuffer targets
    using FragmentContext = FragmentContextType<FragmentContextValues...>; ///< fragment context
    using ProgramTokenGenerator = std::generator<ProgramToken, void, ProgramTokenAllocator>; ///< ProgramToken generator

    /**
     * @brief Default virtual destructor
     */
    virtual ~AbstractProgram() = default;

    /**
     * @brief Function run on every vertex
     */
    virtual void on_vertex(const Uniform&, const Vertex&, Fragment<Varying>&) const = 0;

    /**
     * @brief Function run on every fragment
     */
    virtual auto on_fragment(FragmentContext&, const Uniform&, const ProjectedFragment<Varying>&, Targets&) const
            -> ProgramTokenGenerator = 0;
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
    requires std::semiregular<typename T::Targets>;
    typename T::FragmentContext;
    typename T::ProgramTokenGenerator;
    requires std::same_as<std::remove_cvref_t<typename T::ProgramTokenGenerator::yielded>, ProgramToken>;
    {
        t.on_vertex(std::declval<const typename T::Uniform&>(),     //
                    std::declval<const typename T::Vertex&>(),      //
                    std::declval<Fragment<typename T::Varying>&>()) //
    } -> std::same_as<void>;
    {
        t.on_fragment(std::declval<typename T::FragmentContext&>(),
                      std::declval<const typename T::Uniform&>(), //
                      std::declval<const ProjectedFragment<typename T::Varying>&>(),
                      std::declval<typename T::Targets&>()) //
    } -> std::same_as<typename T::ProgramTokenGenerator>;
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
}; // namespace detail
/// @endcond

}; // namespace asciirast
