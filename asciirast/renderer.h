/**
 * @file program.h
 * @brief Definition of program interface and other related types
 */

// TODO:
// improve documentation

#pragma once

#include <cassert>
#include <cstddef>
#include <deque>
#include <ranges>
#include <vector>

#include "./math/types.h"
#include "./program.h"

#include "./rasterize/bounds_test.h"
#include "./rasterize/interpolate.h"
#include "./rasterize/rasterizate.h"

namespace asciirast {

static inline const auto SCREEN_BOUNDS = math::AABB2D::from_min_max(math::Vec2{ -1, -1 }, math::Vec2{ +1, +1 });

enum class TriangleWindingOrder
{
    CLOCKWISE,
    COUNTER_CLOCKWISE,
    NEITHER,
};

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

struct RendererOptions
{
    TriangleWindingOrder triangle_winding_order = TriangleWindingOrder::NEITHER;
};

template<typename Vertex, class VertexAllocator = std::allocator<Vertex>>
struct VertexBuffer
{
    ShapeType shape_type;
    std::vector<Vertex, VertexAllocator> verticies;
};

template<typename Vertex,
         class VertexAllocator = std::allocator<Vertex>,
         class IndexAllocator  = std::allocator<std::size_t>>
struct IndexedVertexBuffer : VertexBuffer<Vertex, VertexAllocator>
{
    std::vector<std::size_t, IndexAllocator> indicies;
};

template<VaryingInterface Varying,
         typename Vec4TripletAllocator  = std::allocator<rasterize::Vec4Triplet>,
         typename AttrsTripletAllocator = std::allocator<rasterize::AttrsTriplet<Varying>>>
class Renderer
{
    bool m_requires_screen_clipping        = false;
    math::Transform2D m_screen_to_viewport = {};
    math::Transform2D m_screen_to_window   = {};

    static inline math::Transform2D screen_to_viewport_transform(const math::AABB2D& viewport_bounds,
                                                                 const math::AABB2D& screen_bounds)
    {
        assert(viewport_bounds.size_get().x != math::F{ 0 });
        assert(viewport_bounds.size_get().y != math::F{ 0 });

        const auto rel_size = viewport_bounds.size_get() / screen_bounds.size_get();
        const auto min_vec  = screen_bounds.min_get().vector_to(viewport_bounds.min_get());

        return math::Transform2D().scale(rel_size).translate(min_vec);
    }

public:
    Renderer() {};

    Renderer(const math::AABB2D& viewport_bounds)
            : m_requires_screen_clipping{ !SCREEN_BOUNDS.contains(viewport_bounds) }
            , m_screen_to_viewport{ screen_to_viewport_transform(viewport_bounds, SCREEN_BOUNDS) } {};

    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class VertexAllocator>
        requires(detail::can_use_program_with<Program, Uniform, Vertex, Varying, FrameBuffer>::value)
    void draw(const Program& program,
              const Uniform& uniform,
              const VertexBuffer<Vertex, VertexAllocator>& verts,
              FrameBuffer& out,
              RendererOptions options = {})
    {
        draw(program, uniform, verts.shape_type, std::views::all(verts.verticies), out, options);
    }

    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class VertexAllocator,
             class IndexAllocator>
        requires(detail::can_use_program_with<Program, Uniform, Vertex, Varying, FrameBuffer>::value)
    void draw(const Program& program,
              const Uniform& uniform,
              const IndexedVertexBuffer<Vertex, VertexAllocator, IndexAllocator>& verts,
              FrameBuffer& out,
              RendererOptions options = {})
    {
        const auto func = [&verts](const std::size_t i) -> Vertex {
            assert(i < verts.verticies.size() && "index is inside bounds");

            return verts.verticies[i];
        };
        const auto view = std::ranges::views::transform(std::views::all(verts.indicies), func);

        draw(program, uniform, verts.shape_type, view, out, options);
    }

private:
    std::deque<rasterize::Vec4Triplet, Vec4TripletAllocator> vec_queue                     = {};
    std::deque<rasterize::AttrsTriplet<Varying>, AttrsTripletAllocator> vertex_attrs_queue = {};

