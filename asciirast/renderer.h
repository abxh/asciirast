// TODO:
// improve documentation

#pragma once

#include <cassert>
#include <cstddef>
#include <ranges>
#include <vector>

#include "./math.h"
#include "./program.h"
#include "./rasterize/frustum_test.h"
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
    math::Transform2D m_screen_to_viewport = {};
    math::Transform2D m_viewport_to_window = {};
    math::Transform2D m_screen_to_window   = {};

public:
    static inline const auto SCREEN_BOUNDS   = math::AABB2D::from_min_max(math::Vec2{ -1, -1 }, math::Vec2{ +1, +1 });
    static inline const auto VIEWPORT_BOUNDS = math::AABB2D::from_min_max(math::Vec2{ +0, +0 }, math::Vec2{ +1, +1 });

    Renderer()
            : m_screen_to_viewport{ SCREEN_BOUNDS.to_transform().reversed() }
            , m_screen_to_window{ m_screen_to_viewport }
    {
    }

    Renderer(const math::AABB2D& viewport)
            : m_screen_to_viewport{ SCREEN_BOUNDS.to_transform().reversed().stack(viewport.to_transform()) }
            , m_screen_to_window{ m_screen_to_viewport }
    {
        assert(viewport.size_get() != math::Vec2{ 0 });
        assert(VIEWPORT_BOUNDS.contains(viewport));
    }

    template<class Uniforms, class Vertex, class VertexAllocator, VaryingType Varying, FrameBufferType FrameBuffer>
    void draw(const Program<Uniforms, Vertex, Varying, FrameBuffer>& program,
              const Uniforms& uniforms,
              const VertexBuffer<Vertex, VertexAllocator>& verts,
              FrameBuffer& out)
    {
        draw(program, uniforms, verts.shape_type, std::views::all(verts.verticies), out);
    }

    template<class Uniforms, class Vertex, class VertexAllocator, VaryingType Varying, FrameBufferType FrameBuffer>
    void draw(const Program<Uniforms, Vertex, Varying, FrameBuffer>& program,
              const Uniforms& uniforms,
              const IndexedVertexBuffer<Vertex, VertexAllocator>& verts,
              FrameBuffer& out)
    {
        const auto func = [&verts](const std::size_t i) -> Vertex {
            assert(i < verts.verticies.size() && "index is inside bounds");

            return verts.verticies[i];
        };
        const auto view = std::ranges::views::transform(std::views::all(verts.indicies), func);

        draw(program, uniforms, verts.shape_type, view, out);
    }

private:
    template<class Uniforms, class Vertex, VaryingType Varying, FrameBufferType FrameBuffer>
    void draw(const Program<Uniforms, Vertex, Varying, FrameBuffer>& program,
              const Uniforms& uniforms,
              const ShapeType shape_type,
              std::ranges::input_range auto&& range,
              FrameBuffer& framebuffer)
    {
        using Targets = typename FrameBuffer::Targets;
        using PFrag   = ProjectedFragment<Varying>;

        if (!std::ranges::equal(m_viewport_to_window.mat().range(), framebuffer.viewport_to_window().mat().range())) {
            m_viewport_to_window = std::move(framebuffer.viewport_to_window());
            m_screen_to_window = std::move(math::Transform2D().stack(m_screen_to_viewport).stack(m_viewport_to_window));
        }

        const auto screen_to_window_func = [this](const PFrag& wfrag) -> PFrag {
            return PFrag{ .pos   = math::floor(m_screen_to_window.apply(wfrag.pos) + math::Vec2{ 0.5f, 0.5f }),
                          .z_inv = wfrag.z_inv,
                          .w_inv = wfrag.w_inv,
                          .attrs = wfrag.attrs };
        };

        switch (shape_type) {
        case ShapeType::POINTS:
            for (const Vertex& vert : range) {
                // apply vertex shader
                // model space -> world space -> view space -> clip space:
                const auto frag = program.on_vertex(uniforms, vert);

                // cull points outside of viewing volume:
                if (rasterize::point_in_frustum(frag.pos)) {
                    continue;
                }

                // perspective divide
                // clip space -> screen space:
                const auto pfrag = math::project(frag);

                // screen space -> window space:
                const auto wfrag = screen_to_window_func(pfrag);

                // apply fragment shader:
                const auto targets = program.on_fragment(uniforms, wfrag);

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
                    const auto frag0 = program.on_vertex(uniforms, v0);
                    const auto frag1 = program.on_vertex(uniforms, v1);

                    // clip line so it's inside the viewing volume:
                    const auto tup = rasterize::line_in_frustum(frag0.pos, frag1.pos);
                    if (!tup.has_value()) {
                        return;
                    }
                    const auto [t0, t1] = tup.value(); // interpolate line using t values:
                    const auto tfrag0   = math::lerp(frag0, frag1, t0);
                    const auto tfrag1   = math::lerp(frag0, frag1, t1);

                    // perspective divide
                    // clip space -> screen space:
                    const auto pfrag0 = math::project(tfrag0);
                    const auto pfrag1 = math::project(tfrag1);

                    // screen space -> window space:
                    const auto wfrag0 = screen_to_window_func(pfrag0);
                    const auto wfrag1 = screen_to_window_func(pfrag1);

                    // iterate over line fragments:
                    for (const auto [pos, z_inv, w_inv, attrs] : rasterize::rasterize_line(wfrag0, wfrag1)) {

                        // apply fragment shader:
                        const auto targets = program.on_fragment(uniforms, PFrag{ pos, z_inv, w_inv, attrs });

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
                draw_lines(range | std::ranges::views::adjacent<2U>, true);
            }
        } break;
        case ShapeType::TRIANGLES:
        case ShapeType::TRIANGLE_STRIP:
        case ShapeType::TRIANGLE_FAN: {
            const auto draw_triangles = [&](std::ranges::input_range auto&& verticies, const bool fan = false) -> void {
                const auto draw_triangle = [&](const Vertex& v0, const Vertex& v1, const Vertex& v2) -> void {

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
