/**
 * @file renderer.h
 * @brief The renderer class and related types
 *
 * @todo improve documentation
 */

#pragma once

#include <cassert>
#include <deque>
#include <ranges>
#include <vector>

#include "./constants.h"
#include "./math/types.h"
#include "./program.h"
#include "./rasterize/bounds_test.h"
#include "./rasterize/interpolate.h"
#include "./rasterize/rasterize.h"

namespace asciirast {

/**
 * @brief Triangle winding order
 */
enum class WindingOrder
{
    CLOCKWISE,
    COUNTER_CLOCKWISE,
    NEITHER,
};

/**
 * @brief Shape primitives
 */
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

/**
 * @brief Renderer options
 */
struct RendererOptions
{
    WindingOrder winding_order = WindingOrder::NEITHER; ///< Triangle winding order
};

/**
 * @brief Vertex buffer
 *
 * @tparam Vertex The vertex type
 */
template<typename Vertex, class VertexAllocator = std::allocator<Vertex>>
struct VertexBuffer
{
    ShapeType shape_type;                           ///< Shape type
    std::vector<Vertex, VertexAllocator> verticies; ///< Buffer of verticies
};

/**
 * @brief Indexed vertex buffer
 *
 * @tparam Vertex Vertex type
 */
template<typename Vertex,
         class VertexAllocator = std::allocator<Vertex>,
         class IndexAllocator = std::allocator<std::size_t>>
struct IndexedVertexBuffer : VertexBuffer<Vertex, VertexAllocator>
{
    std::vector<std::size_t, IndexAllocator> indicies; ///< Buffer of indicies
};

/**
 * @brief Reusuable renderer pipeline data buffers
 */
template<VaryingInterface Varying,
         typename Vec4TripletAllocator = std::allocator<rasterize::Vec4Triplet>,
         typename AttrsTripletAllocator = std::allocator<rasterize::AttrsTriplet<Varying>>>
struct RendererPipelineData
{
    using Vec4TripletQueue = std::deque<rasterize::Vec4Triplet, Vec4TripletAllocator>; ///< Vec4 queue alias
    using AttrsTripletQueue =
            std::deque<rasterize::AttrsTriplet<Varying>, AttrsTripletAllocator>; ///< Attrs queue alias

    Vec4TripletQueue vec_queue = {};          ///< Vec4 Queue
    AttrsTripletQueue attrs_queue = {};       ///< Attrs Queue
    Vec4TripletQueue inner_vec_queue = {};    ///< Another Vec4 Queue
    AttrsTripletQueue inner_attrs_queue = {}; ///< Another Attrs Queue
};

/**
 * @brief Renderer class
 */
class Renderer
{
public:
    /**
     * @brief Construct renderer with default viewport
     */
    Renderer() {};

    /**
     * @brief Construct renderer with custom viewport
     *
     * The portion of the viewport in the screen is shown.
     * If the viewport is differently sized, the result is scaled accordingly.
     *
     * @param viewport_bounds The bounds of the viewport
     */
    explicit Renderer(const math::AABB2D& viewport_bounds)
            : m_requires_screen_clipping{ !constants::SCREEN_BOUNDS.contains(viewport_bounds) }
            , m_screen_to_viewport{ screen_to_viewport_transform(viewport_bounds, constants::SCREEN_BOUNDS) } {};

    /**
     * @brief Draw on a framebuffer using a program given uniform(s),
     * verticies and options
     *
     * @param program The shader program
     * @param uniform The uniform(s)
     * @param verts The vertex buffer
     * @param framebuffer The frame buffer
     * @param options Optional options
     */
    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class VertexAllocator>
        requires(detail::can_use_program_with<Program, Uniform, Vertex, FrameBuffer>::value)
    void draw(const Program& program,
              const Uniform& uniform,
              const VertexBuffer<Vertex, VertexAllocator>& verts,
              FrameBuffer& framebuffer,
              RendererOptions options = {})
    {
        update_screen_to_window(framebuffer);

        auto temp_pipeline_data = RendererPipelineData<typename Program::Varying>{};

        draw(program,
             uniform,
             verts.shape_type,
             std::views::all(verts.verticies),
             framebuffer,
             options,
             temp_pipeline_data);
    }

