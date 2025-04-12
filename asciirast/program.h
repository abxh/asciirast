/**
 * @file program.h
 * @brief Definition of program interface and other related types
 */

#pragma once

#include <cassert>
#include <concepts>

#include "./framebuffer.h"
#include "./math/types.h"

namespace asciirast {

/**
 * @brief Concept to follow the varying interface
 *
 * Varying are the interpolated attributes of verticies.
 *
 * @tparam T The type to check
 */
template<typename T>
concept VaryingInterface = requires(const T x, T y) {
    { x + x } -> std::same_as<T>;
    { x * math::Float{ -1 } } -> std::same_as<T>;
    { y = x } -> std::same_as<T&>;
    { T() } -> std::same_as<T>;
};

/**
 * @brief Linear interpolation of varying types
 *
 * @param a Left hand side varying
 * @param b Right hand side varying
 * @param t How much to interpolate between the two
 * @return The interpolated varying
 */
template<VaryingInterface Varying>
static Varying
lerp_varying(const Varying& a, const Varying& b, const math::Float t)
{
    return a * (1 - t) + b * t;
}

/**
 * @brief Fragment type
 */
template<VaryingInterface Varying>
struct Fragment
{
    math::Vec4 pos; ///< world position in homogenous space
    Varying attrs;  ///< vertex attributes
};

/**
 * @brief Projected fragment type
 */
template<VaryingInterface Varying>
struct ProjectedFragment
{
    math::Vec2 pos;    ///< window position
    math::Float depth; ///< aka z
    math::Float Z_inv; ///< aka 1/w
    Varying attrs;     ///< fragment attributes
};

/**
 * @brief Project a Fragment to convert it to ProjectFragment
 *
 * @param frag The Fragment object
 * @return The ProjectedFragment object
 */
template<VaryingInterface T>
static ProjectedFragment<T>
project_fragment(const Fragment<T>& frag)
{
    assert(frag.pos.w != 0 && "non-zero w coordinate."
                              "this fragment should be culled by this point");

    const auto Z_inv = 1 / frag.pos.w;
    const auto v = frag.pos.xyz * Z_inv;

    return ProjectedFragment<T>{
        .pos = v.xy,
        .depth = v.z,
        .Z_inv = Z_inv,
        .attrs = frag.attrs,
    };
}

/**
 * @brief Abstract (shader) program that can be used to follow the
 *        program interface
 *
 * @tparam UniformType Arbitary type which contain constant values
 * @tparam VertexType  Vertex type to be used to output a fragment
 * @tparam VaryingType Fragment type that follows the Varying interface
 * @tparam FrameBuffer Framebuffer type that follows the Framebuffer
 *                     interface
 */
template<class UniformType, class VertexType, VaryingInterface VaryingType, FrameBufferInterface FrameBuffer>
class AbstractProgram
{
public:
    using Uniform = UniformType;          ///< uniform(s) type
    using Vertex = VertexType;            ///< vertex type
    using Varying = VaryingType;          ///< varying type
    using Targets = FrameBuffer::Targets; ///< framebuffer targets

    /**
     * @brief Default virtual destructor
     */
    virtual ~AbstractProgram() = default;

    /**
     * @brief Function run on every vertex
     */
    virtual Fragment<VaryingType> on_vertex(const UniformType&, const VertexType&) const = 0;

    /**
     * @brief Function run on every fragment (or pixel)
     */
    virtual Targets on_fragment(const UniformType&, const ProjectedFragment<VaryingType>&) const = 0;
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
    {
        t.on_vertex(std::declval<const typename T::Uniform&>(), std::declval<const typename T::Vertex&>())
    } -> std::same_as<Fragment<typename T::Varying>>;
    {
        t.on_fragment(std::declval<const typename T::Uniform&>(),
                      std::declval<const ProjectedFragment<typename T::Varying>&>())
    } -> std::same_as<typename T::Targets>;
};

namespace detail {

/// @cond DO_NOT_DOCUMENT

template<ProgramInterface Program, class Uniform, class Vertex, class Varying, FrameBufferInterface FrameBuffer>
struct can_use_program_with
{
    static constexpr bool value = std::is_same_v<typename Program::Uniform, Uniform> &&
                                  std::is_same_v<typename Program::Vertex, Vertex> &&
                                  std::is_same_v<typename Program::Varying, Varying> &&
                                  std::is_same_v<typename Program::Targets, typename FrameBuffer::Targets>;
};

/// @endcond

}; // namespace detail

} // namespace asciirast
