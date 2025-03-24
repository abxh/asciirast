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
    virtual void plot(const math::Vec2Int&, const math::F, const Targets&) = 0;
};

template<class T>
concept FrameBufferType = requires(T t) { []<typename... Targets>(const FrameBuffer<Targets...>&) {}(t); };

} // namespace asciirast
