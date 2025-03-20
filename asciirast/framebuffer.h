#pragma once

#include <tuple>

#include "./math.h"
#include "./utils.h"

namespace asciirast {

template<typename... TargetTypes>
class FrameBuffer
{
public:
    using Transform2Wrapped = utils::ChangeDetected<math::Transform2>;
    using Transform2WrappedView = const utils::AbstractChangeDetected<math::Transform2>&;
    using Targets = std::tuple<TargetTypes...>;

    ~FrameBuffer() = default;
    virtual Transform2WrappedView get_viewport_to_window() const = 0;
    virtual void plot(const math::Vec2Int&, const math::F, const Targets&) = 0;
};

template<class T>
concept FrameBufferType = requires(T t) { []<typename... Targets>(const FrameBuffer<Targets...>&) {}(t); };

}
