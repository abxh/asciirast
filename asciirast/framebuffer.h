#pragma once

#include <tuple>

#include "./math.h"
#include "./utils/ChangeDetected.h"

namespace asciirast {

template<typename... TargetTypes>
class FrameBuffer
{
public:
    using Targets = std::tuple<TargetTypes...>;

    ~FrameBuffer() = default;
    virtual const AbstractChangeDetected<math::Transform2>& get_viewport_to_window() const = 0;
    virtual void plot(const math::Vec2&, const Targets&) = 0;
};

template<class T>
concept FrameBufferType = requires(T t) { []<typename... Targets>(FrameBuffer<Targets...>&) {}(t); };

}