    template<ProgramInterface Program, class Uniform, FrameBufferInterface FrameBuffer>
    void draw(const Program& program,
              const Uniform& uniform,
              const ShapeType shape_type,
              std::ranges::input_range auto&& range,
              FrameBuffer& framebuffer,
              const RendererOptions& options)
    {
        using Vertex  = typename Program::Vertex;
        using Frag    = Fragment<Varying>;
        using PFrag   = ProjectedFragment<Varying>;
        using Targets = typename Program::Targets;

        const bool clockwise_winding_order = options.triangle_winding_order == TriangleWindingOrder::CLOCKWISE;
        const bool neither_winding_order   = options.triangle_winding_order == TriangleWindingOrder::NEITHER;

        if (const math::Transform2D screen_to_window = framebuffer.screen_to_window();
            !std::ranges::equal(m_screen_to_window.mat().range(), screen_to_window.mat().range())) {
            m_screen_to_window = std::move(screen_to_window);
        }

        const auto round_pos = [](const math::Vec2& pos) -> math::Vec2 {
            return math::floor(pos + math::Vec2{ 0.5f, 0.5f });
        };
        const auto scale_to_viewport_func = [this](const PFrag& pfrag) -> PFrag {
            return PFrag{ .pos   = m_screen_to_viewport.apply(pfrag.pos),
                          .z_inv = pfrag.z_inv,
                          .w_inv = pfrag.w_inv,
                          .attrs = pfrag.attrs };
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
                const Frag frag = program.on_vertex(uniform, vert);

                // cull points outside of viewing volume:
                if (!rasterize::point_in_frustum(frag.pos)) {
                    continue;
                }

                // perspective divide
                // clip space -> screen space:
                const PFrag pfrag = rasterize::project(frag);

                // scale up to viewport:
                const PFrag vfrag = scale_to_viewport_func(pfrag);

                // cull points outside of screen:
                if (m_requires_screen_clipping && !rasterize::point_in_screen(vfrag.pos)) {
                    continue;
                }

                // screen space -> window space:
                const PFrag wfrag = screen_to_window_func(vfrag);

                // apply fragment shader:
                const Targets targets = program.on_fragment(uniform, wfrag);

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
                    const Frag frag0 = program.on_vertex(uniform, v0);
                    const Frag frag1 = program.on_vertex(uniform, v1);

                    // clip line so it's inside the viewing volume:
                    const auto tup = rasterize::line_in_frustum(frag0.pos, frag1.pos);
                    if (!tup.has_value()) {
                        return;
                    }
                    const auto [t0, t1] = tup.value(); // interpolate line using t values
                    const Frag tfrag0   = rasterize::lerp(frag0, frag1, t0);
                    const Frag tfrag1   = rasterize::lerp(frag0, frag1, t1);

                    // perspective divide
                    // clip space -> screen space:
                    const PFrag pfrag0 = rasterize::project(tfrag0);
                    const PFrag pfrag1 = rasterize::project(tfrag1);

                    // scale up to viewport:
                    const PFrag vfrag0 = scale_to_viewport_func(pfrag0);
                    const PFrag vfrag1 = scale_to_viewport_func(pfrag1);

                    // clip line so it's inside the screen:
                    PFrag vtfrag0 = vfrag0;
                    PFrag vtfrag1 = vfrag1;
                    if (m_requires_screen_clipping) {
                        const auto vtup = rasterize::line_in_screen(vfrag0.pos, vfrag1.pos);
                        if (!vtup.has_value()) {
                            return;
                        }
                        const auto [vt0, vt1] = vtup.value();

                        vtfrag0 = rasterize::lerp(vfrag0, vfrag1, vt0);
                        vtfrag1 = rasterize::lerp(vfrag0, vfrag1, vt1);
                    }

                    // screen space -> window space:
                    const PFrag wfrag0 = screen_to_window_func(vtfrag0);
                    const PFrag wfrag1 = screen_to_window_func(vtfrag1);

                    // iterate over line fragments:
                    const auto func = [&program, &framebuffer, &uniform](const math::Vec2& pos,
                                                                         const math::F z_inv,
                                                                         const math::F w_inv,
                                                                         const Varying& attrs) -> void {
                        // apply fragment shader:
                        const Targets targets = program.on_fragment(uniform, PFrag{ pos, z_inv, w_inv, attrs });

                        // plot point in framebuffer:
                        framebuffer.plot(math::Vec2Int{ pos }, z_inv, targets);
                    };
                    rasterize::rasterize_line(wfrag0, wfrag1, func);
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
            assert(!m_requires_screen_clipping && "not supported yet");

            const auto draw_triangles = [&](std::ranges::input_range auto&& verticies, const bool fan = false) -> void {
                const auto draw_triangle = [&](const Vertex& v0, const Vertex& v1, const Vertex& v2) -> void {
                    // apply vertex shader
                    // model space -> world space -> view space -> clip space:
                    const Frag frag0 = program.on_vertex(uniform, v0);
                    const Frag frag1 = program.on_vertex(uniform, v1);
                    const Frag frag2 = program.on_vertex(uniform, v2);

                    const auto signed_area_2 = math::cross(frag0.pos.vector_to(frag2.pos).xyz.to_vec(),
                                                           frag0.pos.vector_to(frag1.pos).xyz.to_vec());

                    // perform backface culling:
                    if (!neither_winding_order && 0 >= signed_area_2) {
                        return;
                    }

                    vec_queue.clear();
                    vertex_attrs_queue.clear();

                    // sort vertices after winding order:
                    if (clockwise_winding_order || (neither_winding_order && 0 < signed_area_2)) {
                        vec_queue.insert(vec_queue.end(), { frag0.pos, frag1.pos, frag2.pos });
                        vertex_attrs_queue.insert(vertex_attrs_queue.end(), { frag0.attrs, frag1.attrs, frag2.attrs });
                    } else {
                        vec_queue.insert(vec_queue.end(), { frag0.pos, frag2.pos, frag1.pos });
                        vertex_attrs_queue.insert(vertex_attrs_queue.end(), { frag0.attrs, frag2.attrs, frag1.attrs });
                    }

                    // clip triangle so it's inside the viewing volume:
                    if (!rasterize::triangle_in_frustum(vec_queue, vertex_attrs_queue)) {
                        return;
                    }
                    for (const auto& [vec_triplet, attrs_triplet] :
                         std::ranges::views::zip(vec_queue, vertex_attrs_queue)) {
                        const auto [vec0, vec1, vec2]       = vec_triplet;
                        const auto [attrs0, attrs1, attrs2] = attrs_triplet;

                        const Frag tfrag0 = { .pos = vec0, .attrs = attrs0 };
                        const Frag tfrag1 = { .pos = vec1, .attrs = attrs1 };
                        const Frag tfrag2 = { .pos = vec2, .attrs = attrs2 };

                        // perspective divide
                        // clip space -> screen space:
                        const PFrag pfrag0 = rasterize::project(tfrag0);
                        const PFrag pfrag1 = rasterize::project(tfrag1);
                        const PFrag pfrag2 = rasterize::project(tfrag2);

                        // scale to viewport:
                        const PFrag vfrag0 = scale_to_viewport_func(pfrag0);
                        const PFrag vfrag1 = scale_to_viewport_func(pfrag1);
                        const PFrag vfrag2 = scale_to_viewport_func(pfrag2);

                        // TODO: clipping after viewport

                        // screen space -> window space:
                        const PFrag wfrag0 = screen_to_window_func(vfrag0);
                        const PFrag wfrag1 = screen_to_window_func(vfrag1);
                        const PFrag wfrag2 = screen_to_window_func(vfrag2);

                        // iterate over triangle fragments:
                        const auto func = [&program, &framebuffer, &uniform](const math::Vec2& pos,
                                                                             const math::F z_inv,
                                                                             const math::F w_inv,
                                                                             const Varying& attrs) -> void {
                            // apply fragment shader:
                            const Targets targets = program.on_fragment(uniform, PFrag{ pos, z_inv, w_inv, attrs });

                            // plot point in framebuffer:
                            framebuffer.plot(math::Vec2Int{ pos }, z_inv, targets);
                        };
                        rasterize::rasterize_triangle(wfrag0, wfrag1, wfrag2, func);
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
}; // namespace asciirast
