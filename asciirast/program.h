#pragma once

#include <concepts>

#include "./framebuffer.h"
#include "./math.h"

namespace asciirast {

template<typename T>
concept VaryingType = requires(T x) {
    { x + x } -> std::same_as<T>;
    { math::F{ -1 } * x } -> std::same_as<T>;
};

template<VaryingType Varying>
static Varying
lerp(const Varying& a, const Varying& b, const math::F t)
{
    return (1 - t) * a + t * b;
}

template<VaryingType Varying>
struct GenericFragment
{
    math::Vec4 pos;
    Varying attrs;
};

template<class T>
concept FragmentType = requires(T t) { []<VaryingType Varying>(GenericFragment<Varying>&) {}(t); };

template<FragmentType Fragment>
static Fragment
lerp(const Fragment& a, const Fragment& b, const math::F t)
{
    return Fragment{ .pos = math::lerp(a.pos, b.pos, t), .attrs = lerp(a.attrs, b.attrs, t) };
}

template<class Uniforms, class Vertex, VaryingType Varying, FrameBufferType FrameBuffer>
class Program
{
public:
    using Fragment = GenericFragment<Varying>;
    using Targets = FrameBuffer::Targets;

    virtual ~Program() = default;
    virtual Fragment on_vertex(const Uniforms&, const Vertex&) const = 0;
    virtual Targets on_fragment(const Uniforms&, const Fragment&) const = 0;
};

template<class T>
concept ProgramType =
        requires(T t) { []<class T1, class T2, VaryingType T3, FrameBufferType T4>(Program<T1, T2, T3, T4>&) {}(t); };

}