    /**
     * @brief Draw on a framebuffer using a program given uniform(s),
     * verticies, options and pipeline data
     *
     * @param program The shader program
     * @param uniform The uniform(s)
     * @param verts The vertex buffer
     * @param framebuffer The frame buffer
     * @param options Options
     * @param pipeline_data Reusuable pipeline data buffers
     */
    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class VertexAllocator,
             class Vec4TripletAllocator,
             class AttrsTripletAllocator>
        requires(detail::can_use_program_with<Program, Uniform, Vertex, FrameBuffer>::value)
    void draw(
            const Program& program,
            const Uniform& uniform,
            const VertexBuffer<Vertex, VertexAllocator>& verts,
            FrameBuffer& framebuffer,
            RendererOptions options,
            RendererPipelineData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& pipeline_data)
    {
        update_screen_to_window(framebuffer);

        draw(program, uniform, verts.shape_type, std::views::all(verts.verticies), framebuffer, options, pipeline_data);
    }

    /**
     * @brief Draw on a framebuffer using a program given uniform(s),
     * indexed verticies and options
     *
     * @param program The shader program
     * @param uniform The uniform(s)
     * @param verts The vertex buffer with indicies
     * @param framebuffer The frame buffer
     * @param options Optional options
     */
    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class VertexAllocator,
             class IndexAllocator>
        requires(detail::can_use_program_with<Program, Uniform, Vertex, FrameBuffer>::value)
    void draw(const Program& program,
              const Uniform& uniform,
              const IndexedVertexBuffer<Vertex, VertexAllocator, IndexAllocator>& verts,
              FrameBuffer& framebuffer,
              RendererOptions options = {})
    {
        const auto func = [&verts](const std::size_t i) -> Vertex {
            assert(i < verts.verticies.size() && "index is inside bounds");

            return verts.verticies[i];
        };
        const auto view = std::ranges::views::transform(std::views::all(verts.indicies), func);

        update_screen_to_window(framebuffer);

        auto temp_pipeline_data = RendererPipelineData<typename Program::Varying>{};

        draw(program, uniform, verts.shape_type, view, framebuffer, options, temp_pipeline_data);
    }

    /**
     * @brief Draw on a framebuffer using a program given uniform(s),
     * indexed verticies, options and pipeline data
     *
     * @param program The shader program
     * @param uniform The uniform(s)
     * @param verts The vertex buffer with indicies
     * @param framebuffer The frame buffer
     * @param options Optional options
     * @param pipeline_data Optional reusuable pipeline data buffers
     */
    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class VertexAllocator,
             class IndexAllocator,
             class Vec4TripletAllocator,
             class AttrsTripletAllocator>
        requires(detail::can_use_program_with<Program, Uniform, Vertex, FrameBuffer>::value)
    void draw(
            const Program& program,
            const Uniform& uniform,
            const IndexedVertexBuffer<Vertex, VertexAllocator, IndexAllocator>& verts,
            FrameBuffer& framebuffer,
            const RendererOptions& options,
            RendererPipelineData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& pipeline_data)
    {
        const auto func = [&verts](const std::size_t i) -> Vertex {
            assert(i < verts.verticies.size() && "index is inside bounds");

            return verts.verticies[i];
        };
        const auto view = std::ranges::views::transform(std::views::all(verts.indicies), func);

        update_screen_to_window(framebuffer);

        draw(program, uniform, verts.shape_type, view, framebuffer, options, pipeline_data);
    }

private:
    bool m_requires_screen_clipping = false;
    math::Transform2D m_screen_to_viewport = {};
    math::Transform2D m_screen_to_window = {};

    static inline math::Transform2D screen_to_viewport_transform(const math::AABB2D& viewport_bounds,
                                                                 const math::AABB2D& screen_bounds)
    {
        assert(viewport_bounds.size_get().x != 0 && "non-zero size");
        assert(viewport_bounds.size_get().y != 0 && "non-zero size");

        const auto rel_size = viewport_bounds.size_get() / screen_bounds.size_get();
        const auto shf_size = (screen_bounds.min_get() * rel_size).vector_to(viewport_bounds.min_get());

        return math::Transform2D().scale(rel_size).translate(shf_size);
    }

