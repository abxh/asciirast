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
#include <stdexcept>
#include <variant>
#include <vector>

#include "./math/types.h"
#include "./program.h"
#include "./rasterize/bounds_test.h"
#include "./rasterize/interpolate.h"
#include "./rasterize/rasterize.h"

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
 * @brief Triangle winding order
 */
enum class WindingOrder
{
    Clockwise,
    CounterClockwise,
    Neither,
};

/**
 * @brief Line drawing direction
 */
enum class LineDrawingDirection
{
    Upwards,
    Downwards,
    Leftwards,
    Rightwards,
};

/**
 * @brief Line ends inclusion
 */
enum class LineEndsInclusion
{
    ExcludeBoth,
    IncludeStart,
    IncludeEnd,
    IncludeBoth,
};

/**
 * @brief Triangle fill bias
 */
enum class TriangleFillBias
{
    TopLeft,
    BottomRight,
    Neither,
};

/**
 * @brief Renderer options
 */
struct RendererOptions
{
    WindingOrder winding_order = WindingOrder::Neither; ///< triangle winding order

    TriangleFillBias triangle_fill_bias = TriangleFillBias::TopLeft; ///< triangle fill bias

    LineDrawingDirection line_drawing_direction = LineDrawingDirection::Downwards; ///< line drawing direction

    LineEndsInclusion line_ends_inclusion = LineEndsInclusion::IncludeBoth; ///< line ends inclusion option
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
         typename Vec4TripletAllocator = std::allocator<rasterize::Vec4Triplet>,
         typename AttrsTripletAllocator = std::allocator<rasterize::AttrsTriplet<Varying>>>
class RendererData
{
    using Vec4TripletQueue = std::deque<rasterize::Vec4Triplet, Vec4TripletAllocator>;             ///< Vec4 queue
    using AttrsTripletQueue = std::deque<rasterize::AttrsTriplet<Varying>, AttrsTripletAllocator>; ///< Attrs queue

    Vec4TripletQueue m_vec_queue = {};          ///< Vec4 queue
    AttrsTripletQueue m_attrs_queue = {};       ///< Attrs queue
    Vec4TripletQueue m_inner_vec_queue = {};    ///< Vec4 queue used in inner loops
    AttrsTripletQueue m_inner_attrs_queue = {}; ///< Attrs queue used in inner loops

    friend class Renderer;

public:
    math::Transform2D screen_to_window; ///< Screen to window transform

    /**
     * @brief Construct renderer data
     *
     * @param screen_to_window_ The screen to window transform to use
     */
    explicit RendererData(const math::Transform2D& screen_to_window_) noexcept
            : screen_to_window{ screen_to_window_ } {};
};

/**
 * @brief Renderer class
 */
class Renderer
{
    bool m_requires_screen_clipping = false;
    math::Transform2D m_screen_to_viewport = {};

public:
    /**
     * @brief Screen boundary
     *
     * Verticies outside of this boundary are not shown.
     */
    static constexpr auto SCREEN_BOUNDS = math::AABB2D::from_min_max({ -1, -1 }, { +1, +1 });

    /**
     * @brief Calculate the transform to convert screen_bounds points to viewport_bounds poins
     *
     * @param viewport_bounds The viewport bounds AABB
     * @param screen_bounds The screen bounds AABB
     * @return The transform that converts points from the screen to the viewport
     */
    static math::Transform2D screen_to_viewport_transform(const math::AABB2D& viewport_bounds,
                                                          const math::AABB2D& screen_bounds)
    {
        assert(viewport_bounds.size_get().x != 0 && "non-zero size");
        assert(viewport_bounds.size_get().y != 0 && "non-zero size");

        const auto rel_size = viewport_bounds.size_get() / screen_bounds.size_get();
        const auto shf_size = (screen_bounds.min_get() * rel_size).vector_to(viewport_bounds.min_get());

        return math::Transform2D().scale(rel_size).translate(shf_size);
    }

    /**
     * @brief Construct renderer with default viewport
     */
    Renderer() = default;

