/**
 * @file point.h
 * @brief draw point routine
 */

#pragma once

#include "../renderer.h"

namespace asciirast {

template<ProgramInterface Program, class Uniform, class Vertex, FrameBufferInterface FrameBuffer>
    requires(std::is_same_v<Vertex, typename Program::Vertex>)
void
Renderer::draw_point(const Program& program,
                     const Uniform& uniform,
                     const bool requires_screen_clipping,
                     const math::Transform2D& scale_to_viewport,
                     const math::Transform2D& screen_to_window,
                     FrameBuffer& framebuffer,
                     const Vertex& vert)
{
    using Varying = typename Program::Varying;
    using Targets = typename Program::Targets;
    using FragmentContext = typename Program::FragmentContext;

    using Frag = Fragment<Varying>;
    using PFrag = ProjectedFragment<Varying>;

    // apply vertex shader
    // model space -> world space -> view space -> clip space:
    auto frag = Frag{ .pos = { 0, 0, 0, 1 } };
    program.on_vertex(uniform, vert, frag);

    // cull points outside of viewing volume:
    if (!renderer::point_in_frustum(frag.pos)) {
        return;
    }

    // perspective divide
    // clip space -> screen space:
    const PFrag pfrag = project_fragment(frag);

    // scale up to viewport:
    const PFrag vfrag = apply_scale_to_viewport(scale_to_viewport, pfrag);

    // cull points outside of screen:
    if (requires_screen_clipping && !renderer::point_in_screen(vfrag.pos, SCREEN_BOUNDS)) {
        return;
    }

    // screen space -> window space:
    const PFrag wfrag = apply_screen_to_window(screen_to_window, vfrag);

    // prepare values:
    std::array<typename FragmentContext::ValueVariant, 4> quad;

    auto context = FragmentContext{ 0, quad };
    auto targets = Targets{};

    // apply fragment shader and unpack results:
    for (const ProgramToken result : program.on_fragment(context, uniform, wfrag, targets)) {
        if (result == ProgramToken::Syncronize) {
            context.m_type = FragmentContext::Type::POINT;
        } else if (result == ProgramToken::Discard) {
            return;
        }
    }

    // plot if point is not discarded:
    const auto pos_int = math::Vec2Int{ wfrag.pos };

    if (framebuffer.test_and_set_depth(pos_int, wfrag.depth)) {
        framebuffer.plot(pos_int, targets);
    }
};

}
