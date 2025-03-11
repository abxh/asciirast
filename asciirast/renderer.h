// TODO:
// improve documentation
// create AABB objects

#pragma once

#include <cassert>
#include <cstddef>
#include <ranges>
#include <vector>

#include "math/types.h"
#include "program.h"
#include "rasterize.h"

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
public:
    Renderer(const math::Vec2 viewport_min = math::Vec2{ 0.f, 0.f },
             const math::Vec2 viewport_max = math::Vec2{ 1.f, 1.f })
    {
        assert(0.f <= viewport_min.x && viewport_min.x < viewport_max.x && viewport_max.x <= 1.f);
        assert(0.f <= viewport_min.y && viewport_min.y < viewport_max.y && viewport_max.y <= 1.f);

        m_screen_to_viewport = math::Transform2()
                                       .translate(1.f, 1.f)
                                       .scale(0.5f, 0.5f)
                                       .scale(viewport_max - viewport_min)
                                       .translate(viewport_min);
    }

    template<class Uniforms, class Vertex, class Varying, class Framebuffer>
    void draw(const Program<Uniforms, Vertex, Varying, Framebuffer>& program,
              const Uniforms& uniforms,
              const VertexBuffer<Vertex>& verts,
              Framebuffer& out) const
    {
        draw(program, uniforms, verts.shape_type, std::views::all(verts.verticies), out);
    }

    template<class Uniforms, class Vertex, class Varying, class Framebuffer>
    void draw(const Program<Uniforms, Vertex, Varying, Framebuffer>& program,
              const Uniforms& uniforms,
              const IndexedVertexBuffer<Vertex>& verts,
              Framebuffer& out) const
    {
        auto func = [&verts](const std::size_t i) {
            assert(i < verts.verticies.size() && "index is inside bounds");
            return verts.verticies[i];
        };
        auto view = std::ranges::views::transform(std::views::all(verts.indicies), func);

        draw(program, uniforms, verts.shape_type, view, out);
    }

private:
    math::Transform2 m_screen_to_viewport;

private:
    template<class Uniforms, class Vertex, class Varying, class Framebuffer>
    void draw(const Program<Uniforms, Vertex, Varying, Framebuffer>& program,
              const Uniforms& uniforms,
              const ShapeType shape_type,
              std::ranges::input_range auto&& range,
              Framebuffer& fb) const
    {
        using Targets = typename Framebuffer::Targets;

        switch (shape_type) {
        case ShapeType::POINTS:
            for (const Vertex& vert : range) {
                // apply vertex shader
                // model space -> world space -> view space -> NDC space:
                auto frag = program.on_vertex(uniforms, vert);

                // cull points outside of viewing volume:
                if (cull_point(frag.pos)) {
                    continue;
                }

                // perspective divide
                // NDC space -> screen space:
                frag.pos.xyz /= frag.pos.w;

                // screen space -> viewport space:
                frag.pos.xy = std::move(m_screen_to_viewport.apply(frag.pos.xy));

                // apply fragment shader:
                const auto targets = program.on_fragment(uniforms, frag);

                // viewport space -> window space:
                const auto win_pos = fb.get_viewport_to_window_transform().apply(frag.pos.xy);

                // plot in window:
                fb.plot(win_pos, targets);
            }
            break;
        case ShapeType::LINES:
        case ShapeType::LINE_STRIP:
        case ShapeType::LINE_LOOP:
            auto draw_line = [](std::ranges::input_range auto&& verticies, const bool looped = false) -> void {};
            if (shape_type == ShapeType::LINES) {
                draw_line(range | std::ranges::views::chunk(2U));
            } else if (shape_type == ShapeType::LINE_STRIP) {
                draw_line(range | std::ranges::views::adjacent<2U>);
            } else if (shape_type == ShapeType::LINE_LOOP) {
                draw_line(range | std::ranges::views::adjacent<2U>, true);
            }
            break;
        }
    }
};

};