    /**
     * @brief Construct renderer with custom viewport
     *
     * The portion of the viewport in the screen is shown.
     * If the viewport is differently sized, the result is scaled accordingly.
     *
     * @param viewport_bounds The bounds of the viewport
     */
    explicit Renderer(const math::AABB2D& viewport_bounds) noexcept
            : m_requires_screen_clipping{ !SCREEN_BOUNDS.contains(viewport_bounds) }
            , m_screen_to_viewport{ screen_to_viewport_transform(viewport_bounds, SCREEN_BOUNDS) } {};

    /**
     * @brief Draw on a framebuffer using a program given uniform(s),
     * verticies, options and reusable data buffers
     *
     * @throws std::logic_error Fragment result related errors
     *
     * @param program The shader program
     * @param uniform The uniform(s)
     * @param verts The vertex buffer
     * @param framebuffer The frame buffer
     * @param data Renderer data
     * @param options Renderer Options
     */
    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class VertexAllocator,
             class Vec4TripletAllocator,
             class AttrsTripletAllocator>
        requires(detail::can_use_program_with<Program, Uniform, Vertex, FrameBuffer>::value)
    void draw(const Program& program,
              const Uniform& uniform,
              const VertexBuffer<Vertex, VertexAllocator>& verts,
              FrameBuffer& framebuffer,
              RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data,
              RendererOptions options = {}) const
    {
        draw(program, uniform, verts.shape_type, std::views::all(verts.verticies), framebuffer, data, options);
    }

