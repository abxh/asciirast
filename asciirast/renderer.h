// TODO:
// improve documentation

#pragma once

#include <cassert>
#include <cstddef>
#include <ranges>
#include <vector>

#include "./math.h"
#include "./program.h"
#include "./rasterize.h"

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
    static inline const math::AABB2 screen_AABB = math::AABB2::from_min_max(math::Vec2{ -1, -1 }, math::Vec2{ +1, +1 });

    Renderer()
            : m_screen_to_viewport{ screen_AABB.to_transform().reversed() }
    {
    }

    Renderer(const math::AABB2& viewport_AABB)
            : m_screen_to_viewport{ screen_AABB.to_transform().reversed().stack(viewport_AABB.to_transform()) }
    {
        assert(math::AABB2::from_min_max(math::Vec2{ 0, 0 }, math::Vec2{ 1, 1 }).contains(viewport_AABB));
        assert(viewport_AABB.size_get() != math::Vec2{ 0 });
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
            auto draw_line = [&](std::ranges::input_range auto&& verticies, const bool looped = false) -> void {
                for (const auto& [v1, v2] : verticies) {
                    // apply vertex shader
                    // model space -> world space -> view space -> NDC space:
                    // auto frag1 = program.on_vertex(uniforms, v1);
                    // auto frag2 = program.on_vertex(uniforms, v2);

                    // clip line so it's inside viewing volume:
                    // if () {
                    //     continue;
                    // }
                }
            };
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
