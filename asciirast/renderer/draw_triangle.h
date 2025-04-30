/**
 * @file triangle.h
 * @brief Render triangle
 */

#pragma once

#include "../renderer.h"

namespace asciirast {

template<ProgramInterface Program,
         class Uniform,
         class Vertex,
         FrameBufferInterface FrameBuffer,
         class Vec4TripletAllocator,
         class AttrsTripletAllocator>
    requires(std::is_same_v<Vertex, typename Program::Vertex>)
void
Renderer::draw_triangle(const Program& program,
                        const Uniform& uniform,
                        const bool requires_screen_clipping,
                        const math::Transform2D& scale_to_viewport,
                        const math::Transform2D& screen_to_window,
                        const RendererOptions& options,
                        RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data,
                        FrameBuffer& framebuffer,
                        const Vertex& v0,
                        const Vertex& v1,
                        const Vertex& v2)
{
    using Varying = typename Program::Varying;
    using Targets = typename Program::Targets;
    using FragmentContext = typename Program::FragmentContext;

    using Frag = Fragment<Varying>;
    using PFrag = ProjectedFragment<Varying>;

    const auto plot_func = [&program, &framebuffer, &uniform](const std::array<ProjectedFragment<Varying>, 4>& rfrag,
                                                              const std::array<bool, 4>& in_triangle) -> void {
        const auto [rfrag0, rfrag1, rfrag2, rfrag3] = rfrag;

        std::array<typename FragmentContext::ValueVariant, 4> quad;
        FragmentContext c0{ 0, quad, !in_triangle[0] };
        FragmentContext c1{ 1, quad, !in_triangle[1] };
        FragmentContext c2{ 2, quad, !in_triangle[2] };
        FragmentContext c3{ 3, quad, !in_triangle[3] };

        Targets targets0 = {};
        Targets targets1 = {};
        Targets targets2 = {};
        Targets targets3 = {};

        bool discarded0 = false;
        bool discarded1 = false;
        bool discarded2 = false;
        bool discarded3 = false;

        // apply fragment shader and unpack wrapped result:
        for (const auto [r0, r1, r2, r3] :
             std::ranges::views::zip(program.on_fragment(c0, uniform, rfrag0, targets0),
                                     program.on_fragment(c1, uniform, rfrag1, targets1),
                                     program.on_fragment(c2, uniform, rfrag2, targets2),
                                     program.on_fragment(c3, uniform, rfrag3, targets3))) {
            if (r0 == ProgramToken::Syncronize || r1 == ProgramToken::Syncronize || r2 == ProgramToken::Syncronize ||
                r3 == ProgramToken::Syncronize) {
                if (r0 != ProgramToken::Syncronize || r1 != ProgramToken::Syncronize ||
                    r2 != ProgramToken::Syncronize || r3 != ProgramToken::Syncronize) {
                    throw std::logic_error("asciirast::Renderer::draw() : Fragment shader must should"
                                           "syncronize in the same order in all instances");
                }
                c0.m_type = FragmentContext::Type::FILLED;
                c1.m_type = FragmentContext::Type::FILLED;
                c2.m_type = FragmentContext::Type::FILLED;
                c3.m_type = FragmentContext::Type::FILLED;
            }
            discarded0 |= r0 == ProgramToken::Discard;
            discarded1 |= r1 == ProgramToken::Discard;
            discarded2 |= r2 == ProgramToken::Discard;
            discarded3 |= r3 == ProgramToken::Discard;

            if (discarded0 || discarded1 || discarded2 || discarded3) {
                break;
            }
        }
        const auto rfrag0_pos_int = math::Vec2Int{ rfrag0.pos };
        const auto rfrag1_pos_int = math::Vec2Int{ rfrag1.pos };
        const auto rfrag2_pos_int = math::Vec2Int{ rfrag2.pos };
        const auto rfrag3_pos_int = math::Vec2Int{ rfrag3.pos };

        if (in_triangle[0] && !discarded0 && framebuffer.test_and_set_depth(rfrag0_pos_int, rfrag0.depth)) {
            framebuffer.plot(rfrag0_pos_int, targets0);
        }
        if (in_triangle[1] && !discarded1 && framebuffer.test_and_set_depth(rfrag1_pos_int, rfrag1.depth)) {
            framebuffer.plot(rfrag1_pos_int, targets1);
        }
        if (in_triangle[2] && !discarded2 && framebuffer.test_and_set_depth(rfrag2_pos_int, rfrag2.depth)) {
            framebuffer.plot(rfrag2_pos_int, targets2);
        }
        if (in_triangle[3] && !discarded3 && framebuffer.test_and_set_depth(rfrag3_pos_int, rfrag3.depth)) {
            framebuffer.plot(rfrag3_pos_int, targets3);
        }
    };
    const auto rasterize_triangle = [&plot_func,
                                     &options](const PFrag& wfrag0, const PFrag& wfrag1, const PFrag& wfrag2) -> void {
        const bool clockwise_winding_order = options.winding_order == WindingOrder::Clockwise;
        const bool cclockwise_winding_order = options.winding_order == WindingOrder::CounterClockwise;
        const bool neither_winding_order = options.winding_order == WindingOrder::Neither;

        // perform backface culling:
        const auto p0p2 = wfrag0.pos.vector_to(wfrag2.pos);
        const auto p0p1 = wfrag0.pos.vector_to(wfrag1.pos);
        const auto signed_area_2 = math::cross(p0p2, p0p1);
        const bool backface_cull_cond = !neither_winding_order && ((clockwise_winding_order && 0 < signed_area_2) || //
                                                                   (cclockwise_winding_order && 0 > signed_area_2));
        if (backface_cull_cond) {
            return;
        }

        // swap vertices after flexible winding order, and iterate over triangle fragments:
        if (clockwise_winding_order || (neither_winding_order && 0 > signed_area_2)) {
            renderer::rasterize_triangle(
                    wfrag0, wfrag1, wfrag2, std::forward<decltype(plot_func)>(plot_func), options.triangle_fill_bias);
        } else {
            renderer::rasterize_triangle(
                    wfrag0, wfrag2, wfrag1, std::forward<decltype(plot_func)>(plot_func), options.triangle_fill_bias);
        }
    };

    // apply vertex shader
    // model space -> world space -> view space -> clip space:
    Frag frag0 = { .pos = { 0, 0, 0, 1 } };
    Frag frag1 = { .pos = { 0, 0, 0, 1 } };
    Frag frag2 = { .pos = { 0, 0, 0, 1 } };

    program.on_vertex(uniform, v0, frag0);
    program.on_vertex(uniform, v1, frag1);
    program.on_vertex(uniform, v2, frag2);

    data.m_vec_queue0.clear();
    data.m_attrs_queue0.clear();
    data.m_vec_queue0.insert(                 //
            data.m_vec_queue0.end(),          //
            renderer::Vec4Triplet{ frag0.pos, //
                                   frag1.pos,
                                   frag2.pos });
    data.m_attrs_queue0.insert(                           //
            data.m_attrs_queue0.end(),                    //
            renderer::AttrsTriplet<Varying>{ frag0.attrs, //
                                             frag1.attrs,
                                             frag2.attrs });

    // clip triangle so it's inside the viewing volume:
    if (!renderer::triangle_in_frustum(data.m_vec_queue0, data.m_attrs_queue0)) {
        return;
    }
    for (const auto& [vec_triplet, attrs_triplet] : std::ranges::views::zip(data.m_vec_queue0, data.m_attrs_queue0)) {

        const auto [vec0, vec1, vec2] = vec_triplet;
        const auto [attrs0, attrs1, attrs2] = attrs_triplet;

        const Frag tfrag0 = { .pos = vec0, .attrs = attrs0 };
        const Frag tfrag1 = { .pos = vec1, .attrs = attrs1 };
        const Frag tfrag2 = { .pos = vec2, .attrs = attrs2 };

        // perspective divide
        // clip space -> screen space:
        const PFrag pfrag0 = project_fragment(tfrag0);
        const PFrag pfrag1 = project_fragment(tfrag1);
        const PFrag pfrag2 = project_fragment(tfrag2);

        // scale to viewport:
        const PFrag vfrag0 = apply_scale_to_viewport(scale_to_viewport, pfrag0);
        const PFrag vfrag1 = apply_scale_to_viewport(scale_to_viewport, pfrag1);
        const PFrag vfrag2 = apply_scale_to_viewport(scale_to_viewport, pfrag2);

        if (!requires_screen_clipping) {
            // screen space -> window space:
            const PFrag wfrag0 = apply_screen_to_window(screen_to_window, vfrag0);
            const PFrag wfrag1 = apply_screen_to_window(screen_to_window, vfrag1);
            const PFrag wfrag2 = apply_screen_to_window(screen_to_window, vfrag2);

            // iterate over triangle fragments:
            rasterize_triangle(wfrag0, wfrag1, wfrag2);

            continue;
        }

        data.m_vec_queue1.clear();
        data.m_attrs_queue1.clear();

        const auto p0 = math::Vec4{ vfrag0.pos, vfrag0.depth, vfrag0.Z_inv };
        const auto p1 = math::Vec4{ vfrag1.pos, vfrag1.depth, vfrag1.Z_inv };
        const auto p2 = math::Vec4{ vfrag2.pos, vfrag2.depth, vfrag2.Z_inv };
        const auto [a0, a1, a2] = //
                renderer::AttrsTriplet<Varying>{ vfrag0.attrs, vfrag1.attrs, vfrag2.attrs };
        const auto lp = renderer::Vec4Triplet{ p0, p1, p2 };
        const auto la = renderer::AttrsTriplet<Varying>{ a0, a1, a2 };

        data.m_vec_queue1.insert(data.m_vec_queue1.end(), lp);
        data.m_attrs_queue1.insert(data.m_attrs_queue1.end(), la);

        // clip line so it's inside the screen:
        if (!renderer::triangle_in_screen(data.m_vec_queue1, data.m_attrs_queue1, Renderer::SCREEN_BOUNDS)) {
            continue;
        }
        for (const auto& [inner_vec_triplet, inner_attrs_triplet] :
             std::ranges::views::zip(data.m_vec_queue1, data.m_attrs_queue1)) {

            const auto [inner_vec0, inner_vec1, inner_vec2] = inner_vec_triplet;
            const auto [inner_attrs0, inner_attrs1, inner_attrs2] = inner_attrs_triplet;

            const PFrag inner_tfrag0 = { inner_vec0.xy, inner_vec0.z, inner_vec0.w, inner_attrs0 };
            const PFrag inner_tfrag1 = { inner_vec1.xy, inner_vec1.z, inner_vec1.w, inner_attrs1 };
            const PFrag inner_tfrag2 = { inner_vec2.xy, inner_vec2.z, inner_vec2.w, inner_attrs2 };

            // screen space -> window space:
            const PFrag wfrag0 = apply_screen_to_window(screen_to_window, inner_tfrag0);
            const PFrag wfrag1 = apply_screen_to_window(screen_to_window, inner_tfrag1);
            const PFrag wfrag2 = apply_screen_to_window(screen_to_window, inner_tfrag2);

            // iterate over triangle fragments:
            rasterize_triangle(wfrag0, wfrag1, wfrag2);
        }
    }
}

}
