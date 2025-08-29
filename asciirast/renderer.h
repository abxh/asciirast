/**
 * @file renderer.h
 * @brief The renderer class and related types
 */

#pragma once

#include <deque>
#include <vector>

#include "./framebuffer.h"
#include "./math/types.h"
#include "./program.h"
#include "./renderer/rasterize.h"
#include "./renderer/test_bounds.h"
#include "./utils.h"

namespace asciirast {

/**
 * @brief Shape primitives
 */
enum class ShapeType
{
    Points,
    Lines,
    LineStrip,
    LineLoop,
    Triangles,
    TriangleStrip,
    TriangleFan,
};

/**
 * @brief Vertex buffer
 *
 * @tparam Vertex The vertex type
 */
template<typename Vertex, class VertexAllocator = std::allocator<Vertex>>
struct VertexBuffer
{
    ShapeType shape_type = {};                           ///< Shape type
    std::vector<Vertex, VertexAllocator> verticies = {}; ///< Buffer of verticies
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
    std::vector<std::size_t, IndexAllocator> indicies = {}; ///< Buffer of indicies
};

/**
 * @brief Reusuable renderer data
 */
template<VaryingInterface Varying,
         typename Vec4TripletAllocator = std::allocator<std::array<math::Vec4, 3>>,
         typename AttrsTripletAllocator = std::allocator<std::array<Varying, 3>>>
struct RendererData
{
    using Vec4TripletDeque = std::deque<std::array<math::Vec4, 3>, Vec4TripletAllocator>;
    using AttrsTripletDeque = std::deque<std::array<Varying, 3>, AttrsTripletAllocator>;

    Vec4TripletDeque vec_queue = {};        ///< Deque for clipping NDC triangles attributes
    Vec4TripletDeque vec_queue_screen = {}; ///< Deque for clipping screen triangles

    AttrsTripletDeque attrs_queue = {};        ///< Deque for clipping NDC triangle attributes
    AttrsTripletDeque attrs_queue_screen = {}; ///< Deque for clipping screen triangle attributes
};

/**
 * @brief Renderer class
 */
template<RendererOptions Options = {}>
class Renderer
{
    bool m_requires_screen_clipping = false;
    math::Transform2D m_scale_to_viewport = {};

public:
    /**
     * @brief Construct renderer with default viewport
     */
    Renderer() = default;

    /**
     * @brief Construct renderer with custom viewport
     *
     * The portion of the viewport in the screen is shown.
     * If the viewport is differently sized, the result is scaled
     * accordingly.
     *
     * @param viewport_bounds The bounds of the viewport
     */
    explicit Renderer(const math::AABB2D& viewport_bounds) noexcept
        : m_requires_screen_clipping{ !SCREEN_BOUNDS.contains(viewport_bounds) }
        , m_scale_to_viewport{ scale_to_viewport_transform(viewport_bounds, SCREEN_BOUNDS) } {};

    /**
     * @brief Get scale to viewport transform
     *
     * @return Const reference to the scale to viewport transform
     */
    const math::Transform2D& scale_to_viewport() const { return m_scale_to_viewport; }

    /**
     * @brief Draw on a framebuffer using a program given uniform(s),
     * verticies and reusable data buffers
     *
     * @throws std::logic_error If fragments do not syncronize in the same
     * order
     *
     * @param program The shader program
     * @param uniform The uniform(s)
     * @param verts The vertex buffer
     * @param framebuffer The frame buffer
     * @param data Renderer data
     */
    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class VertexAllocator,
             class Vec4TripletAllocator,
             class AttrsTripletAllocator>
        requires(std::is_same_v<typename Program::Uniform, Uniform> &&
                 std::is_same_v<typename Program::Vertex, Vertex> &&
                 std::is_same_v<typename Program::Targets, typename FrameBuffer::Targets>)
    void draw(const Program& program,
              const Uniform& uniform,
              const VertexBuffer<Vertex, VertexAllocator>& verts,
              FrameBuffer& framebuffer,
              RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data) const
    {
        draw(program, uniform, verts.shape_type, std::views::all(verts.verticies), framebuffer, data);
    }

