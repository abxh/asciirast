// TODO:
// improve documentation

#pragma once

#include <cassert>
#include <cstddef>
#include <ranges>
#include <vector>

#include "./math.h"
#include "./program.h"
#include "./rasterize/clip_line.h"
#include "./rasterize/cull_point.h"
#include "./rasterize/generate_attrs.h"
#include "./rasterize/generate_depth.h"
#include "./rasterize/generate_line.h"

namespace asciirast {

enum class ShapeType
{
    POINTS,
    LINES,
    LINE_STRIP,
    LINE_LOOP,
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
public:
    static inline const math::AABB2D SCREEN_BOUNDS =
            math::AABB2D::from_min_max(math::Vec2{ -1, -1 }, math::Vec2{ +1, +1 });
    static inline const math::AABB2D VIEWPORT_BOUNDS =
            math::AABB2D::from_min_max(math::Vec2{ +0, +0 }, math::Vec2{ +1, +1 });

    Renderer()
            : m_screen_to_viewport{ SCREEN_BOUNDS.to_transform().reversed() }
            , m_viewport_to_window{}
            , m_screen_to_window{}
    {
    }

    Renderer(const math::AABB2D& viewport)
            : m_screen_to_viewport{ SCREEN_BOUNDS.to_transform().reversed().stack(viewport.to_transform()) }
            , m_viewport_to_window{}
            , m_screen_to_window{}
    {
        assert(viewport.size_get() != math::Vec2{ 0 });
        assert(VIEWPORT_BOUNDS.contains(viewport));
    }

    template<class Uniforms, class Vertex, VaryingType Varying, FrameBufferType FrameBuffer, class VertexAllocator>
    void draw(const Program<Uniforms, Vertex, Varying, FrameBuffer>& program,
              const Uniforms& uniforms,
              const VertexBuffer<Vertex, VertexAllocator>& verts,
              FrameBuffer& out)
    {
        draw(program, uniforms, verts.shape_type, std::views::all(verts.verticies), out);
    }

    template<class Uniforms, class Vertex, VaryingType Varying, FrameBufferType FrameBuffer, class VertexAllocator>
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
    math::Transform2D m_screen_to_viewport;
    math::Transform2D m_viewport_to_window;
    math::Transform2D m_screen_to_window;

    template<class Uniforms, class Vertex, VaryingType Varying, FrameBufferType FrameBuffer>
    void draw(const Program<Uniforms, Vertex, Varying, FrameBuffer>& program,
              const Uniforms& uniforms,
              const ShapeType shape_type,
              std::ranges::input_range auto&& range,
              FrameBuffer& framebuffer)
    {
        if (!std::ranges::equal(m_viewport_to_window.mat().range(), framebuffer.viewport_to_window().mat().range())) {
            m_viewport_to_window = framebuffer.viewport_to_window();
            m_screen_to_window = math::Transform2D().stack(m_screen_to_viewport).stack(m_viewport_to_window);
        }

        const auto screen_to_window_func = [this](const math::Vec2& pos) -> math::Vec2 {
            return math::floor(m_screen_to_window.apply(pos) + math::Vec2{ 0.5f, 0.5f });
        };

        using Targets = typename FrameBuffer::Targets;
        using PFrag = ProjectedFragment<Varying>;

        switch (shape_type) {
        case ShapeType::POINTS:
            for (const Vertex& vert : range) {
                // apply vertex shader
                // model space -> world space -> view space -> clip space:
                const auto frag = program.on_vertex(uniforms, vert);

                // cull points outside of viewing volume:
                if (rasterize::cull_point(frag.pos)) {
                    continue;
                }

                // perspective divide
                // clip space -> screen space:
                const auto pfrag = project(frag);

                // screen space -> window space:
                const auto wfrag =
                        PFrag{ .pos = screen_to_window_func(pfrag.pos), .depth = pfrag.depth, .attrs = pfrag.attrs };

                // apply fragment shader:
                const auto targets = program.on_fragment(uniforms, wfrag);

                // plot in framebuffer:
                framebuffer.plot(math::Vec2Int{ wfrag.pos }, wfrag.depth, targets);
            }
            break;
        case ShapeType::LINES:
        case ShapeType::LINE_STRIP:
        case ShapeType::LINE_LOOP:
            const auto draw_lines = [&](std::ranges::input_range auto&& verticies, const bool looped = false) -> void {
                const auto draw_line = [&](const Vertex& v0, const Vertex& v1) -> void {
                    // apply vertex shader
                    // model space -> world space -> view space -> clip space:
                    const auto frag0 = program.on_vertex(uniforms, v0);
                    const auto frag1 = program.on_vertex(uniforms, v1);

                    // clip line so it's inside the viewing volume:
                    const auto tup = rasterize::clip_line(frag0.pos, frag1.pos);
                    if (!tup.has_value()) {
                        return;
                    }
                    const auto [t0, t1] = tup.value(); // interpolate line using t values:
                    const auto tfrag0 = lerp(frag0, frag1, t0);
                    const auto tfrag1 = lerp(frag0, frag1, t1);

                    // perspective divide
                    // clip space -> screen space:
                    const auto pfrag0 = project(tfrag0);
                    const auto pfrag1 = project(tfrag1);

                    // screen space -> window space:
                    const auto wfrag0 = PFrag{ .pos = screen_to_window_func(pfrag0.pos),
                                               .depth = pfrag0.depth,
                                               .attrs = pfrag0.attrs };
                    const auto wfrag1 = PFrag{ .pos = screen_to_window_func(pfrag1.pos),
                                               .depth = pfrag1.depth,
                                               .attrs = pfrag1.attrs };

                    const auto delta = wfrag1.pos - wfrag0.pos;
                    const auto size = math::abs(delta);
                    const auto len = std::max<math::F>(size.x, size.y);
                    const auto len_inv = 1 / len; // division by zero let through.

                    // iterate over line fragments:
                    for (const auto [pos, depth, attrs] :
                         std::ranges::views::zip(rasterize::generate_line(len, len_inv, wfrag0.pos, wfrag1.pos),
                                                 rasterize::generate_depth(len, len_inv, wfrag0.depth, wfrag1.depth),
                                                 rasterize::generate_attrs(len, len_inv, wfrag0, wfrag1))) {
                        using PFrag = ProjectedFragment<Varying>;

                        // apply fragment shader:
                        const auto targets = program.on_fragment(uniforms, PFrag{ pos, depth, attrs });

                        // plot point in framebuffer:
                        framebuffer.plot(math::Vec2Int{ pos }, 0, targets);
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
                    return std::make_tuple(*r.cbegin(), *(r.cbegin() + 1));
                };
                if (std::ranges::distance(range) % 2U == 0U) {
                    draw_lines(range | std::ranges::views::chunk(2U) | std::ranges::views::transform(func));
                } else if (std::ranges::distance(range) >= 1U) {
                    draw_lines(range | std::ranges::views::take(std::ranges::distance(range) - 1U) |
                               std::ranges::views::chunk(2U) | std::ranges::views::transform(func));
                }
            } else if (shape_type == ShapeType::LINE_STRIP) {
                draw_lines(range | std::ranges::views::adjacent<2U>);
            } else if (shape_type == ShapeType::LINE_LOOP) {
                draw_lines(range | std::ranges::views::adjacent<2U>, true);
            }
            break;
        }
    }
};

};