    template<FrameBufferInterface FrameBuffer>
    void update_screen_to_window(FrameBuffer& framebuffer)
    {
        const math::Transform2D& t = framebuffer.screen_to_window();

        const math::Mat3& cur_mat = m_screen_to_window.mat();
        const math::Mat3& new_mat = t.mat();

        const bool bitwise_eq = std::ranges::equal(cur_mat.array(), new_mat.array());

        if (!bitwise_eq) {
            m_screen_to_window = t;
        }
    }

private:
    template<VaryingInterface Varying>
    ProjectedFragment<Varying> apply_scale_to_viewport(const ProjectedFragment<Varying>& pfrag) const
    {
        return ProjectedFragment<Varying>{ .pos = m_screen_to_viewport.apply(pfrag.pos),
                                           .depth = pfrag.depth,
                                           .Z_inv = pfrag.Z_inv,
                                           .attrs = pfrag.attrs };
    }

    template<VaryingInterface Varying>
    ProjectedFragment<Varying> apply_screen_to_window(const ProjectedFragment<Varying>& pfrag) const
    {
        const math::Vec2 new_pos = m_screen_to_window.apply(pfrag.pos);

        return ProjectedFragment<Varying>{ .pos = math::floor(new_pos + math::Vec2{ 0.5f, 0.5f }),
                                           .depth = pfrag.depth,
                                           .Z_inv = pfrag.Z_inv,
                                           .attrs = pfrag.attrs };
    }

    template<ProgramInterface Program, class Uniform, class Vertex, FrameBufferInterface FrameBuffer>
        requires(std::is_same_v<Vertex, typename Program::Vertex>)
    void draw_point(const Program& program, const Uniform& uniform, FrameBuffer& framebuffer, const Vertex& vert) const
    {
        using Varying = typename Program::Varying;
        using Frag = Fragment<Varying>;
        using PFrag = ProjectedFragment<Varying>;
        using Targets = typename Program::Targets;

        // apply vertex shader
        // model space -> world space -> view space -> clip space:
        const Frag frag = program.on_vertex(uniform, vert);

        // cull points outside of viewing volume:
        if (!rasterize::point_in_frustum(frag.pos)) {
            return;
        }

        // perspective divide
        // clip space -> screen space:
        const PFrag pfrag = project_fragment(frag);

        // scale up to viewport:
        const PFrag vfrag = apply_scale_to_viewport(pfrag);

        if (m_requires_screen_clipping) {
            // cull points outside of screen:
            if (!rasterize::point_in_screen(vfrag.pos)) {
                return;
            }
        }

        // screen space -> window space:
        const PFrag wfrag = apply_screen_to_window(vfrag);

        // apply fragment shader:
        const Targets targets = program.on_fragment(uniform, wfrag);

        // plot in framebuffer:
        const auto pos_int = math::Vec2Int{ wfrag.pos };
        if (framebuffer.test_and_set_depth(pos_int, wfrag.depth)) {
            framebuffer.plot(pos_int, targets);
        }
    }