    /**
     * @brief Draw on a framebuffer using a program given uniform(s),
     * indexed verticies and reusable data buffers
     *
     * @throws std::runtime_error If the vertex indicies are out of bounds
     * @throws std::logic_error If fragments do not syncronize in the same
     * order
     *
     * @param program The shader program
     * @param uniform The uniform(s)
     * @param verts The vertex buffer with indicies
     * @param framebuffer The frame buffer
     * @param data Renderer data
     */
    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class VertexAllocator,
             class IndexAllocator,
             class Vec4TripletAllocator,
             class AttrsTripletAllocator>
        requires(std::is_same_v<typename Program::Uniform, Uniform> &&
                 std::is_same_v<typename Program::Vertex, Vertex> &&
                 std::is_same_v<typename Program::Targets, typename FrameBuffer::Targets>)
    void draw(const Program& program,
              const Uniform& uniform,
              const IndexedVertexBuffer<Vertex, VertexAllocator, IndexAllocator>& verts,
              FrameBuffer& framebuffer,
              RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data) const
    {
        const auto func = [&verts](const std::size_t i) -> Vertex {
            if (i >= verts.verticies.size()) {
                throw std::runtime_error("asciirast::Renderer::draw() : vertex index is out of "
                                         "bounds");
            }
            return verts.verticies[i];
        };
        const auto view = std::ranges::views::transform(std::views::all(verts.indicies), func);

        draw(program, uniform, verts.shape_type, view, framebuffer, data);
    }

    /**
     * @brief Calculate the transform to convert screen_bounds points to
     * viewport_bounds poins
     *
     * @param viewport_bounds The viewport bounds AABB
     * @param screen_bounds The screen bounds AABB
     * @return The transform that converts points from the screen to the
     * viewport
     */
    static auto scale_to_viewport_transform(const math::AABB2D& viewport_bounds, const math::AABB2D& screen_bounds)
        -> math::Transform2D
    {
        ASCIIRAST_ASSERT(viewport_bounds.size_get().x != 0, "non-zero x size");
        ASCIIRAST_ASSERT(viewport_bounds.size_get().y != 0, "non-zero y size");

        const auto rel_size = viewport_bounds.size_get() / screen_bounds.size_get();
        const auto shf_size = (screen_bounds.min_get() * rel_size).vector_to(viewport_bounds.min_get());

        return math::Transform2D().scale(rel_size).translate(shf_size);
    }

    /**
     * @brief Apply scale_to_viewport transform
     *
     * @param scale_to_viewport the transform
     * @param pfrag The projected fragment
     * @return The projected fragment transformed
     */
    template<VaryingInterface Varying>
    static auto apply_scale_to_viewport(const math::Transform2D& scale_to_viewport, ProjectedFragment<Varying> frag)
        -> ProjectedFragment<Varying>
    {
        frag.pos = scale_to_viewport.apply(frag.pos);
        return frag;
    }

