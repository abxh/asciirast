/**
 * @file renderer.h
 * @brief The renderer class and related types
 */

#pragma once

#include <cassert>
#include <deque>
#include <ranges>
#include <stdexcept>
#include <vector>

#include "./math/types.h"
#include "./program.h"
#include "./renderer/interpolate.h"
#include "./renderer/rasterize.h"
#include "./renderer/test_bounds.h"

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
class RendererData
{
    using Vec4TripletQueue = std::deque<std::array<math::Vec4, 3>, Vec4TripletAllocator>;
    using AttrsTripletQueue = std::deque<std::array<Varying, 3>, AttrsTripletAllocator>;

    Vec4TripletQueue m_vec_queue0 = {};
    Vec4TripletQueue m_vec_queue1 = {};

    AttrsTripletQueue m_attrs_queue0 = {};
    AttrsTripletQueue m_attrs_queue1 = {};

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
    math::Transform2D m_scale_to_viewport = {};

public:
    /**
     * @brief Screen boundary
     *
     * Verticies outside of this boundary are not shown.
     */
    static constexpr auto SCREEN_BOUNDS = math::AABB2D::from_min_max({ -1, -1 }, { +1, +1 });

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
            , m_scale_to_viewport{ scale_to_viewport_transform(viewport_bounds, SCREEN_BOUNDS) } {};

    /**
     * @brief Get scale to viewport transform
     *
     * @return Const reference to the scale to viewport transform
     */
    const math::Transform2D& scale_to_viewport() const { return m_scale_to_viewport; }

    /**
     * @brief Draw on a framebuffer using a program given uniform(s),
     * verticies, options and reusable data buffers
     *
     * @throws std::logic_error If fragments do not syncronize in the same order
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
     * @throws std::runtime_error If the vertex indicies are out of bounds
     * @throws std::logic_error If fragments do not syncronize in the same order
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

    /**
     * @brief Calculate the transform to convert screen_bounds points to viewport_bounds poins
     *
     * @param viewport_bounds The viewport bounds AABB
     * @param screen_bounds The screen bounds AABB
     * @return The transform that converts points from the screen to the viewport
     */
    static auto scale_to_viewport_transform(const math::AABB2D& viewport_bounds, const math::AABB2D& screen_bounds)
            -> math::Transform2D
    {
        assert(viewport_bounds.size_get().x != 0 && "non-zero size");
        assert(viewport_bounds.size_get().y != 0 && "non-zero size");

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
    static auto apply_screen_to_window(const math::Transform2D& screen_to_window, ProjectedFragment<Varying> frag)
            -> ProjectedFragment<Varying>
    {
        frag.pos = math::round(screen_to_window.apply(frag.pos));
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
              RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data,
              const RendererOptions& options) const
    {
        using Vertex = typename Program::Vertex;

        const auto draw_point_func = [&](const Vertex& vert) -> void {
            draw_point(program,
                       uniform,
                       m_requires_screen_clipping,
                       m_scale_to_viewport,
                       data.screen_to_window,
                       framebuffer,
                       vert);
        };
        const auto draw_line_func = [&](const Vertex& v0, const Vertex& v1) -> void {
            draw_line(program,
                      uniform,
                      m_requires_screen_clipping,
                      m_scale_to_viewport,
                      data.screen_to_window,
                      options,
                      framebuffer,
                      v0,
                      v1);
        };
        const auto draw_triangle_func = [&](const Vertex& v0, const Vertex& v1, const Vertex& v2) -> void {
            draw_triangle(program,
                          uniform,
                          m_requires_screen_clipping,
                          m_scale_to_viewport,
                          data.screen_to_window,
                          options,
                          data,
                          framebuffer,
                          v0,
                          v1,
                          v2);
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
            const auto verticies_tup = verticies_inp | std::ranges::views::adjacent<3U>;

            for (const auto& [v0, v1, v2] : verticies_tup) {
                draw_triangle_func(v0, v1, v2);
            }
            if (std::ranges::distance(verticies_inp) >= 3U) {
                const auto v0 = *(std::ranges::cend(verticies_inp) - 2);
                const auto v1 = *(std::ranges::cend(verticies_inp) - 1);
                const auto v2 = *(std::ranges::cbegin(verticies_inp) + 0);

                draw_triangle_func(v0, v1, v2);
            };
        } break;
        }
    }

    template<ProgramInterface Program, class Uniform, class Vertex, FrameBufferInterface FrameBuffer>
        requires(std::is_same_v<Vertex, typename Program::Vertex>)
    static void draw_point(const Program& program,
                           const Uniform& uniform,
                           const bool requires_screen_clipping,
                           const math::Transform2D& scale_to_viewport,
                           const math::Transform2D& screen_to_window,
                           FrameBuffer& framebuffer,
                           const Vertex& vert);

    template<ProgramInterface Program, class Uniform, class Vertex, FrameBufferInterface FrameBuffer>
        requires(std::is_same_v<Vertex, typename Program::Vertex>)
    static void draw_line(const Program& program,
                          const Uniform& uniform,
                          const bool requires_screen_clipping,
                          const math::Transform2D& scale_to_viewport,
                          const math::Transform2D& screen_to_window,
                          const RendererOptions& options,
                          FrameBuffer& framebuffer,
                          const Vertex& v0,
                          const Vertex& v1);

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
            const math::Transform2D& screen_to_window,
            const RendererOptions& options,
            RendererData<typename Program::Varying, Vec4TripletAllocator, AttrsTripletAllocator>& data,
            FrameBuffer& framebuffer,
            const Vertex& v0,
            const Vertex& v1,
            const Vertex& v2);
};

}; // namespace asciirast

#include "renderer/draw_line.h"
#include "renderer/draw_point.h"
#include "renderer/draw_triangle.h"