    template<ProgramInterface Program, class Uniform, class Vertex, FrameBufferInterface FrameBuffer>
        requires(std::is_same_v<Vertex, typename Program::Vertex>)
    void draw_line(const Program& program,
                   const Uniform& uniform,
                   FrameBuffer& framebuffer,
                   const Vertex& v0,
                   const Vertex& v1) const
    {
        using Varying = typename Program::Varying;
        using Frag = Fragment<Varying>;
        using PFrag = ProjectedFragment<Varying>;
        using Targets = typename Program::Targets;

        // apply vertex shader
        // model space -> world space -> view space -> clip space:
        const Frag frag0 = program.on_vertex(uniform, v0);
        const Frag frag1 = program.on_vertex(uniform, v1);

        // clip line so it's inside the viewing volume:
        const auto tup = rasterize::line_in_frustum(frag0.pos, frag1.pos);
        if (!tup.has_value()) {
            return;
        }
        const auto [t0, t1] = tup.value();
        const Frag tfrag0 = rasterize::lerp(frag0, frag1, t0);
        const Frag tfrag1 = rasterize::lerp(frag0, frag1, t1);

        // perspective divide
        // clip space -> screen space:
        const PFrag pfrag0 = project_fragment(tfrag0);
        const PFrag pfrag1 = project_fragment(tfrag1);

        // scale up to viewport:
        const PFrag vfrag0 = apply_scale_to_viewport(pfrag0);
        const PFrag vfrag1 = apply_scale_to_viewport(pfrag1);

        PFrag inner_tfrag0 = vfrag0;
        PFrag inner_tfrag1 = vfrag1;
        if (m_requires_screen_clipping) {
            // clip line so it's inside the screen:
            const auto inner_tup = rasterize::line_in_screen(vfrag0.pos, vfrag1.pos);
            if (!inner_tup.has_value()) {
                return;
            }
            const auto [inner_t0, inner_t1] = inner_tup.value();
            inner_tfrag0 = rasterize::lerp(vfrag0, vfrag1, inner_t0);
            inner_tfrag1 = rasterize::lerp(vfrag0, vfrag1, inner_t1);
        }

        // screen space -> window space:
        const PFrag wfrag0 = apply_screen_to_window(inner_tfrag0);
        const PFrag wfrag1 = apply_screen_to_window(inner_tfrag1);

        const auto test_and_set_depth_func = [&framebuffer](const math::Vec2Int& pos, const math::Float depth) {
            return framebuffer.test_and_set_depth(pos, depth);
        };
        const auto plot_func = [&program, &framebuffer, &uniform](const math::Vec2& pos,
                                                                  const math::Float depth,
                                                                  const math::Float Z_inv,
                                                                  const Varying& attrs) -> void {
            // apply fragment shader:
            const Targets targets = program.on_fragment(uniform, PFrag{ pos, depth, Z_inv, attrs });

            // plot point in framebuffer:
            framebuffer.plot(math::Vec2Int{ pos }, targets);
        };

        // iterate over line fragments:
        rasterize::rasterize_line(wfrag0, wfrag1, plot_func, test_and_set_depth_func);
    }

    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class Vec4TripletAllocator,
             class AttrsTripletAllocator>
        requires(std::is_same_v<Vertex, typename Program::Vertex>)
    void draw_triangle(
            const Program& program,
            const Uniform& uniform,
            FrameBuffer& framebuffer,
            const RendererOptions& options,
            RendererPipelineData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& pipeline_data,
            const Vertex& v0,
            const Vertex& v1,
            const Vertex& v2)
    {
        using Varying = typename Program::Varying;
        using Frag = Fragment<Varying>;
        using PFrag = ProjectedFragment<Varying>;
        using Targets = typename Program::Targets;

        // apply vertex shader
        // model space -> world space -> view space -> clip space:
        const Frag frag0 = program.on_vertex(uniform, v0);
        const Frag frag1 = program.on_vertex(uniform, v1);
        const Frag frag2 = program.on_vertex(uniform, v2);

        const auto test_and_set_depth_func = [&framebuffer](const math::Vec2Int& pos, const math::Float depth) -> bool {
            return framebuffer.test_and_set_depth(pos, depth);
        };
        const auto plot_func = [&program, &framebuffer, &uniform](const math::Vec2& pos,
                                                                  const math::Float depth,
                                                                  const math::Float Z_inv,
                                                                  const Varying& attrs) -> void {
            // apply fragment shader:
            const Targets targets = program.on_fragment(uniform, PFrag{ pos, depth, Z_inv, attrs });

            // plot point in framebuffer:
            framebuffer.plot(math::Vec2Int{ pos }, targets);
        };
        const auto rasterize_triangle = [&options, plot_func, test_and_set_depth_func](
                                                const PFrag& wfrag0, const PFrag& wfrag1, const PFrag& wfrag2) -> void {
            const bool clockwise_winding_order = options.winding_order == WindingOrder::CLOCKWISE;
            const bool counter_clockwise_winding_order = options.winding_order == WindingOrder::COUNTER_CLOCKWISE;
            const bool neither_winding_order = options.winding_order == WindingOrder::NEITHER;

            // perform backface culling:
            const auto p0p2 = wfrag0.pos.vector_to(wfrag2.pos);
            const auto p0p1 = wfrag0.pos.vector_to(wfrag1.pos);
            const auto signed_area_2 = math::cross(p0p2, p0p1);
            const bool backface_cull_cond =
                    !neither_winding_order && ((clockwise_winding_order && 0 < signed_area_2) ||
                                               (counter_clockwise_winding_order && 0 > signed_area_2));
            if (backface_cull_cond) {
                return;
            }

            // iterate over triangle fragments:
            if (clockwise_winding_order || (neither_winding_order && 0 > signed_area_2)) {
                rasterize::rasterize_triangle(wfrag0, wfrag1, wfrag2, plot_func, test_and_set_depth_func);
            } else {
                rasterize::rasterize_triangle(wfrag0, wfrag2, wfrag1, plot_func, test_and_set_depth_func);
            }
        };

        pipeline_data.vec_queue.clear();
        pipeline_data.attrs_queue.clear();
        pipeline_data.vec_queue.insert(pipeline_data.vec_queue.end(),
                                       rasterize::Vec4Triplet{ frag0.pos, frag1.pos, frag2.pos });
        pipeline_data.attrs_queue.insert(pipeline_data.attrs_queue.end(),
                                         rasterize::AttrsTriplet<Varying>{ frag0.attrs, frag1.attrs, frag2.attrs });

        // clip triangle so it's inside the viewing volume:
        if (!rasterize::triangle_in_frustum(pipeline_data.vec_queue, pipeline_data.attrs_queue)) {
            return;
        }
        for (const auto& [vec_triplet, attrs_triplet] :
             std::ranges::views::zip(pipeline_data.vec_queue, pipeline_data.attrs_queue)) {

            const auto [vec0, vec1, vec2] = vec_triplet;
            const auto [attrs0, attrs1, attrs2] = attrs_triplet;

            const Frag tfrag0 = { .pos = vec0, .attrs = attrs0 };
            const Frag tfrag1 = { .pos = vec1, .attrs = attrs1 };
            const Frag tfrag2 = { .pos = vec2, .attrs = attrs2 };

            // perspective divide
            // clip space -> screen space:
            const PFrag pfrag0 = project_fragment(tfrag0);
            const PFrag pfrag1 = project_fragment(tfrag1);
            const PFrag pfrag2 = project_fragment(tfrag2);

            // scale to viewport:
            const PFrag vfrag0 = apply_scale_to_viewport(pfrag0);
            const PFrag vfrag1 = apply_scale_to_viewport(pfrag1);
            const PFrag vfrag2 = apply_scale_to_viewport(pfrag2);

            if (!m_requires_screen_clipping) {
                // screen space -> window space:
                const PFrag wfrag0 = apply_screen_to_window(vfrag0);
                const PFrag wfrag1 = apply_screen_to_window(vfrag1);
                const PFrag wfrag2 = apply_screen_to_window(vfrag2);

                // iterate over triangle fragments:
                rasterize_triangle(wfrag0, wfrag1, wfrag2);
            } else {
                pipeline_data.inner_vec_queue.clear();
                pipeline_data.inner_attrs_queue.clear();

                const auto p0 = math::Vec4{ vfrag0.pos, vfrag0.depth, vfrag0.Z_inv };
                const auto p1 = math::Vec4{ vfrag1.pos, vfrag1.depth, vfrag1.Z_inv };
                const auto p2 = math::Vec4{ vfrag2.pos, vfrag2.depth, vfrag2.Z_inv };
                const auto [a0, a1, a2] = rasterize::AttrsTriplet<Varying>{ vfrag0.attrs, vfrag1.attrs, vfrag2.attrs };
                const auto lp = rasterize::Vec4Triplet{ p0, p1, p2 };
                const auto la = rasterize::AttrsTriplet<Varying>{ a0, a1, a2 };

                pipeline_data.inner_vec_queue.insert(pipeline_data.inner_vec_queue.end(), lp);
                pipeline_data.inner_attrs_queue.insert(pipeline_data.inner_attrs_queue.end(), la);

                // clip line so it's inside the screen:
                if (!rasterize::triangle_in_screen(pipeline_data.inner_vec_queue, pipeline_data.inner_attrs_queue)) {
                    return;
                }
                for (const auto& [inner_vec_triplet, inner_attrs_triplet] :
                     std::ranges::views::zip(pipeline_data.inner_vec_queue, pipeline_data.inner_attrs_queue)) {

                    const auto [inner_vec0, inner_vec1, inner_vec2] = inner_vec_triplet;
                    const auto [inner_attrs0, inner_attrs1, inner_attrs2] = inner_attrs_triplet;

                    const PFrag inner_tfrag0 = { inner_vec0.xy, inner_vec0.z, inner_vec0.w, inner_attrs0 };
                    const PFrag inner_tfrag1 = { inner_vec1.xy, inner_vec1.z, inner_vec1.w, inner_attrs1 };
                    const PFrag inner_tfrag2 = { inner_vec2.xy, inner_vec2.z, inner_vec2.w, inner_attrs2 };

                    // screen space -> window space:
                    const PFrag wfrag0 = apply_screen_to_window(inner_tfrag0);
                    const PFrag wfrag1 = apply_screen_to_window(inner_tfrag1);
                    const PFrag wfrag2 = apply_screen_to_window(inner_tfrag2);

                    // iterate over triangle fragments:
                    rasterize_triangle(wfrag0, wfrag1, wfrag2);
                }
            }
        }
    }

