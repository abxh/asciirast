/**
 * @file point.h
 * @brief Render points
 */

#pragma once

#include "../renderer.h"

namespace asciirast {

template<ProgramInterface Program, class Uniform, class Vertex, FrameBufferInterface FrameBuffer>
    requires(std::is_same_v<Vertex, typename Program::Vertex>)
void
Renderer::draw_line(const Program& program,
                    const Uniform& uniform,
                    const bool requires_screen_clipping,
                    const math::Transform2D& scale_to_viewport,
                    const math::Transform2D& screen_to_window,
                    const RendererOptions& options,
                    FrameBuffer& framebuffer,
                    const Vertex& v0,
                    const Vertex& v1)
{
    using Varying = typename Program::Varying;
    using Targets = typename Program::Targets;
    using FragmentContext = typename Program::FragmentContext;

    using Frag = Fragment<Varying>;
    using PFrag = ProjectedFragment<Varying>;

    // apply vertex shader
    // model space -> world space -> view space -> clip space:
    Frag frag0 = { .pos = { 0, 0, 0, 1 } };
    Frag frag1 = { .pos = { 0, 0, 0, 1 } };

    program.on_vertex(uniform, v0, frag0);
    program.on_vertex(uniform, v1, frag1);

    // clip line so it's inside the viewing volume:
    const auto tup = renderer::line_in_frustum(frag0.pos, frag1.pos);
    if (!tup.has_value()) {
        return;
    }
    const auto [t0, t1] = tup.value();
    const Frag tfrag0 = renderer::lerp(frag0, frag1, t0);
    const Frag tfrag1 = renderer::lerp(frag0, frag1, t1);

    // perspective divide
    // clip space -> screen space:
    const PFrag pfrag0 = project_fragment(tfrag0);
    const PFrag pfrag1 = project_fragment(tfrag1);

    // scale up to viewport:
    const PFrag vfrag0 = apply_scale_to_viewport(scale_to_viewport, pfrag0);
    const PFrag vfrag1 = apply_scale_to_viewport(scale_to_viewport, pfrag1);

    PFrag inner_tfrag0 = vfrag0;
    PFrag inner_tfrag1 = vfrag1;
    if (requires_screen_clipping) {
        // clip line so it's inside the screen:
        const auto inner_tup = renderer::line_in_screen(vfrag0.pos, vfrag1.pos, Renderer::SCREEN_BOUNDS);
        if (!inner_tup.has_value()) {
            return;
        }
        const auto [inner_t0, inner_t1] = inner_tup.value();
        inner_tfrag0 = renderer::lerp(vfrag0, vfrag1, inner_t0);
        inner_tfrag1 = renderer::lerp(vfrag0, vfrag1, inner_t1);
    }

    // screen space -> window space:
    const PFrag wfrag0 = apply_screen_to_window(screen_to_window, inner_tfrag0);
    const PFrag wfrag1 = apply_screen_to_window(screen_to_window, inner_tfrag1);

    // swap vertices after line drawing direction
    bool keep = true;
    switch (const auto v0v1 = wfrag0.pos.vector_to(wfrag1.pos); options.line_drawing_direction) {
    case LineDrawingDirection::Upwards:
        keep = v0v1.y > 0;
        break;
    case LineDrawingDirection::Downwards:
        keep = v0v1.y < 0;
        break;
    case LineDrawingDirection::Leftwards:
        keep = v0v1.x < 0;
        break;
    case LineDrawingDirection::Rightwards:
        keep = v0v1.x > 0;
        break;
    }

    const auto plot_func = [&program, &framebuffer, &uniform](const std::array<ProjectedFragment<Varying>, 2>& rfrag,
                                                              const std::array<bool, 2>& in_line) -> void {
        const auto [rfrag0, rfrag1] = rfrag;

        std::array<typename FragmentContext::ValueVariant, 4> quad;

        FragmentContext c0{ 0, quad, !in_line[0] };
        FragmentContext c1{ 1, quad, !in_line[1] };

        Targets targets0 = {};
        Targets targets1 = {};

        bool discarded0 = false;
        bool discarded1 = false;

        // apply fragment shader and unpack results:
        for (const auto [r0, r1] : std::ranges::views::zip(program.on_fragment(c0, uniform, rfrag0, targets0),
                                                           program.on_fragment(c1, uniform, rfrag1, targets1))) {
            if (r0 == ProgramToken::Syncronize || r1 == ProgramToken::Syncronize) {
                if (r0 != ProgramToken::Syncronize || r1 != ProgramToken::Syncronize) {
                    throw std::logic_error("asciirast::Renderer::draw() : Fragment shader must should"
                                           "syncronize in the same order in all instances");
                }
                c0.m_type = FragmentContext::Type::LINE;
                c1.m_type = FragmentContext::Type::LINE;
            }
            discarded0 |= r0 == ProgramToken::Discard;
            discarded1 |= r1 == ProgramToken::Discard;

            if (discarded0 || discarded1) {
                break;
            }
        }

        const auto rfrag0_pos_int = math::Vec2Int{ rfrag0.pos };
        const auto rfrag1_pos_int = math::Vec2Int{ rfrag1.pos };

        if (in_line[0] && !discarded0 && framebuffer.test_and_set_depth(rfrag0_pos_int, rfrag0.depth)) {
            framebuffer.plot(rfrag0_pos_int, targets0);
        }
        if (in_line[1] && !discarded1 && framebuffer.test_and_set_depth(rfrag1_pos_int, rfrag1.depth)) {
            framebuffer.plot(rfrag1_pos_int, targets1);
        }
    };

    if (keep) {
        renderer::rasterize_line(
                wfrag0, wfrag1, std::forward<decltype(plot_func)>(plot_func), options.line_ends_inclusion);
    } else {
        renderer::rasterize_line(
                wfrag1, wfrag0, std::forward<decltype(plot_func)>(plot_func), options.line_ends_inclusion);
    }
}

};
