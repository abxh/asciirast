#pragma once

#include <tuple>

#include "math/types.h"

namespace asciirast {

template<typename... Targets>
class Framebuffer;

template<class FB>
concept FramebufferType = requires(FB fb) { []<typename... Targets>(Framebuffer<Targets...>&) {}(fb); };

template<typename... Targets>
class Framebuffer
{
public:
    using targets = std::tuple<Targets...>;

    ~Framebuffer() = default;
    virtual math::Transform2& get_viewport_to_window_transform() = 0;
    virtual void plot(const math::Vec2Int& pos, std::tuple<Targets...> targets) = 0;
};

}
