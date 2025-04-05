/**
 * @file program.h
 * @brief Definition of program interface and other related types
 */

// TODO:
// improve documentation

#pragma once

#include <cassert>
#include <cstddef>
#include <ranges>
#include <vector>

#include "./math/types.h"
#include "./program.h"
#include "./rasterize/bounds_test.h"
#include "./rasterize/lerp.h"
#include "./rasterize/rasterize.h"

namespace asciirast {

enum class ShapeType
{
    POINTS,
    LINES,
    LINE_STRIP,
    LINE_LOOP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN
};

template<typename Vertex, class Allocator = std::allocator<Vertex>>
struct VertexBuffer
{
    ShapeType shape_type;
    std::vector<Vertex, Allocator> verticies;
};

template<typename Vertex, class Allocator = std::allocator<Vertex>>
struct IndexedVertexBuffer : VertexBuffer<Vertex>
{
    std::vector<std::size_t, Allocator> indicies;
};

class Renderer
{
    bool m_requires_viewport_clipping;
    math::Transform2D m_screen_to_viewport; ///@< screen -> viewport
    math::Transform2D m_viewport_to_window; ///@< viewport -> window
    math::Transform2D m_screen_to_window;   ///@< screen -> viewport -> window

    static inline math::Transform2D screen_to_viewport_transform(const math::AABB2D& viewport_bounds,
                                                                 const math::AABB2D& screen_bounds)
    {
        assert(viewport_bounds.size_get().x != math::F{ 0 });
        assert(viewport_bounds.size_get().y != math::F{ 0 });

        const auto rel_size = viewport_bounds.size_get() / screen_bounds.size_get();
        const auto rel_min  = screen_bounds.min_get().vector_to(viewport_bounds.min_get()) / screen_bounds.size_get();

        return SCREEN_BOUNDS.to_transform().reversed().scale(rel_size).translate(rel_min);
    }

public:
    static inline const auto SCREEN_BOUNDS = math::AABB2D::from_min_max(math::Vec2{ -1, -1 }, math::Vec2{ +1, +1 });

    Renderer()
            : m_requires_viewport_clipping{ false }
            , m_screen_to_viewport{ SCREEN_BOUNDS.to_transform().reversed() }
            , m_viewport_to_window{}
            , m_screen_to_window{ m_screen_to_viewport } {};

