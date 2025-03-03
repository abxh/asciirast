// inspiration:
// https://github.com/nikolausrauch/software-rasterizer

#pragma once

#include <concepts>

#include "framebuffer.h"
#include "math/types.h"

namespace asciirast {

template<typename T>
concept VaryingType = requires(T x) {
    { x + x } -> std::same_as<T>;
    { -1.f * x } -> std::same_as<T>;
    requires std::is_same_v<decltype(x.pos), math::Vec4>;
};

template<class Uniforms, class Vertex, VaryingType Varying, FramebufferType Framebuffer>
class Program
{
public:
    using Targets = Framebuffer::targets;

    virtual ~Program() = default;
    virtual Varying on_vertex(const Uniforms& u, const Vertex& vert) const = 0;
    virtual Targets on_fragment(const Uniforms& u, const Varying& frag) const = 0;
};

}
