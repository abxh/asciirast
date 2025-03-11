#pragma once

#include <tuple>

#include "math/types.h"

namespace asciirast {

template<typename... Targets>
class Framebuffer
{
public:
    using targets = std::tuple<Targets...>;

    ~Framebuffer() = default;
    virtual const math::Transform2& get_viewport_to_window_transform() const = 0;
    virtual void plot(const math::Vec2Int& pos, std::tuple<Targets...> targets) = 0;
};

template<class T>
concept FramebufferType = requires(T t) { []<typename... Targets>(Framebuffer<Targets...>&) {}(t); };

}
