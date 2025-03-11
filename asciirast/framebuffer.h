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
    virtual const math::Transform2& get_viewport_to_window_transform() const = 0;
    virtual void plot(const math::Vec2&, const Targets&) = 0;
};

template<class T>
concept FrameBufferType = requires(T t) { []<typename... Targets>(FrameBuffer<Targets...>&) {}(t); };

}
