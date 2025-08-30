/**
 * @file framebuffer.hpp
 * @brief Definition of framebuffer interface
 */

#pragma once

#include "./math/types.hpp"

namespace asciirast {

/**
 * @brief Check if framebuffer has screen_to_window_transform() support
 */
template<class T>
concept FrameBuffer_TransformSupport = requires(const T t) {
    {
        t.screen_to_window_transform()
    } -> std::same_as<const math::Transform2D&>;
};

/**
 * @brief Check if framebuffer has plot() support
 */
template<class T>
concept FrameBuffer_PlotSupport = requires(T t) {
    typename T::Targets;
    {
        t.plot(std::declval<const math::Vec2Int&>(),       //
               std::declval<const typename T::Targets&>()) //
    } -> std::same_as<void>;
};

/**
 * @brief Check if framebuffer has test_and_set_depth() support
 */
template<class T>
concept FrameBuffer_DepthSupport = requires(T t) {
    {
        t.test_and_set_depth(std::declval<const math::Vec2Int&>(), //
                             std::declval<const math::Float>())    //
    } -> std::same_as<bool>;
};

/**
 * @brief Concept to follow the framebuffer interface
 */
template<class T>
concept FrameBufferInterface = requires(T t) {
    requires FrameBuffer_TransformSupport<T>;
    requires FrameBuffer_PlotSupport<T>;
} || requires(T t) {
    requires FrameBuffer_TransformSupport<T>;
    requires FrameBuffer_PlotSupport<T>;
    requires FrameBuffer_DepthSupport<T>;
};

} // namespace asciirast
