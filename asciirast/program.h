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

template<VaryingType Varying, bool Projected>
struct GenericFragment;

template<VaryingType Varying>
struct GenericFragment<Varying, false>
{
    math::Vec4 pos;
    Varying attrs;
};

template<VaryingType Varying>
struct GenericFragment<Varying, true>
{
    math::Vec2 pos;
    math::F depth;
    Varying attrs;
};

template<class T>
concept FragmentType = requires(T t) { []<VaryingType Varying>(const GenericFragment<Varying, false>&) {}(t); };

template<class T>
concept ProjectedFragmentType = requires(T t) { []<VaryingType Varying>(const GenericFragment<Varying, true>&) {}(t); };

template<VaryingType T>
static GenericFragment<T, true>
project(const GenericFragment<T, false>& frag)
{
    return GenericFragment<T, true>{ .pos = frag.pos.xy / frag.pos.w,
                                     .depth = frag.pos.w / frag.pos.z,
                                     .attrs = frag.attrs };
}

template<VaryingType Varying>
static Varying
lerp(const Varying& a, const Varying& b, const math::F t)
{
    return (1 - t) * a + t * b;
}

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
    using Fragment = GenericFragment<Varying, false>;
    using ProjectedFragment = GenericFragment<Varying, true>;
    using Targets = FrameBuffer::Targets;

    virtual ~Program() = default;
    virtual Fragment on_vertex(const Uniforms&, const Vertex&) const = 0;
    virtual Targets on_fragment(const Uniforms&, const ProjectedFragment&) const = 0;
};

template<class T>
concept ProgramType = requires(T t) {
    []<class T1, class T2, VaryingType T3, FrameBufferType T4>(const Program<T1, T2, T3, T4>&) {}(t);
};

}
