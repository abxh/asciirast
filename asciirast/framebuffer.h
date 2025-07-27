/**
 * @file framebuffer.h
 * @brief Definition of framebuffer interface
 */

#pragma once

#include "./math/types.h"

namespace asciirast {

/// @cond DO_NOT_DOCUMENT
namespace detail {

template<class T>
concept FrameBuffer_PlotSupport = requires(T t) {
    typename T::Targets;
    {
        t.plot(std::declval<const math::Vec2Int&>(),       //
               std::declval<const typename T::Targets&>()) //
    } -> std::same_as<void>;
};

template<class T>
concept FrameBuffer_DepthSupport = requires(T t) {
    {
        t.test_and_set_depth(std::declval<const math::Vec2Int&>(), //
                             std::declval<const math::Float>())    //
    } -> std::same_as<bool>;
};

};
/// @endcond

/**
 * @brief Concept for following the framebuffer interface
 *
 * @tparam T The type to check
 */
template<class T>
concept FrameBufferInterface = requires(T t) {
    requires detail::FrameBuffer_PlotSupport<T>;
} || requires(T t) {
    requires detail::FrameBuffer_PlotSupport<T>;
    requires detail::FrameBuffer_DepthSupport<T>;
};

} // namespace asciirast
