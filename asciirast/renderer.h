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
        auto func = [&verts](const std::size_t i) {
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

        using Targets = typename FrameBuffer::Targets;

        switch (shape_type) {
        case ShapeType::POINTS:
            for (const Vertex& vert : range) {
                // apply vertex shader
                // model space -> world space -> view space -> clip space:
                auto frag = program.on_vertex(uniforms, vert);

                // cull points outside of viewing volume:
                if (cull_point(frag.pos)) {
                    continue;
                }

                // perspective divide
                // clip space -> screen space:
                frag.pos.w = 1 / frag.pos.w;
                frag.pos.xyz *= frag.pos.w;

                // apply fragment shader:
                const auto targets = program.on_fragment(uniforms, frag);

                // screen space -> window space:
                const auto pos = m_screen_to_window.apply(frag.pos.xy);

                // plot in framebuffer:
                framebuffer.plot(pos, targets);
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
                    const auto tup = clip_line(frag0.pos, frag1.pos);
                    if (!tup.has_value()) {
                        return;
                    }
                    const auto [t0, t1] = tup.value(); // interpolate line using t values:
                    frag0 = std::move(lerp(frag0, frag1, t0));
                    frag1 = std::move(lerp(frag0, frag1, t1));

                    // perspective divide
                    // clip space -> screen space:
                    frag0.pos.w = 1 / frag0.pos.w;
                    frag0.pos.xyz *= frag0.pos.w;

                    frag1.pos.w = 1 / frag1.pos.w;
                    frag1.pos.xyz *= frag1.pos.w;

                    static auto plot_point = [](const VaryingType auto& frag,
                                                const math::Transform2& screen_to_window,
                                                const Uniforms& uniforms,
                                                const ProgramType auto& program,
                                                FrameBufferType auto& framebuffer) -> void {
                        // apply fragment shader:
                        const auto targets = program.on_fragment(uniforms, frag);

                        // screen space -> window space:
                        const auto pos = screen_to_window.apply(frag.pos.xy);

                        // plot point in framebuffer:
                        framebuffer.plot(pos, targets);
                    };

                    // plot line in framebuffer:
                    plot_line(plot_point, frag0, frag1, m_screen_to_window, uniforms, program, framebuffer);
                };
                for (auto [v0, v1] : verticies) {
                    draw_line(v0, v1);
                }
                if (looped && std::ranges::distance(range) >= 1U) {
                    draw_line(std::get<1>(*(verticies.cend() - 1U)), std::get<0>(*verticies.cbegin()));
                };
            };
            if (shape_type == ShapeType::LINES) {
                auto func = [](auto&& r) -> std::tuple<const Vertex&, const Vertex&> {
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
