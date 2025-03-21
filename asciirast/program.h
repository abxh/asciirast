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
struct Fragment
{
    math::Vec4 pos;
    Varying attrs;
};

template<VaryingType Varying>
struct ProjectedFragment
{
    math::Vec2 pos;
    math::F depth;
    Varying attrs;
};

template<class Uniforms, class Vertex, VaryingType Varying, FrameBufferType FrameBuffer>
class Program
{
public:
    using Targets = FrameBuffer::Targets;

    virtual ~Program() = default;
    virtual Fragment<Varying> on_vertex(const Uniforms&, const Vertex&) const = 0;
    virtual Targets on_fragment(const Uniforms&, const ProjectedFragment<Varying>&) const = 0;
};

template<class T>
concept ProgramType = requires(T t) {
    []<class T1, class T2, VaryingType T3, FrameBufferType T4>(const Program<T1, T2, T3, T4>&) {}(t);
};

template<VaryingType T>
static ProjectedFragment<T>
project(const Fragment<T>& frag)
{
    return ProjectedFragment<T>{ .pos = frag.pos.xy / frag.pos.w,
                                 .depth = frag.pos.w / frag.pos.z, // 1 / (z / w)
                                 .attrs = frag.attrs };
}

template<VaryingType Varying>
static Varying
lerp(const Varying& a, const Varying& b, const math::F t)
{
    return (1 - t) * a + t * b;
}

template<VaryingType T>
static Fragment<T>
lerp(const Fragment<T>& a, const Fragment<T>& b, const math::F t)
{
    return Fragment<T>{ .pos = math::lerp(a.pos, b.pos, t), .attrs = lerp(a.attrs, b.attrs, t) };
}

}