    /**
     * @brief Apply screen_to_window transform
     *
     * @param screen_to_window the transform
     * @param pfrag The projected fragment
     * @return The projected fragment transformed
     */
    template<VaryingInterface Varying>
    static auto apply_screen_to_window_transform(const math::Transform2D& screen_to_window,
                                                 ProjectedFragment<Varying> frag) -> ProjectedFragment<Varying>
    {
        frag.pos = round(screen_to_window.apply(frag.pos));
        return frag;
    }

private:
    template<ProgramInterface Program,
             class Uniform,
             FrameBufferInterface FrameBuffer,
             class Vec4TripletAllocator,
             class AttrsTripletAllocator>
    void draw(const Program& program,
              const Uniform& uniform,
              const ShapeType shape_type,
              std::ranges::input_range auto&& verticies_inp,
              FrameBuffer& framebuffer,
              RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data) const
    {
        using Vertex = typename Program::Vertex;

        const auto draw_point_func = [&](const Vertex& vert) -> void {
            draw_point(program, uniform, m_requires_screen_clipping, m_scale_to_viewport, framebuffer, vert);
        };
        const auto draw_line_func = [&](const Vertex& v0, const Vertex& v1) -> void {
            draw_line(program, uniform, m_requires_screen_clipping, m_scale_to_viewport, framebuffer, v0, v1);
        };
        const auto draw_triangle_func = [&](const Vertex& v0, const Vertex& v1, const Vertex& v2) -> void {
            draw_triangle(
                program, uniform, m_requires_screen_clipping, m_scale_to_viewport, data, framebuffer, v0, v1, v2);
        };

        switch (shape_type) {
        case ShapeType::Points: {
            for (const Vertex& vert : verticies_inp) {
                draw_point_func(vert);
            }
        } break;
        case ShapeType::Lines: {
            const auto func = [](auto&& range) -> std::tuple<Vertex, Vertex> {
                return std::make_tuple(*range.cbegin(), *(range.cbegin() + 1U));
            };
            const auto rem = std::ranges::distance(verticies_inp) % 2U;
            const auto subrange = verticies_inp | std::ranges::views::take(std::ranges::distance(verticies_inp) - rem);
            const auto verticies_tup = subrange | std::ranges::views::chunk(2U) | std::ranges::views::transform(func);

            for (const auto& [v0, v1] : verticies_tup) {
                draw_line_func(v0, v1);
            }
        } break;
        case ShapeType::LineStrip: {
            const auto verticies_tup = verticies_inp | std::ranges::views::adjacent<2U>;

            for (const auto& [v0, v1] : verticies_tup) {
                draw_line_func(v0, v1);
            }
        } break;
        case ShapeType::LineLoop: {
            const auto verticies_tup = verticies_inp | std::ranges::views::adjacent<2U>;

            for (const auto& [v0, v1] : verticies_tup) {
                draw_line_func(v0, v1);
            }
            if (std::ranges::distance(verticies_inp) >= 2U) {
                const auto v0 = *(std::ranges::cend(verticies_inp) - 1);
                const auto v1 = *(std::ranges::cbegin(verticies_inp) + 0);

                draw_line_func(v0, v1);
            };
        } break;
        case ShapeType::Triangles: {
            const auto func = [](auto&& range) -> std::tuple<Vertex, Vertex, Vertex> {
                return std::make_tuple(*range.cbegin(), *(range.cbegin() + 1U), *(range.cbegin() + 2U));
            };
            const auto rem = std::ranges::distance(verticies_inp) % 3U;
            const auto subrange = verticies_inp | std::ranges::views::take(std::ranges::distance(verticies_inp) - rem);
            const auto verticies = subrange | std::ranges::views::chunk(3U) | std::ranges::views::transform(func);

            for (const auto& [v0, v1, v2] : verticies) {
                draw_triangle_func(v0, v1, v2);
            }
        } break;
        case ShapeType::TriangleStrip: {
            const auto verticies_tup = verticies_inp | std::ranges::views::adjacent<3U>;

            for (const auto& [v0, v1, v2] : verticies_tup) {
                draw_triangle_func(v0, v1, v2);
            }
        } break;
        case ShapeType::TriangleFan: {
            const auto vertex_center = verticies_inp | std::ranges::views::take(1U);
            const auto verticies_tup = verticies_inp | std::ranges::views::drop(1U) | std::ranges::views::adjacent<2U>;

            if (!std::ranges::empty(vertex_center)) {
                const auto v0 = vertex_center[0];
                for (const auto& [v1, v2] : verticies_tup) {
                    draw_triangle_func(v0, v1, v2);
                }
            }
        } break;
        }
    }

