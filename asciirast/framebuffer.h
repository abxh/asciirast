/**
 * @file framebuffer.h
 * @brief Definition of framebuffer interface
 *
 * @todo improve documentation
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
    using Targets = std::tuple<TargetTypes...>; ///@< target types

    /**
     * @brief Default virtual destructor
     */
    ~AbstractFrameBuffer() = default;

    /**
     * @brief Get transform that converts points from the screen to
     *        the points in the window
     */
    virtual const math::Transform2D& screen_to_window() = 0;

    /**
     * @brief Test and set the depth of a pixel
     */
    virtual bool test_and_set_depth(const math::Vec2Int& pos, const math::Float depth) = 0;

    /**
     * @brief Plot a point in the framebuffer
     */
    virtual void plot(const math::Vec2Int& pos, const Targets& targets) = 0;
};

/**
 * @brief Concept of types that follow the framebuffer interface
 */
template<class T>
concept FrameBufferInterface = requires(T t) {
    typename T::Targets;
    { t.screen_to_window() } -> std::same_as<const math::Transform2D&>;
    {
        t.test_and_set_depth(std::declval<const math::Vec2Int&>(), std::declval<const math::Float>())
    } -> std::same_as<bool>;
    { t.plot(std::declval<const math::Vec2Int&>(), std::declval<const typename T::Targets&>()) } -> std::same_as<void>;
};

} // namespace asciirast
