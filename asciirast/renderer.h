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

template<typename Vertex>
struct VertexBuffer
{
    ShapeType shape_type;
    std::vector<Vertex> verticies;
};

template<typename Vertex>
struct IndexedVertexBuffer : VertexBuffer<Vertex>
{
    std::vector<std::size_t> indicies;
};

class Renderer
{
private:
    math::Transform2 m_screen_to_viewport;
    math::Transform2 m_screen_to_window;

public:
    static inline const math::AABB2 SCREEN_BOUNDS =
            math::AABB2::from_min_max(math::Vec2{ -1, -1 }, math::Vec2{ +1, +1 });
    static inline const math::AABB2 VIEWPORT_BOUNDS =
            math::AABB2::from_min_max(math::Vec2{ +0, +0 }, math::Vec2{ +1, +1 });

    Renderer()
            : m_screen_to_viewport{ SCREEN_BOUNDS.to_transform2().reversed() }
            , m_screen_to_window{}
    {
    }

    Renderer(const math::AABB2& viewport)
            : m_screen_to_viewport{ SCREEN_BOUNDS.to_transform2().reversed().stack(viewport.to_transform2()) }
            , m_screen_to_window{}
    {
        assert(viewport.size_get() != math::Vec2{ 0 });
        assert(VIEWPORT_BOUNDS.contains(viewport));
    }

    template<class Uniforms, class Vertex, VaryingType Varying, FrameBufferType FrameBuffer>
    void draw(const Program<Uniforms, Vertex, Varying, FrameBuffer>& program,
              const Uniforms& uniforms,
              const VertexBuffer<Vertex>& verts,
              FrameBuffer& out)
    {
        draw(program, uniforms, verts.shape_type, std::views::all(verts.verticies), out);
    }

    template<class Uniforms, class Vertex, VaryingType Varying, FrameBufferType FrameBuffer>
    void draw(const Program<Uniforms, Vertex, Varying, FrameBuffer>& program,
              const Uniforms& uniforms,
              const IndexedVertexBuffer<Vertex>& verts,
              FrameBuffer& out)
    {
        auto func = [&verts](const std::size_t i) -> const Vertex {
            assert(i < verts.verticies.size() && "index is inside bounds");

            return verts.verticies[i];
        };
        auto view = std::ranges::views::transform(std::views::all(verts.indicies), func);

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
        if (auto&& t = framebuffer.get_viewport_to_window(); t.changed()) {
            m_screen_to_window = math::Transform2().stack(m_screen_to_viewport).stack(t.get());
        }

        auto screen_to_window_func = [this](const math::Vec2& pos) -> math::Vec2 {
            return math::floor(m_screen_to_window.apply(pos) + math::Vec2{ 0.5f, 0.5f });
        };

        using Targets = typename FrameBuffer::Targets;

        switch (shape_type) {
        case ShapeType::POINTS:
            for (const Vertex& vert : range) {
                // apply vertex shader
                // model space -> world space -> view space -> clip space:
                auto frag = program.on_vertex(uniforms, vert);

                // cull points outside of viewing volume:
                if (rasterize::cull_point(frag.pos)) {
                    continue;
                }

                // perspective divide
                // clip space -> screen space:
                auto pfrag = project(frag);

                // screen space -> window space:
                pfrag.pos = std::move(screen_to_window_func(pfrag.pos));

                // apply fragment shader:
                const auto targets = program.on_fragment(uniforms, pfrag);

                // plot in framebuffer:
                framebuffer.plot(math::Vec2Int{ pfrag.pos }, pfrag.depth, targets);
            }
            break;
        case ShapeType::LINES:
        case ShapeType::LINE_STRIP:
        case ShapeType::LINE_LOOP:
            auto draw_lines = [&](std::ranges::input_range auto&& verticies, const bool looped = false) -> void {
                auto draw_line = [&](const Vertex& v0, const Vertex& v1) -> void {
                    // apply vertex shader
                    // model space -> world space -> view space -> clip space:
                    auto frag0 = program.on_vertex(uniforms, v0);
                    auto frag1 = program.on_vertex(uniforms, v1);

                    // clip line so it's inside the viewing volume:
                    const auto tup = rasterize::clip_line(frag0.pos, frag1.pos);
                    if (!tup.has_value()) {
                        return;
                    }
                    const auto [t0, t1] = tup.value(); // interpolate line using t values:
                    frag0 = std::move(lerp(frag0, frag1, t0));
                    frag1 = std::move(lerp(frag0, frag1, t1));

                    // perspective divide
                    // clip space -> screen space:
                    auto pfrag0 = project(frag0);
                    auto pfrag1 = project(frag1);

                    // screen space -> window space:
                    pfrag0.pos = std::move(screen_to_window_func(pfrag0.pos));
                    pfrag1.pos = std::move(screen_to_window_func(pfrag1.pos));

                    const auto delta = pfrag1.pos - pfrag0.pos;
                    const auto size = math::abs(delta);
                    const auto len = std::max<math::F>(size.x, size.y);
                    const auto len_inv = 1 / len;

                    // iterate over line fragments:
                    for (const auto [pos, depth, attrs] :
                         std::ranges::views::zip(rasterize::generate_line(delta, size, pfrag0.pos, pfrag1.pos),
                                                 rasterize::generate_depth(len, len_inv, pfrag0.depth, pfrag1.depth),
                                                 rasterize::generate_attrs(len, len_inv, pfrag0, pfrag1))) {
                        using PFrag = ProjectedFragment<Varying>;

                        // apply fragment shader:
                        const auto targets = program.on_fragment(uniforms, PFrag{ pos, depth, attrs });

                        // plot point in framebuffer:
                        framebuffer.plot(math::Vec2Int{ pos }, 0, targets);
                    }
                };
                for (auto [v0, v1] : verticies) {
                    draw_line(v0, v1);
                }
                if (looped && std::ranges::distance(range) >= 1U) {
                    draw_line(std::get<1>(*(verticies.cend() - 1U)), std::get<0>(*verticies.cbegin()));
                };
            };
            if (shape_type == ShapeType::LINES) {
                auto func = [](auto&& r) -> std::tuple<Vertex, Vertex> {
                    auto&& it = r.cbegin();
                    return std::make_tuple(*it, *it++);
                };
                if (std::ranges::distance(range) % 2U == 0U) {
                    draw_lines(range | std::ranges::views::chunk(2U) | std::ranges::views::transform(func));
                } else if (std::ranges::distance(range) >= 1U) {
                    draw_lines(range | std::ranges::views::chunk(2U) |
                               std::ranges::views::take(std::ranges::distance(range) - 1U) |
                               std::ranges::views::transform(func));
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