    template<ProgramInterface Program,
             class Uniform,
             FrameBufferInterface FrameBuffer,
             class Vec4TripletAllocator,
             class AttrsTripletAllocator>
    void draw(
            const Program& program,
            const Uniform& uniform,
            const ShapeType shape_type,
            std::ranges::input_range auto&& vert_range,
            FrameBuffer& framebuffer,
            const RendererOptions& options,
            RendererPipelineData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& pipeline_data)
    {
        using Vertex = typename Program::Vertex;

        switch (shape_type) {
        case ShapeType::POINTS: {
            for (const Vertex& vert : vert_range) {
                draw_point(program, uniform, framebuffer, vert);
            }
        } break;
        case ShapeType::LINES: {
            const auto func = [](auto&& range) -> std::tuple<Vertex, Vertex> {
                return std::make_tuple(*range.cbegin(), *(range.cbegin() + 1U));
            };
            const auto rem = std::ranges::distance(vert_range) % 2U;
            const auto subrange = vert_range | std::ranges::views::take(std::ranges::distance(vert_range) - rem);
            const auto verticies = subrange | std::ranges::views::chunk(2U) | std::ranges::views::transform(func);

            for (const auto [v0, v1] : verticies) {
                draw_line(program, uniform, framebuffer, v0, v1);
            }
        } break;
        case ShapeType::LINE_STRIP: {
            const auto verticies = vert_range | std::ranges::views::adjacent<2U>;

            for (const auto [v0, v1] : verticies) {
                draw_line(program, uniform, framebuffer, v0, v1);
            }
        } break;
        case ShapeType::LINE_LOOP: {
            const auto verticies = vert_range | std::ranges::views::adjacent<2U>;

            for (const auto [v0, v1] : verticies) {
                draw_line(program, uniform, framebuffer, v0, v1);
            }
            if (std::ranges::distance(vert_range) >= 1U) {
                const auto v0 = std::get<1>(*(verticies.cend() - 1U));
                const auto v1 = std::get<0>(*verticies.cbegin());

                draw_line(program, uniform, framebuffer, v0, v1);
            };
        } break;
        case ShapeType::TRIANGLES: {
            const auto func = [](auto&& range) -> std::tuple<Vertex, Vertex, Vertex> {
                return std::make_tuple(*range.cbegin(), *(range.cbegin() + 1U), *(range.cbegin() + 2U));
            };
            const auto rem = std::ranges::distance(vert_range) % 3U;
            const auto subrange = vert_range | std::ranges::views::take(std::ranges::distance(vert_range) - rem);
            const auto verticies = subrange | std::ranges::views::chunk(3U) | std::ranges::views::transform(func);

            for (const auto [v0, v1, v2] : verticies) {
                draw_triangle(program, uniform, framebuffer, options, pipeline_data, v0, v1, v2);
            }
        } break;
        case ShapeType::TRIANGLE_STRIP: {
            const auto verticies = vert_range | std::ranges::views::adjacent<3U>;

            for (const auto [v0, v1, v2] : verticies) {
                draw_triangle(program, uniform, framebuffer, options, pipeline_data, v0, v1, v2);
            }
        } break;
        case ShapeType::TRIANGLE_FAN: {
            const auto verticies = vert_range | std::ranges::views::adjacent<3U>;

            for (const auto [v0, v1, v2] : verticies) {
                draw_triangle(program, uniform, framebuffer, options, pipeline_data, v0, v1, v2);
            }
            if (std::ranges::distance(vert_range) >= 1U) {
                const auto v0 = std::get<1>(*(verticies.cend() - 1U));
                const auto v1 = std::get<2>(*(verticies.cend() - 1U));
                const auto v2 = std::get<0>(*verticies.cbegin());

                draw_triangle(program, uniform, framebuffer, options, pipeline_data, v0, v1, v2);
            };
        } break;
        }
    }
};

}; // namespace asciirast