    /**
     * @brief Draw on a framebuffer using a program given uniform(s),
     * indexed verticies, renderer data and options
     *
     * @throws std::runtime_error When the indicies are out of bounds
     * @throws std::logic_error Fragment result related errors
     *
     * @param program The shader program
     * @param uniform The uniform(s)
     * @param verts The vertex buffer with indicies
     * @param framebuffer The frame buffer
     * @param data Renderer data
     * @param options Renderer options
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
    void draw(const Program& program,
              const Uniform& uniform,
              const IndexedVertexBuffer<Vertex, VertexAllocator, IndexAllocator>& verts,
              FrameBuffer& framebuffer,
              RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data,
              RendererOptions options = {}) const
    {
        const auto func = [&verts](const std::size_t i) -> Vertex {
            if (i >= verts.verticies.size()) {
                throw std::runtime_error("asciirast::Renderer::draw() : vertex index is out of bounds");
            }
            return verts.verticies[i];
        };
        const auto view = std::ranges::views::transform(std::views::all(verts.indicies), func);

        draw(program, uniform, verts.shape_type, view, framebuffer, data, options);
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
              RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data,
              const RendererOptions& options) const
    {
        using Vertex = typename Program::Vertex;

        switch (shape_type) {
        case ShapeType::Points: {
            for (const Vertex& vert : verticies_inp) {
                draw_point(program, uniform, framebuffer, data, vert);
            }
        } break;
        case ShapeType::Lines: {
            const auto func = [](auto&& range) -> std::tuple<Vertex, Vertex> {
                return std::make_tuple(*range.cbegin(), *(range.cbegin() + 1U));
            };
            const auto rem = std::ranges::distance(verticies_inp) % 2U;
            const auto subrange = verticies_inp | std::ranges::views::take(std::ranges::distance(verticies_inp) - rem);
            const auto verticies = subrange | std::ranges::views::chunk(2U) | std::ranges::views::transform(func);

            for (const auto& [v0, v1] : verticies) {
                draw_line(program, uniform, framebuffer, data, options, v0, v1);
            }
        } break;
        case ShapeType::LineStrip: {
            const auto verticies = verticies_inp | std::ranges::views::adjacent<2U>;

            for (const auto& [v0, v1] : verticies) {
                draw_line(program, uniform, framebuffer, data, options, v0, v1);
            }
        } break;
        case ShapeType::LineLoop: {
            const auto verticies = verticies_inp | std::ranges::views::adjacent<2U>;

            for (const auto& [v0, v1] : verticies) {
                draw_line(program, uniform, framebuffer, data, options, v0, v1);
            }
            if (std::ranges::distance(verticies) >= 1U) {
                const auto v0 = std::get<1>(*(verticies.cend() - 1U));
                const auto v1 = std::get<0>(*verticies.cbegin());

                draw_line(program, uniform, framebuffer, data, options, v0, v1);
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
                draw_triangle(program, uniform, framebuffer, data, options, v0, v1, v2);
            }
        } break;
        case ShapeType::TriangleStrip: {
            const auto verticies = verticies_inp | std::ranges::views::adjacent<3U>;

            for (const auto& [v0, v1, v2] : verticies) {
                draw_triangle(program, uniform, framebuffer, data, options, v0, v1, v2);
            }
        } break;
        case ShapeType::TriangleFan: {
            const auto verticies = verticies_inp | std::ranges::views::adjacent<3U>;

            for (const auto& [v0, v1, v2] : verticies) {
                draw_triangle(program, uniform, framebuffer, data, options, v0, v1, v2);
            }
            if (std::ranges::distance(verticies) >= 1U) {
                const auto v0 = std::get<1>(*(verticies.cend() - 1U));
                const auto v1 = std::get<2>(*(verticies.cend() - 1U));
                const auto v2 = std::get<0>(*verticies.cbegin());

                draw_triangle(program, uniform, framebuffer, data, options, v0, v1, v2);
            };
        } break;
        }
    }

    template<VaryingInterface Varying>
    ProjectedFragment<Varying> apply_scale_to_viewport(const ProjectedFragment<Varying>& pfrag) const
    {
        return ProjectedFragment<Varying>{ .pos = m_screen_to_viewport.apply(pfrag.pos),
                                           .depth = pfrag.depth,
                                           .Z_inv = pfrag.Z_inv,
                                           .attrs = pfrag.attrs };
    }

    template<VaryingInterface Varying, class Vec4TripletAllocator, class AttrsTripletAllocator>
    ProjectedFragment<Varying> apply_screen_to_window(
            const ProjectedFragment<Varying>& pfrag,
            const RendererData<Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data) const
    {
        const auto new_pos = data.screen_to_window.apply(pfrag.pos);

        return ProjectedFragment<Varying>{ .pos = math::trunc(new_pos + math::Vec2{ 0.5f, 0.5f }),
                                           .depth = pfrag.depth,
                                           .Z_inv = pfrag.Z_inv,
                                           .attrs = pfrag.attrs };
    }

    rasterize::TriangleFillBias conv(const TriangleFillBias& b) const
    {
        switch (b) {
        case TriangleFillBias::BottomRight:
            return rasterize::TriangleFillBias::BottomRight;
        case TriangleFillBias::TopLeft:
            return rasterize::TriangleFillBias::TopLeft;
        case TriangleFillBias::Neither:
            return rasterize::TriangleFillBias::Neither;
        }
        return rasterize::TriangleFillBias::Neither;
    }

    rasterize::LineEndsInclusion conv(const LineEndsInclusion& b) const
    {
        switch (b) {
        case LineEndsInclusion::ExcludeBoth:
            return rasterize::LineEndsInclusion::ExcludeBoth;
        case LineEndsInclusion::IncludeStart:
            return rasterize::LineEndsInclusion::IncludeStart;
        case LineEndsInclusion::IncludeEnd:
            return rasterize::LineEndsInclusion::IncludeEnd;
        case LineEndsInclusion::IncludeBoth:
            return rasterize::LineEndsInclusion::IncludeBoth;
        }
        return rasterize::LineEndsInclusion::IncludeBoth;
    }

    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class Vec4TripletAllocator,
             class AttrsTripletAllocator>
        requires(std::is_same_v<Vertex, typename Program::Vertex>)
    void draw_point(const Program& program,
                    const Uniform& uniform,
                    FrameBuffer& framebuffer,
                    const RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data,
                    const Vertex& vert) const
    {
        using Varying = typename Program::Varying;
        using Targets = typename Program::Targets;
        using FragmentContext = typename Program::FragmentContext;

        using Frag = Fragment<Varying>;
        using PFrag = ProjectedFragment<Varying>;
        using FragResult = FragmentResult<Targets>;

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
            if (!rasterize::point_in_screen(vfrag.pos, SCREEN_BOUNDS)) {
                return;
            }
        }

        // screen space -> window space:
        const PFrag wfrag = apply_screen_to_window(vfrag, data);

        // create empty fragment context
        std::array<typename FragmentContext::ValueVariant, 4> quad;
        auto context = FragmentContext{ 0, &quad[0] };

        // apply fragment shader and unpack results:
        for (const auto& r : program.on_fragment(context, uniform, wfrag)) {
            const bool holds_prepare = std::holds_alternative<typename FragResult::FragmentContextPrepare>(r.m_value);
            const bool holds_targets = std::holds_alternative<Targets>(r.m_value);

            if (holds_prepare) {
                context.m_type = FragmentContext::Type::POINT;
                continue; // for consistency with other kinds of shapes
            } else if (holds_targets) {
                const auto pos_int = math::Vec2Int{ wfrag.pos };

                if (framebuffer.test_and_set_depth(pos_int, wfrag.depth)) {
                    framebuffer.plot(pos_int, std::get<Targets>(r.m_value));
                }
                break; // do nothing special but plot point in framebuffer
            } else {
                break; // discard point
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
    void draw_line(const Program& program,
                   const Uniform& uniform,
                   FrameBuffer& framebuffer,
                   const RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data,
                   const RendererOptions& options,
                   const Vertex& v0,
                   const Vertex& v1) const
    {
        using Varying = typename Program::Varying;
        using Targets = typename Program::Targets;
        using FragmentContext = typename Program::FragmentContext;

        using Frag = Fragment<Varying>;
        using PFrag = ProjectedFragment<Varying>;
        using FragResult = FragmentResult<Targets>;

        const auto plot_func =
                [&program, &framebuffer, &uniform](const std::array<ProjectedFragment<Varying>, 2>& rfrag,
                                                   const std::array<bool, 2>& in_line) -> void {
            const auto [rfrag0, rfrag1] = rfrag;

            // stop here if all pixels are helper invocations:
            if (!in_line[0] && !in_line[1]) {
                return;
            }

            std::array<typename FragmentContext::ValueVariant, 4> quad;
            FragmentContext c0{ 0, &quad[0], !in_line[0] };
            FragmentContext c1{ 1, &quad[0], !in_line[1] };

            // apply fragment shader and unpack results:
            for (const auto& [r0, r1] : std::ranges::views::zip(program.on_fragment(c0, uniform, rfrag0),
                                                                program.on_fragment(c1, uniform, rfrag1))) {
                const bool holds_targets0 = std::holds_alternative<Targets>(r0.m_value);
                const bool holds_targets1 = std::holds_alternative<Targets>(r1.m_value);

                const bool holds_prepare0 =
                        std::holds_alternative<typename FragResult::FragmentContextPrepare>(r0.m_value);
                const bool holds_prepare1 =
                        std::holds_alternative<typename FragResult::FragmentContextPrepare>(r1.m_value);

                if (holds_prepare0 || holds_prepare1) {
                    if (!holds_prepare0 || !holds_prepare1) {
                        throw std::logic_error("asciirast::Renderer::draw() : Fragment shader must should"
                                               "prepare context in the same order in all instances");
                    }
                    c0.m_type = FragmentContext::Type::LINE;
                    c1.m_type = FragmentContext::Type::LINE;
                    continue; // prepare fragment contexts concurrently
                } else if (holds_targets0 || holds_targets1) {
                    const auto rfrag0_pos_int = math::Vec2Int{ rfrag0.pos };
                    const auto rfrag1_pos_int = math::Vec2Int{ rfrag1.pos };

                    if (in_line[0] && holds_targets0 && framebuffer.test_and_set_depth(rfrag0_pos_int, rfrag0.depth)) {
                        framebuffer.plot(rfrag0_pos_int, std::get<Targets>(r0.m_value));
                    }
                    if (in_line[1] && holds_targets1 && framebuffer.test_and_set_depth(rfrag1_pos_int, rfrag1.depth)) {
                        framebuffer.plot(rfrag1_pos_int, std::get<Targets>(r1.m_value));
                    }
                    break; // do nothing special but plot the point(s) in the framebuffer
                } else {
                    break; // all points discarded
                }
            }
        };
        const auto bias = conv(options.line_ends_inclusion);
        const auto rasterize_line = [plot_func, &options, bias](const PFrag& wfrag0, const PFrag& wfrag1) -> void {
            bool keep = true;
            const auto v0v1 = wfrag0.pos.vector_to(wfrag1.pos);

            switch (options.line_drawing_direction) {
            case LineDrawingDirection::Upwards:
                keep = v0v1.y > 0;
                break;
            case LineDrawingDirection::Downwards:
                keep = v0v1.y < 0;
                break;
            case LineDrawingDirection::Leftwards:
                keep = v0v1.x < 0;
                break;
            case LineDrawingDirection::Rightwards:
                keep = v0v1.x > 0;
                break;
            }

            // swap vertices after line drawing direction
            if (keep) {
                rasterize::rasterize_line(wfrag0, wfrag1, plot_func, bias);
            } else {
                rasterize::rasterize_line(wfrag1, wfrag0, plot_func, bias);
            }
        };

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
            const auto inner_tup = rasterize::line_in_screen(vfrag0.pos, vfrag1.pos, SCREEN_BOUNDS);
            if (!inner_tup.has_value()) {
                return;
            }
            const auto [inner_t0, inner_t1] = inner_tup.value();
            inner_tfrag0 = rasterize::lerp(vfrag0, vfrag1, inner_t0);
            inner_tfrag1 = rasterize::lerp(vfrag0, vfrag1, inner_t1);
        }

        // screen space -> window space:
        const PFrag wfrag0 = apply_screen_to_window(inner_tfrag0, data);
        const PFrag wfrag1 = apply_screen_to_window(inner_tfrag1, data);

        // iterate over line fragments:
        rasterize_line(wfrag0, wfrag1);
    }

    template<ProgramInterface Program,
             class Uniform,
             class Vertex,
             FrameBufferInterface FrameBuffer,
             class Vec4TripletAllocator,
             class AttrsTripletAllocator>
        requires(std::is_same_v<Vertex, typename Program::Vertex>)
    void draw_triangle(const Program& program,
                       const Uniform& uniform,
                       FrameBuffer& framebuffer,
                       RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data,
                       const RendererOptions& options,
                       const Vertex& v0,
                       const Vertex& v1,
                       const Vertex& v2) const
    {
        using Varying = typename Program::Varying;
        using Targets = typename Program::Targets;
        using FragmentContext = typename Program::FragmentContext;

        using Frag = Fragment<Varying>;
        using PFrag = ProjectedFragment<Varying>;
        using FragResult = FragmentResult<Targets>;

        const auto plot_func =
                [&program, &framebuffer, &uniform](const std::array<ProjectedFragment<Varying>, 4>& rfrag,
                                                   const std::array<bool, 4>& in_triangle) -> void {
            const auto [rfrag0, rfrag1, rfrag2, rfrag3] = rfrag;

            // stop here if all pixels are helper invocations:
            if (!in_triangle[0] && !in_triangle[1] && !in_triangle[2] && !in_triangle[3]) {
                return;
            }

            std::array<typename FragmentContext::ValueVariant, 4> quad;
            FragmentContext c0{ 0, &quad[0], !in_triangle[0] };
            FragmentContext c1{ 1, &quad[0], !in_triangle[1] };
            FragmentContext c2{ 2, &quad[0], !in_triangle[2] };
            FragmentContext c3{ 3, &quad[0], !in_triangle[3] };

            // apply fragment shader and unpack wrapped result:
            for (const auto& [r0, r1, r2, r3] : std::ranges::views::zip(program.on_fragment(c0, uniform, rfrag0),
                                                                        program.on_fragment(c1, uniform, rfrag1),
                                                                        program.on_fragment(c2, uniform, rfrag2),
                                                                        program.on_fragment(c3, uniform, rfrag3))) {
                const bool holds_targets0 = std::holds_alternative<Targets>(r0.m_value);
                const bool holds_targets1 = std::holds_alternative<Targets>(r1.m_value);
                const bool holds_targets2 = std::holds_alternative<Targets>(r2.m_value);
                const bool holds_targets3 = std::holds_alternative<Targets>(r3.m_value);

                const bool holds_prepare0 =
                        std::holds_alternative<typename FragResult::FragmentContextPrepare>(r0.m_value);
                const bool holds_prepare1 =
                        std::holds_alternative<typename FragResult::FragmentContextPrepare>(r1.m_value);
                const bool holds_prepare2 =
                        std::holds_alternative<typename FragResult::FragmentContextPrepare>(r2.m_value);
                const bool holds_prepare3 =
                        std::holds_alternative<typename FragResult::FragmentContextPrepare>(r3.m_value);

                if (holds_prepare0 || holds_prepare1 || holds_prepare2 || holds_prepare3) {
                    if (!holds_prepare0 || !holds_prepare1 || !holds_prepare2 || !holds_prepare3) {
                        throw std::logic_error("asciirast::Renderer::draw() : Fragment shader must should"
                                               "prepare context in the same order in all instances");
                    }
                    c0.m_type = FragmentContext::Type::FILLED;
                    c1.m_type = FragmentContext::Type::FILLED;
                    c2.m_type = FragmentContext::Type::FILLED;
                    c3.m_type = FragmentContext::Type::FILLED;
                    continue; // prepare fragment contexts concurrently
                } else if (holds_targets0 || holds_targets1 || holds_targets2 || holds_prepare3) {
                    const auto rfrag0_pos_int = math::Vec2Int{ rfrag0.pos };
                    const auto rfrag1_pos_int = math::Vec2Int{ rfrag1.pos };
                    const auto rfrag2_pos_int = math::Vec2Int{ rfrag2.pos };
                    const auto rfrag3_pos_int = math::Vec2Int{ rfrag3.pos };

                    if (in_triangle[0] && holds_targets0 &&
                        framebuffer.test_and_set_depth(rfrag0_pos_int, rfrag0.depth)) {
                        framebuffer.plot(rfrag0_pos_int, std::get<Targets>(r0.m_value));
                    }
                    if (in_triangle[1] && holds_targets1 &&
                        framebuffer.test_and_set_depth(rfrag1_pos_int, rfrag1.depth)) {
                        framebuffer.plot(rfrag1_pos_int, std::get<Targets>(r1.m_value));
                    }
                    if (in_triangle[2] && holds_targets2 &&
                        framebuffer.test_and_set_depth(rfrag2_pos_int, rfrag2.depth)) {
                        framebuffer.plot(rfrag2_pos_int, std::get<Targets>(r2.m_value));
                    }
                    if (in_triangle[3] && holds_targets3 &&
                        framebuffer.test_and_set_depth(rfrag3_pos_int, rfrag3.depth)) {
                        framebuffer.plot(rfrag3_pos_int, std::get<Targets>(r3.m_value));
                    }
                    break; // do nothing special but plot the point(s) in the framebuffer
                } else {
                    break; // both points discarded
                }
            }
        };
        const auto bias = conv(options.triangle_fill_bias);
        const auto rasterize_triangle =
                [plot_func, &options, bias](const PFrag& wfrag0, const PFrag& wfrag1, const PFrag& wfrag2) -> void {
            const bool clockwise_winding_order = options.winding_order == WindingOrder::Clockwise;
            const bool cclockwise_winding_order = options.winding_order == WindingOrder::CounterClockwise;
            const bool neither_winding_order = options.winding_order == WindingOrder::Neither;

            // perform backface culling:
            const auto p0p2 = wfrag0.pos.vector_to(wfrag2.pos);
            const auto p0p1 = wfrag0.pos.vector_to(wfrag1.pos);
            const auto signed_area_2 = math::cross(p0p2, p0p1);
            const bool backface_cull_cond =
                    !neither_winding_order && ((clockwise_winding_order && 0 < signed_area_2) || //
                                               (cclockwise_winding_order && 0 > signed_area_2));
            if (backface_cull_cond) {
                return;
            }

            // swap vertices after flexible winding order, and iterate over triangle fragments:
            if (clockwise_winding_order || (neither_winding_order && 0 > signed_area_2)) {
                rasterize::rasterize_triangle(wfrag0, wfrag1, wfrag2, plot_func, bias);
            } else {
                rasterize::rasterize_triangle(wfrag0, wfrag2, wfrag1, plot_func, bias);
            }
        };

        // apply vertex shader
        // model space -> world space -> view space -> clip space:
        const Frag frag0 = program.on_vertex(uniform, v0);
        const Frag frag1 = program.on_vertex(uniform, v1);
        const Frag frag2 = program.on_vertex(uniform, v2);

        data.m_vec_queue.clear();
        data.m_attrs_queue.clear();
        data.m_vec_queue.insert(data.m_vec_queue.end(), rasterize::Vec4Triplet{ frag0.pos, frag1.pos, frag2.pos });
        data.m_attrs_queue.insert(data.m_attrs_queue.end(),
                                  rasterize::AttrsTriplet<Varying>{ frag0.attrs, frag1.attrs, frag2.attrs });

        // clip triangle so it's inside the viewing volume:
        if (!rasterize::triangle_in_frustum(data.m_vec_queue, data.m_attrs_queue)) {
            return;
        }
        for (const auto& [vec_triplet, attrs_triplet] : std::ranges::views::zip(data.m_vec_queue, data.m_attrs_queue)) {

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
                const PFrag wfrag0 = apply_screen_to_window(vfrag0, data);
                const PFrag wfrag1 = apply_screen_to_window(vfrag1, data);
                const PFrag wfrag2 = apply_screen_to_window(vfrag2, data);

                // iterate over triangle fragments:
                rasterize_triangle(wfrag0, wfrag1, wfrag2);

                continue;
            }

            data.m_inner_vec_queue.clear();
            data.m_inner_attrs_queue.clear();

            const auto p0 = math::Vec4{ vfrag0.pos, vfrag0.depth, vfrag0.Z_inv };
            const auto p1 = math::Vec4{ vfrag1.pos, vfrag1.depth, vfrag1.Z_inv };
            const auto p2 = math::Vec4{ vfrag2.pos, vfrag2.depth, vfrag2.Z_inv };
            const auto [a0, a1, a2] = rasterize::AttrsTriplet<Varying>{ vfrag0.attrs, vfrag1.attrs, vfrag2.attrs };
            const auto lp = rasterize::Vec4Triplet{ p0, p1, p2 };
            const auto la = rasterize::AttrsTriplet<Varying>{ a0, a1, a2 };

            data.m_inner_vec_queue.insert(data.m_inner_vec_queue.end(), lp);
            data.m_inner_attrs_queue.insert(data.m_inner_attrs_queue.end(), la);

            // clip line so it's inside the screen:
            if (!rasterize::triangle_in_screen(data.m_inner_vec_queue, data.m_inner_attrs_queue, SCREEN_BOUNDS)) {
                continue;
            }
            for (const auto& [inner_vec_triplet, inner_attrs_triplet] :
                 std::ranges::views::zip(data.m_inner_vec_queue, data.m_inner_attrs_queue)) {

                const auto [inner_vec0, inner_vec1, inner_vec2] = inner_vec_triplet;
                const auto [inner_attrs0, inner_attrs1, inner_attrs2] = inner_attrs_triplet;

                const PFrag inner_tfrag0 = { inner_vec0.xy, inner_vec0.z, inner_vec0.w, inner_attrs0 };
                const PFrag inner_tfrag1 = { inner_vec1.xy, inner_vec1.z, inner_vec1.w, inner_attrs1 };
                const PFrag inner_tfrag2 = { inner_vec2.xy, inner_vec2.z, inner_vec2.w, inner_attrs2 };

                // screen space -> window space:
                const PFrag wfrag0 = apply_screen_to_window(inner_tfrag0, data);
                const PFrag wfrag1 = apply_screen_to_window(inner_tfrag1, data);
                const PFrag wfrag2 = apply_screen_to_window(inner_tfrag2, data);

                // iterate over triangle fragments:
                rasterize_triangle(wfrag0, wfrag1, wfrag2);
            }
        }
    }
};
}; // namespace asciirast
