// Note:
// Screen ranges from [-1; -1] to [+1; 1]

// TODO:
// improve documentation
// create a AABB object

#pragma once

#include <cassert>
#include <cstddef>
#include <vector>

#include "math/types.h"
#include "program.h"

namespace asciirast {

enum class ShapeType
{
    POINTS,
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
    void apply(const Program<Uniforms, Vertex, Varying, Framebuffer>& program,
               const Uniforms& uniforms,
               const VertexBuffer<Vertex>& verts,
               Framebuffer& out) const
    {
        apply(program, uniforms, verts.shape_type, std::views::all(verts.verticies), out);
    }

    template<class Uniforms, class Vertex, class Varying, class Framebuffer>
    void apply(const Program<Uniforms, Vertex, Varying, Framebuffer>& program,
               const Uniforms& uniforms,
               const IndexedVertexBuffer<Vertex>& verts,
               Framebuffer& out) const
    {
        auto view = std::ranges::views::transform(std::views::all(verts.indicies),
                                                  [&](const std::size_t i) { return verts.verticies[i]; });

        apply(program, uniforms, verts.shape_type, view, out);
    }

private:
    math::Transform2 m_screen_to_viewport;

private:
    bool cull_point(const math::Vec2 p) const;

    template<class Uniforms, class Vertex, class Varying, class Framebuffer>
    Varying transfrom_vertex(const Program<Uniforms, Vertex, Varying, Framebuffer>& program,
                             const Uniforms& uniforms,
                             const Vertex& vert,
                             Framebuffer& out) const
    {
        Varying frag = program.on_vertex(uniforms, vert);

        frag.pos.xyz /= frag.pos.w; // perspective divide

        return frag;
    }

    template<class Uniforms, class Vertex, class Varying, class Framebuffer>
    std::tuple<math::Vec2Int, typename Framebuffer::targets> transfrom_fragment(
            const Program<Uniforms, Vertex, Varying, Framebuffer>& program,
            const Uniforms& uniforms,
            Varying frag,
            Framebuffer& out) const
    {
        frag.pos.xy = std::move(m_screen_to_viewport.apply(frag.pos.xy));

        const auto targets = program.on_fragment(uniforms, frag);

        const math::Vec2 win_pos = out.get_viewport_to_window_transform().apply(frag.pos.xy);

        return { math::Vec2Int{ win_pos }, targets };
    }

    template<class Uniforms, class Vertex, class Varying, class Framebuffer>
    void apply(const Program<Uniforms, Vertex, Varying, Framebuffer>& program,
               const Uniforms& uniforms,
               const ShapeType shape_type,
               std::ranges::input_range auto&& range,
               Framebuffer& out) const
    {
        switch (shape_type) {
        case ShapeType::POINTS:
            for (const Vertex& vert : range) {
                const auto frag = transfrom_vertex(program, uniforms, vert, out);

                if (cull_point(frag.pos.xy)) {
                    continue;
                }

                const auto [pos, targets] = transfrom_fragment(program, uniforms, frag, out);

                out.plot(pos, targets);
            }
            break;
        }
    }
};

};