    template<ProgramInterface Program, class Uniform, class Vertex, FrameBufferInterface FrameBuffer>
        requires(std::is_same_v<Vertex, typename Program::Vertex>)
    static void draw_point(const Program& program,
                           const Uniform& uniform,
                           const bool requires_screen_clipping,
                           const math::Transform2D& scale_to_viewport,
                           FrameBuffer& framebuffer,
                           const Vertex& vert)
    {
        using Varying = typename Program::Varying;
        using Targets = typename Program::Targets;

        using Frag = Fragment<Varying>;
        using PFrag = ProjectedFragment<Varying>;

        // apply vertex shader
        // model space -> world space -> view space -> clip space:
        Frag frag{};
        program.on_vertex(uniform, vert, frag);

        // cull points outside of viewing volume:
        if (!renderer::point_in_frustum(frag.pos)) {
            return;
        }

        // perspective divide
        // clip space -> screen space:
        const PFrag pfrag = project_fragment(frag);

        // scale up to viewport:
        const PFrag vfrag = apply_scale_to_viewport(scale_to_viewport, pfrag);

        // cull points outside of screen:
        if (requires_screen_clipping && !renderer::point_in_screen(vfrag.pos, SCREEN_BOUNDS)) {
            return;
        }

        // screen space -> window space:
        const PFrag wfrag = apply_screen_to_window_transform(framebuffer.screen_to_window_transform(), vfrag);

        if constexpr (ProgramInterface_FragCoroutineSupport<Program>) {
            using FragmentContext = typename Program::FragmentContext;

            // prepare values:
            std::array<typename FragmentContext::ValueVariant, 4> quad;
            auto context = FragmentContext{ 0, quad, FragmentContext::Type::POINT };
            auto targets = Targets{};

            // apply fragment shader and unpack results:
            for (const ProgramToken result : program.on_fragment(context, uniform, wfrag, targets)) {
                if (result == ProgramToken::Discard) return;
            }
            // plot if point is not discarded:
            const auto pos_int = math::Vec2Int{ wfrag.pos };

            if constexpr (FrameBuffer_DepthSupport<FrameBuffer>) {
                if (framebuffer.test_and_set_depth(pos_int, wfrag.depth)) {
                    framebuffer.plot(pos_int, targets);
                }
            } else {
                framebuffer.plot(pos_int, targets);
            }
        } else {
            static_assert(ProgramInterface_FragRegularSupport<Program>);

            const auto pos_int = math::Vec2Int{ wfrag.pos };
            auto targets = Targets{};

            // early z testing
            if constexpr (FrameBuffer_DepthSupport<FrameBuffer>) {
                if (framebuffer.test_and_set_depth(pos_int, wfrag.depth)) {
                    program.on_fragment(uniform, wfrag, targets);
                    framebuffer.plot(pos_int, targets);
                }
            } else {
                program.on_fragment(uniform, wfrag, targets);
                framebuffer.plot(pos_int, targets);
            }
        }
    }

