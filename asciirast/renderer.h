// Note:
// Screen ranges from [-1; -1] to [+1; 1]

// TODO: improve documentation

// inspiration:
// https://github.com/nikolausrauch/software-rasterizer

#pragma once

#include <cassert>
#include <cstddef>
#include <vector>

#include "math/types.h"
#include "math/types/Vec.h"
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
    Renderer()
    {
        // TODO: for now, viewport min max is fixed
        const auto viewport_min = math::Vec2{ 0.f, 0.f };
        const auto viewport_max = math::Vec2{ 1.f, 1.f };

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
        // TODO: assuming only points and VertexBuffer are drawn (for now)

        using Targets = Framebuffer::targets;

        for (const Vertex& vert : verts.verticies) {
            Varying frag = program.on_vertex(uniforms, vert);

            // perspective divide and apply screen to viewport transformation:
            frag.pos.xyz /= frag.pos.w;
            frag.pos.xy = std::move(m_screen_to_viewport.apply(frag.pos.xy));

            Targets targets = program.on_fragment(uniforms, frag);

            // apply viewport to window transformation:
            frag.pos.xy = std::move(out.get_viewport_to_window_transform().apply(frag.pos.xy));

            // finally convert position to int and plot:
            out.plot(math::Vec2Int{ frag.pos.xy }, targets);
        }
    }

private:
    math::Transform2 m_screen_to_viewport;
};

};
