/**
 * @file framebuffer.h
 * @brief Definition of framebuffer interface
 */

#pragma once

#include <tuple>

#include "./math/types.h"

namespace asciirast {

/**
 * @brief Abstract framebuffer that can be used to follow the
 *        framebuffer interface
 *
 * @tparam TargetTypes The types of the targets that the renderer will
 *                     output to the framebuffer
 */
template<typename... TargetTypes>
class AbstractFrameBuffer
{
public:
    using Targets = std::tuple<TargetTypes...>; ///< target types

    /**
     * @brief Default virtual destructor
     */
    ~AbstractFrameBuffer() = default;

    /**
     * @brief Test and set the depth of a pixel
     *
     * @param pos The position of the pixel
     * @param depth The proposed depth of the pixel
     * @return Whether the depth has been set
     */
    virtual bool test_and_set_depth([[maybe_unused]] const math::Vec2Int& pos, [[maybe_unused]] const math::Float depth)
    {
        return true;
    }

    /**
     * @brief Plot a pixel in the framebuffer
     *
     * @param pos The position of the pixel
     * @param targets The user-specified targets for the pixel
     */
    virtual void plot(const math::Vec2Int& pos, const Targets& targets) = 0;
};

/**
 * @brief Concept for following the framebuffer interface
 *
 * @tparam T The type to check
 */
template<class T>
concept FrameBufferInterface = requires(T t) {
    typename T::Targets;
    {
        t.test_and_set_depth(std::declval<const math::Vec2Int&>(), //
                             std::declval<const math::Float>())    //
    } -> std::same_as<bool>;
    {
        t.plot(std::declval<const math::Vec2Int&>(),       //
               std::declval<const typename T::Targets&>()) //
    } -> std::same_as<void>;
};

} // namespace asciirast
