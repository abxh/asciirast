#pragma once

#include <tuple>

#include "./math.h"

namespace asciirast {

template<typename... TargetTypes>
class FrameBuffer
{
public:
    using Targets = std::tuple<TargetTypes...>;

    ~FrameBuffer() = default;

    virtual const math::Transform2D& viewport_to_window() const = 0;

    virtual void plot(const math::Vec2Int& pos, const math::F depth, const Targets& targets) = 0;
};

template<class T>
concept FrameBufferType = requires(T t) { []<typename... TargetTypes>(const FrameBuffer<TargetTypes...>&) {}(t); };

} // namespace asciirast