    Renderer(const math::AABB2D& viewport_bounds)
            : m_requires_viewport_clipping{ !SCREEN_BOUNDS.contains(viewport_bounds) }
            , m_screen_to_viewport{ screen_to_viewport_transform(viewport_bounds, SCREEN_BOUNDS) }
            , m_viewport_to_window{}
            , m_screen_to_window{ m_screen_to_viewport } {};

    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             class VertexAllocator,
             FrameBufferInterface FrameBuffer>
        requires(detail::can_use_program_with<Program, Uniform, Vertex, FrameBuffer>::value)
    void draw(const Program& program,
              const Uniform& uniform,
              const VertexBuffer<Vertex, VertexAllocator>& verts,
              FrameBuffer& out)
    {
        draw(program, uniform, verts.shape_type, std::views::all(verts.verticies), out);
    }

    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             class VertexAllocator,
             FrameBufferInterface FrameBuffer>
        requires(detail::can_use_program_with<Program, Uniform, Vertex, FrameBuffer>::value)
    void draw(const Program& program,
              const Uniform& uniform,
              const IndexedVertexBuffer<Vertex, VertexAllocator>& verts,
              FrameBuffer& out)
    {
        const auto func = [&verts](const std::size_t i) -> Vertex {
            assert(i < verts.verticies.size() && "index is inside bounds");

            return verts.verticies[i];
        };
        const auto view = std::ranges::views::transform(std::views::all(verts.indicies), func);

        draw(program, uniform, verts.shape_type, view, out);
    }

private:
    template<ProgramInterface Program, class Uniform, FrameBufferInterface FrameBuffer>
    void draw(const Program& program,
              const Uniform& uniform,
              const ShapeType shape_type,
              std::ranges::input_range auto&& range,
              FrameBuffer& framebuffer)
    {
        using Vertex  = typename Program::Vertex;
        using PFrag   = ProjectedFragment<typename Program::Varying>;

        if (const math::Transform2D viewport_to_window = framebuffer.viewport_to_window();
            !std::ranges::equal(m_viewport_to_window.mat().range(), viewport_to_window.mat().range())) {
            m_viewport_to_window = std::move(viewport_to_window);
            m_screen_to_window   = math::Transform2D().stack(m_screen_to_viewport).stack(m_viewport_to_window);
        }

        const auto round_pos = [](const math::Vec2& pos) -> math::Vec2 {
            return math::floor(pos + math::Vec2{ 0.5f, 0.5f });
        };
        const auto screen_to_viewport_func = [this](const PFrag& pfrag) -> PFrag {
            return PFrag{ .pos   = m_screen_to_viewport.apply(pfrag.pos),
                          .z_inv = pfrag.z_inv,
                          .w_inv = pfrag.w_inv,
                          .attrs = pfrag.attrs };
        };
        const auto viewport_to_window_func = [this, round_pos](const PFrag& vfrag) -> PFrag {
            return PFrag{ .pos   = round_pos(m_viewport_to_window.apply(vfrag.pos)),
                          .z_inv = vfrag.z_inv,
                          .w_inv = vfrag.w_inv,
                          .attrs = vfrag.attrs };
        };
        const auto screen_to_window_func = [this, round_pos](const PFrag& vfrag) -> PFrag {
            return PFrag{ .pos   = round_pos(m_screen_to_window.apply(vfrag.pos)),
                          .z_inv = vfrag.z_inv,
                          .w_inv = vfrag.w_inv,
                          .attrs = vfrag.attrs };
        };

        switch (shape_type) {
        case ShapeType::POINTS:
            for (const Vertex& vert : range) {
                // apply vertex shader
                // model space -> world space -> view space -> clip space:
                const auto frag = program.on_vertex(uniform, vert);

                // cull points outside of viewing volume:
                if (!rasterize::point_in_frustum(frag.pos)) {
                    continue;
                }

                // perspective divide
                // clip space -> screen space:
                const auto pfrag = rasterize::project(frag);

                // screen space -> window space:
                PFrag wfrag;
                if (m_requires_viewport_clipping) {
                    // scale up to viewport:
                    const auto vfrag = screen_to_viewport_func(pfrag);

                    // cull points outside of viewport:
                    if (!rasterize::point_in_unit_square(vfrag.pos)) {
                        continue;
                    }

                    wfrag = viewport_to_window_func(vfrag);
                } else {
                    wfrag = screen_to_window_func(pfrag);
                }

                // apply fragment shader:
                const auto targets = program.on_fragment(uniform, wfrag);

                // plot in framebuffer:
                framebuffer.plot(math::Vec2Int{ wfrag.pos }, wfrag.z_inv, targets);
            }
            break;
        case ShapeType::LINES:
        case ShapeType::LINE_STRIP:
        case ShapeType::LINE_LOOP: {
            const auto draw_lines = [&](std::ranges::input_range auto&& verticies, const bool looped = false) -> void {
                const auto draw_line = [&](const Vertex& v0, const Vertex& v1) -> void {
                    // apply vertex shader
                    // model space -> world space -> view space -> clip space:
                    const auto frag0 = program.on_vertex(uniform, v0);
                    const auto frag1 = program.on_vertex(uniform, v1);

                    // clip line so it's inside the viewing volume:
                    const auto tup = rasterize::line_in_frustum(frag0.pos, frag1.pos);
                    if (!tup.has_value()) {
                        return;
                    }
                    const auto [t0, t1] = tup.value(); // interpolate line using t values
                    const auto tfrag0   = rasterize::lerp(frag0, frag1, t0);
                    const auto tfrag1   = rasterize::lerp(frag0, frag1, t1);

                    // perspective divide
                    // clip space -> screen space:
                    const auto pfrag0 = rasterize::project(tfrag0);
                    const auto pfrag1 = rasterize::project(tfrag1);

                    // screen space -> window space:
                    PFrag wfrag0;
                    PFrag wfrag1;
                    if (m_requires_viewport_clipping) {
                        // scale up to viewport:
                        const auto vfrag0 = screen_to_viewport_func(pfrag0);
                        const auto vfrag1 = screen_to_viewport_func(pfrag1);

                        // clip line so it's inside the viewport:
                        const auto vtup = rasterize::line_in_unit_square(vfrag0.pos, vfrag1.pos);
                        if (!vtup.has_value()) {
                            return;
                        }
                        const auto [vt0, vt1] = vtup.value();
                        const auto vtfrag0    = rasterize::lerp(vfrag0, vfrag1, vt0);
                        const auto vtfrag1    = rasterize::lerp(vfrag0, vfrag1, vt1);

                        wfrag0 = viewport_to_window_func(vtfrag0);
                        wfrag1 = viewport_to_window_func(vtfrag1);
                    } else {
                        wfrag0 = screen_to_window_func(pfrag0);
                        wfrag1 = screen_to_window_func(pfrag1);
                    }

                    // iterate over line fragments:
                    for (const auto& [pos, z_inv, w_inv, attrs] : rasterize::rasterize_line(wfrag0, wfrag1)) {

                        // apply fragment shader:
                        const auto targets = program.on_fragment(uniform, PFrag{ pos, z_inv, w_inv, attrs });

                        // plot point in framebuffer:
                        framebuffer.plot(math::Vec2Int{ pos }, z_inv, targets);
                    }
                };
                for (const auto [v0, v1] : verticies) {
                    draw_line(v0, v1);
                }
                if (looped && std::ranges::distance(range) >= 1U) {
                    draw_line(std::get<1>(*(verticies.cend() - 1U)), std::get<0>(*verticies.cbegin()));
                };
            };
            if (shape_type == ShapeType::LINES) {
                const auto func = [](auto&& r) -> std::tuple<const Vertex, const Vertex> {
                    return std::make_tuple(*(r.cbegin() + 0U), *(r.cbegin() + 1U));
                };
                const auto rem      = std::ranges::distance(range) % 2U;
                const auto subrange = range | std::ranges::views::take(std::ranges::distance(range) - rem);

                draw_lines(subrange | std::ranges::views::chunk(2U) | std::ranges::views::transform(func));
            } else if (shape_type == ShapeType::LINE_STRIP) {
                draw_lines(range | std::ranges::views::adjacent<2U>);
            } else if (shape_type == ShapeType::LINE_LOOP) {
                const bool looped = true;
                draw_lines(range | std::ranges::views::adjacent<2U>, looped);
            }
        } break;
        case ShapeType::TRIANGLES:
        case ShapeType::TRIANGLE_STRIP:
        case ShapeType::TRIANGLE_FAN: {
            const auto draw_triangles = [&](std::ranges::input_range auto&& verticies, const bool fan = false) -> void {
                const auto draw_triangle = [&](const Vertex& v0, const Vertex& v1, const Vertex& v2) -> void {
                    // apply vertex shader
                    // model space -> world space -> view space -> clip space:
                    const auto frag0 = program.on_vertex(uniform, v0);
                    const auto frag1 = program.on_vertex(uniform, v1);
                    const auto frag2 = program.on_vertex(uniform, v2);

                    // TODO: clipping

                    // perspective divide
                    // clip space -> screen space:
                    const auto pfrag0 = rasterize::project(frag0);
                    const auto pfrag1 = rasterize::project(frag1);
                    const auto pfrag2 = rasterize::project(frag2);

                    // screen space -> window space:
                    const PFrag wfrag0 = screen_to_window_func(pfrag0);
                    const PFrag wfrag1 = screen_to_window_func(pfrag1);
                    const PFrag wfrag2 = screen_to_window_func(pfrag2);

                    // iterate over triangle fragments:
                    for (const auto& [pos, z_inv, w_inv, attrs] :
                         rasterize::rasterize_triangle<decltype(wfrag0.attrs), true>(wfrag0, wfrag1, wfrag2)) {

                        // apply fragment shader:
                        const auto targets = program.on_fragment(uniform, PFrag{ pos, z_inv, w_inv, attrs });

                        // plot point in framebuffer:
                        framebuffer.plot(math::Vec2Int{ pos }, z_inv, targets);
                    }
                };
                for (const auto [v0, v1, v2] : verticies) {
                    draw_triangle(v0, v1, v2);
                }
                if (fan && std::ranges::distance(range) >= 1U) {
                    draw_triangle(std::get<1>(*(verticies.cend() - 1U)),
                                  std::get<2>(*(verticies.cend() - 1U)),
                                  std::get<0>(*verticies.cbegin()));
                };
            };

            if (shape_type == ShapeType::TRIANGLES) {
                const auto func = [](auto&& r) -> std::tuple<const Vertex, const Vertex, const Vertex> {
                    return std::make_tuple(*(r.cbegin() + 0U), *(r.cbegin() + 1U), *(r.cbegin() + 2U));
                };
                const auto rem      = std::ranges::distance(range) % 3U;
                const auto subrange = range | std::ranges::views::take(std::ranges::distance(range) - rem);

                draw_triangles(subrange | std::ranges::views::chunk(3U) | std::ranges::views::transform(func));
            } else if (shape_type == ShapeType::TRIANGLE_STRIP) {
                draw_triangles(range | std::ranges::views::adjacent<3U>);
            } else if (shape_type == ShapeType::TRIANGLE_FAN) {
                draw_triangles(range | std::ranges::views::adjacent<3U>, true);
            }
        } break;
        }
    }
};

};