    template<ProgramInterface Program, class Uniform, class Vertex, FrameBufferInterface FrameBuffer>
        requires(std::is_same_v<Vertex, typename Program::Vertex>)
    static void draw_line(const Program& program,
                          const Uniform& uniform,
                          const bool requires_screen_clipping,
                          const math::Transform2D& scale_to_viewport,
                          FrameBuffer& framebuffer,
                          const Vertex& v0,
                          const Vertex& v1)
    {
        using Varying = typename Program::Varying;
        using Targets = typename Program::Targets;

        using Frag = Fragment<Varying>;
        using PFrag = ProjectedFragment<Varying>;

        // apply vertex shader
        // model space -> world space -> view space -> clip space:
        Frag frag0{};
        Frag frag1{};

        program.on_vertex(uniform, v0, frag0);
        program.on_vertex(uniform, v1, frag1);

        // clip line so it's inside the viewing volume:
        const auto tup = renderer::line_in_frustum(frag0.pos, frag1.pos);
        if (!tup.has_value()) {
            return;
        }
        const auto [t0, t1] = tup.value();
        const Frag tfrag0 = lerp<Options.attr_interpolation>(frag0, frag1, t0);
        const Frag tfrag1 = lerp<Options.attr_interpolation>(frag0, frag1, t1);

        // perspective divide
        // clip space -> screen space:
        const PFrag pfrag0 = project_fragment(tfrag0);
        const PFrag pfrag1 = project_fragment(tfrag1);

        // scale up to viewport:
        const PFrag vfrag0 = apply_scale_to_viewport(scale_to_viewport, pfrag0);
        const PFrag vfrag1 = apply_scale_to_viewport(scale_to_viewport, pfrag1);

        PFrag inner_tfrag0 = vfrag0;
        PFrag inner_tfrag1 = vfrag1;
        if (requires_screen_clipping) {
            // clip line so it's inside the screen:
            const auto inner_tup = renderer::line_in_screen(vfrag0.pos, vfrag1.pos, SCREEN_BOUNDS);
            if (!inner_tup.has_value()) {
                return;
            }
            const auto [inner_t0, inner_t1] = inner_tup.value();
            inner_tfrag0 = lerp<Options.attr_interpolation>(vfrag0, vfrag1, inner_t0);
            inner_tfrag1 = lerp<Options.attr_interpolation>(vfrag0, vfrag1, inner_t1);
        }

        // screen space -> window space:
        const PFrag wfrag0 = apply_screen_to_window_transform(framebuffer.screen_to_window_transform(), inner_tfrag0);
        const PFrag wfrag1 = apply_screen_to_window_transform(framebuffer.screen_to_window_transform(), inner_tfrag1);

        // swap vertices after line drawing direction
        bool keep_vertex_order = true;
        switch (const auto v0v1 = wfrag0.pos.vector_to(wfrag1.pos); Options.line_drawing_direction) {
        case LineDrawingDirection::Upwards:
            keep_vertex_order = v0v1.y > 0;
            break;
        case LineDrawingDirection::Downwards:
            keep_vertex_order = v0v1.y < 0;
            break;
        case LineDrawingDirection::Leftwards:
            keep_vertex_order = v0v1.x < 0;
            break;
        case LineDrawingDirection::Rightwards:
            keep_vertex_order = v0v1.x > 0;
            break;
        }

        if constexpr (ProgramInterface_FragCoroutineSupport<Program>) {
            const auto plot_func =
                [&program, &framebuffer, &uniform](const std::array<ProjectedFragment<Varying>, 2>& rfrag,
                                                   const std::array<bool, 2>& in_line) -> void {
                const auto [rfrag0, rfrag1] = rfrag;

                using FragmentContext = typename Program::FragmentContext;
                std::array<typename FragmentContext::ValueVariant, 4> quad;
                FragmentContext c0{ 0, quad, FragmentContext::Type::LINE, !in_line[0] };
                FragmentContext c1{ 1, quad, FragmentContext::Type::LINE, !in_line[1] };

                Targets targets0 = {};
                Targets targets1 = {};

                bool discarded0 = false;
                bool discarded1 = false;

                // apply fragment shader and unpack results:
                for (const auto [r0, r1] :
                     std::ranges::views::zip(program.on_fragment(c0, uniform, rfrag0, targets0),
                                             program.on_fragment(c1, uniform, rfrag1, targets1))) {
                    if (r0 == ProgramToken::Syncronize || r1 == ProgramToken::Syncronize) {
                        if (r0 != ProgramToken::Syncronize || r1 != ProgramToken::Syncronize) {
                            throw std::logic_error("asciirast::Renderer::draw() : Fragment shader must "
                                                   "should"
                                                   "syncronize in the same order in all instances");
                        }
                    }
                    discarded0 |= r0 == ProgramToken::Discard;
                    discarded1 |= r1 == ProgramToken::Discard;

                    if (discarded0 || discarded1) {
                        break;
                    }
                }

                const auto rfrag0_pos_int = math::Vec2Int{ rfrag0.pos };
                const auto rfrag1_pos_int = math::Vec2Int{ rfrag1.pos };

                if (in_line[0] && !discarded0) {
                    if constexpr (FrameBuffer_DepthSupport<FrameBuffer>) {
                        if (framebuffer.test_and_set_depth(rfrag0_pos_int, rfrag0.depth)) {
                            framebuffer.plot(rfrag0_pos_int, targets0);
                        }
                    } else {
                        framebuffer.plot(rfrag0_pos_int, targets0);
                    }
                }
                if (in_line[1] && !discarded1) {
                    if constexpr (FrameBuffer_DepthSupport<FrameBuffer>) {
                        if (framebuffer.test_and_set_depth(rfrag1_pos_int, rfrag1.depth)) {
                            framebuffer.plot(rfrag1_pos_int, targets1);
                        }
                    } else {
                        framebuffer.plot(rfrag1_pos_int, targets1);
                    }
                }
            };

            if (keep_vertex_order) {
                renderer::rasterize_line<Options>(wfrag0, wfrag1, std::forward<decltype(plot_func)>(plot_func));
            } else {
                renderer::rasterize_line<Options>(wfrag1, wfrag0, std::forward<decltype(plot_func)>(plot_func));
            }
        } else {
            static_assert(ProgramInterface_FragRegularSupport<Program>);

            const auto plot_func = [&program, &framebuffer, &uniform](const ProjectedFragment<Varying>& rfrag) -> void {
                const auto pos_int = math::Vec2Int{ rfrag.pos };
                Targets targets = {};

                // early z testing
                if constexpr (FrameBuffer_DepthSupport<FrameBuffer>) {
                    if (framebuffer.test_and_set_depth(pos_int, rfrag.depth)) {
                        program.on_fragment(uniform, rfrag, targets);
                        framebuffer.plot(pos_int, targets);
                    }
                } else {
                    program.on_fragment(uniform, rfrag, targets);
                    framebuffer.plot(pos_int, targets);
                }
            };

            if (keep_vertex_order) {
                renderer::rasterize_line<Options>(wfrag0, wfrag1, std::forward<decltype(plot_func)>(plot_func));
            } else {
                renderer::rasterize_line<Options>(wfrag1, wfrag0, std::forward<decltype(plot_func)>(plot_func));
            }
        }
    }

    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class Vec4TripletAllocator,
             class AttrsTripletAllocator>
        requires(std::is_same_v<Vertex, typename Program::Vertex>)
    static void draw_triangle(
        const Program& program,
        const Uniform& uniform,
        const bool requires_screen_clipping,
        const math::Transform2D& scale_to_viewport,
        RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data,
        FrameBuffer& framebuffer,
        const Vertex& v0,
        const Vertex& v1,
        const Vertex& v2)
    {
        using Varying = typename Program::Varying;
        using Targets = typename Program::Targets;

        using Frag = Fragment<Varying>;
        using PFrag = ProjectedFragment<Varying>;

        const auto rasterize_triangle =
            [&program, &framebuffer, &uniform](const PFrag& wfrag0, const PFrag& wfrag1, const PFrag& wfrag2) -> void {
            constexpr bool neither_winding_order = Options.winding_order == WindingOrder::Neither;
            constexpr bool clockwise_winding_order = Options.winding_order == WindingOrder::Clockwise;
            constexpr bool cclockwise_winding_order = Options.winding_order == WindingOrder::CounterClockwise;

            const auto signed_area_2 = cross(wfrag0.pos.vector_to(wfrag1.pos), wfrag0.pos.vector_to(wfrag2.pos));
            if (signed_area_2 == 0) {
                return;
            }

            const bool backface_cull_cond = (cclockwise_winding_order && 0 < signed_area_2) || //
                                            (clockwise_winding_order && 0 > signed_area_2);
            if (!neither_winding_order && backface_cull_cond) {
                return;
            }

            if constexpr (ProgramInterface_FragCoroutineSupport<Program>) {
                const auto plot_func =
                    [&program, &framebuffer, &uniform](const std::array<ProjectedFragment<Varying>, 4>& rfrag,
                                                       const std::array<bool, 4>& in_triangle) -> void {
                    const auto [rfrag0, rfrag1, rfrag2, rfrag3] = rfrag;

                    using FragmentContext = typename Program::FragmentContext;
                    std::array<typename FragmentContext::ValueVariant, 4> quad;
                    FragmentContext c0{ 0, quad, FragmentContext::Type::FILLED, !in_triangle[0] };
                    FragmentContext c1{ 1, quad, FragmentContext::Type::FILLED, !in_triangle[1] };
                    FragmentContext c2{ 2, quad, FragmentContext::Type::FILLED, !in_triangle[2] };
                    FragmentContext c3{ 3, quad, FragmentContext::Type::FILLED, !in_triangle[3] };

                    Targets targets0 = {};
                    Targets targets1 = {};
                    Targets targets2 = {};
                    Targets targets3 = {};

                    bool discarded0 = false;
                    bool discarded1 = false;
                    bool discarded2 = false;
                    bool discarded3 = false;

                    // apply fragment shader and unpack wrapped result:
                    for (const auto [r0, r1, r2, r3] :
                         std::ranges::views::zip(program.on_fragment(c0, uniform, rfrag0, targets0),
                                                 program.on_fragment(c1, uniform, rfrag1, targets1),
                                                 program.on_fragment(c2, uniform, rfrag2, targets2),
                                                 program.on_fragment(c3, uniform, rfrag3, targets3))) {
                        if (r0 == ProgramToken::Syncronize || r1 == ProgramToken::Syncronize ||
                            r2 == ProgramToken::Syncronize || r3 == ProgramToken::Syncronize) {
                            if (r0 != ProgramToken::Syncronize || r1 != ProgramToken::Syncronize ||
                                r2 != ProgramToken::Syncronize || r3 != ProgramToken::Syncronize) {
                                throw std::logic_error("asciirast::Renderer::draw() : Fragment shader "
                                                       "must should"
                                                       "syncronize in the same order in all instances");
                            }
                        }
                        discarded0 |= r0 == ProgramToken::Discard;
                        discarded1 |= r1 == ProgramToken::Discard;
                        discarded2 |= r2 == ProgramToken::Discard;
                        discarded3 |= r3 == ProgramToken::Discard;

                        if (discarded0 || discarded1 || discarded2 || discarded3) {
                            break;
                        }
                    }
                    const auto rfrag0_pos_int = math::Vec2Int{ rfrag0.pos };
                    const auto rfrag1_pos_int = math::Vec2Int{ rfrag1.pos };
                    const auto rfrag2_pos_int = math::Vec2Int{ rfrag2.pos };
                    const auto rfrag3_pos_int = math::Vec2Int{ rfrag3.pos };

                    if (in_triangle[0] && !discarded0) {
                        if constexpr (FrameBuffer_DepthSupport<FrameBuffer>) {
                            if (framebuffer.test_and_set_depth(rfrag0_pos_int, rfrag0.depth)) {
                                framebuffer.plot(rfrag0_pos_int, targets0);
                            }
                        } else {
                            framebuffer.plot(rfrag0_pos_int, targets0);
                        }
                    }
                    if (in_triangle[1] && !discarded1) {
                        if constexpr (FrameBuffer_DepthSupport<FrameBuffer>) {
                            if (framebuffer.test_and_set_depth(rfrag1_pos_int, rfrag1.depth)) {
                                framebuffer.plot(rfrag1_pos_int, targets1);
                            }
                        } else {
                            framebuffer.plot(rfrag1_pos_int, targets1);
                        }
                    }
                    if (in_triangle[2] && !discarded2) {
                        if constexpr (FrameBuffer_DepthSupport<FrameBuffer>) {
                            if (framebuffer.test_and_set_depth(rfrag2_pos_int, rfrag2.depth)) {
                                framebuffer.plot(rfrag2_pos_int, targets2);
                            }
                        } else {
                            framebuffer.plot(rfrag2_pos_int, targets2);
                        }
                    }
                    if (in_triangle[3] && !discarded3) {
                        if constexpr (FrameBuffer_DepthSupport<FrameBuffer>) {
                            if (framebuffer.test_and_set_depth(rfrag3_pos_int, rfrag3.depth)) {
                                framebuffer.plot(rfrag3_pos_int, targets3);
                            }
                        } else {
                            framebuffer.plot(rfrag3_pos_int, targets3);
                        }
                    }
                };

                if (signed_area_2 > 0) {
                    renderer::rasterize_triangle<Options>(
                        wfrag0, wfrag1, wfrag2, std::forward<decltype(plot_func)>(plot_func));
                } else {
                    renderer::rasterize_triangle<Options>(
                        wfrag0, wfrag2, wfrag1, std::forward<decltype(plot_func)>(plot_func));
                }
            } else {
                static_assert(ProgramInterface_FragRegularSupport<Program>);

                const auto plot_func =
                    [&program, &framebuffer, &uniform](const ProjectedFragment<Varying>& rfrag) -> void {
                    const auto pos_int = math::Vec2Int{ rfrag.pos };
                    Targets targets = {};

                    // early z testing
                    if constexpr (FrameBuffer_DepthSupport<FrameBuffer>) {
                        if (framebuffer.test_and_set_depth(pos_int, rfrag.depth)) {
                            program.on_fragment(uniform, rfrag, targets);
                            framebuffer.plot(pos_int, targets);
                        }
                    } else {
                        program.on_fragment(uniform, rfrag, targets);
                        framebuffer.plot(pos_int, targets);
                    }
                };

                if (signed_area_2 > 0) {
                    renderer::rasterize_triangle<Options>(
                        wfrag0, wfrag1, wfrag2, std::forward<decltype(plot_func)>(plot_func));
                } else {
                    renderer::rasterize_triangle<Options>(
                        wfrag0, wfrag2, wfrag1, std::forward<decltype(plot_func)>(plot_func));
                }
            }
        };

        // apply vertex shader
        // model space -> world space -> view space -> clip space:
        Frag frag0{};
        Frag frag1{};
        Frag frag2{};

        program.on_vertex(uniform, v0, frag0);
        program.on_vertex(uniform, v1, frag1);
        program.on_vertex(uniform, v2, frag2);

        data.vec_queue.clear();
        data.attrs_queue.clear();
        data.vec_queue.insert(data.vec_queue.end(),
                              renderer::Vec4Triplet{
                                  frag0.pos,
                                  frag1.pos,
                                  frag2.pos,
                              });
        data.attrs_queue.insert(data.attrs_queue.end(),
                                renderer::AttrsTriplet<Varying>{
                                    frag0.attrs,
                                    frag1.attrs,
                                    frag2.attrs,
                                });

        // clip triangle so it's inside the viewing volume:
        if (!renderer::triangle_in_frustum<Options.attr_interpolation>(data.vec_queue, data.attrs_queue)) {
            return;
        }
        for (const auto& [vec_triplet, attrs_triplet] : std::ranges::views::zip(data.vec_queue, data.attrs_queue)) {
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
            const PFrag vfrag0 = apply_scale_to_viewport(scale_to_viewport, pfrag0);
            const PFrag vfrag1 = apply_scale_to_viewport(scale_to_viewport, pfrag1);
            const PFrag vfrag2 = apply_scale_to_viewport(scale_to_viewport, pfrag2);

            if (!requires_screen_clipping) {
                // screen space -> window space:
                const PFrag wfrag0 = apply_screen_to_window_transform(framebuffer.screen_to_window_transform(), vfrag0);
                const PFrag wfrag1 = apply_screen_to_window_transform(framebuffer.screen_to_window_transform(), vfrag1);
                const PFrag wfrag2 = apply_screen_to_window_transform(framebuffer.screen_to_window_transform(), vfrag2);

                // iterate over triangle fragments:
                rasterize_triangle(wfrag0, wfrag1, wfrag2);

                continue;
            }

            data.vec_queue_screen.clear();
            data.attrs_queue_screen.clear();

            const auto p0 = math::Vec4{ vfrag0.pos, vfrag0.depth, vfrag0.Z_inv };
            const auto p1 = math::Vec4{ vfrag1.pos, vfrag1.depth, vfrag1.Z_inv };
            const auto p2 = math::Vec4{ vfrag2.pos, vfrag2.depth, vfrag2.Z_inv };
            const auto [a0, a1, a2] = renderer::AttrsTriplet<Varying>{ vfrag0.attrs, vfrag1.attrs, vfrag2.attrs };
            const auto lp = renderer::Vec4Triplet{ p0, p1, p2 };
            const auto la = renderer::AttrsTriplet<Varying>{ a0, a1, a2 };

            data.vec_queue_screen.insert(data.vec_queue_screen.end(), lp);
            data.attrs_queue_screen.insert(data.attrs_queue_screen.end(), la);

            // clip line so it's inside the screen:
            if (!renderer::triangle_in_screen<Options.attr_interpolation>(
                    data.vec_queue_screen, data.attrs_queue_screen, SCREEN_BOUNDS)) {
                continue;
            }
            for (const auto& [inner_vec_triplet, inner_attrs_triplet] :
                 std::ranges::views::zip(data.vec_queue_screen, data.attrs_queue_screen)) {
                const auto [inner_vec0, inner_vec1, inner_vec2] = inner_vec_triplet;
                const auto [inner_attrs0, inner_attrs1, inner_attrs2] = inner_attrs_triplet;

                const PFrag inner_tfrag0 = { inner_vec0.xy, inner_vec0.z, inner_vec0.w, inner_attrs0 };
                const PFrag inner_tfrag1 = { inner_vec1.xy, inner_vec1.z, inner_vec1.w, inner_attrs1 };
                const PFrag inner_tfrag2 = { inner_vec2.xy, inner_vec2.z, inner_vec2.w, inner_attrs2 };

                // screen space -> window space:
                const PFrag wfrag0 =
                    apply_screen_to_window_transform(framebuffer.screen_to_window_transform(), inner_tfrag0);
                const PFrag wfrag1 =
                    apply_screen_to_window_transform(framebuffer.screen_to_window_transform(), inner_tfrag1);
                const PFrag wfrag2 =
                    apply_screen_to_window_transform(framebuffer.screen_to_window_transform(), inner_tfrag2);

                // iterate over triangle fragments:
                rasterize_triangle(wfrag0, wfrag1, wfrag2);
            }
        }
    }
};

}; // namespace asciirast
