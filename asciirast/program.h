#pragma once

#include <concepts>

#include "./framebuffer.h"
#include "./math.h"

namespace asciirast {

template<typename T>
concept VaryingType = requires(T x) {
    { x + x } -> std::same_as<T>;
    { math::FloatingPointType{ -1 } * x } -> std::same_as<T>;
    requires std::is_same_v<decltype(x.pos), math::Vec4>;
};

template<VaryingType V>
static V
lerp(const V& a, const V& b, const math::FloatingPointType t)
{
    const auto weight_a = 1 - t;
    const auto weight_b = t;

    return weight_a * a + weight_b * b;
}

template<class Uniforms, class Vertex, VaryingType Varying, FrameBufferType FrameBuffer>
class Program
{
public:
    using Targets = FrameBuffer::Targets;

    virtual ~Program() = default;
    virtual Varying on_vertex(const Uniforms&, const Vertex&) const = 0;
    virtual Targets on_fragment(const Uniforms&, const Varying&) const = 0;
};

template<class T>
concept ProgramType = requires(T t) {
    []<class Uniforms, class Vertex, VaryingType Varying, FrameBufferType FrameBuffer>(
            Program<Uniforms, Vertex, Varying, FrameBuffer>&) {}(t);
};

}
