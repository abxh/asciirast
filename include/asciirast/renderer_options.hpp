/**
 * @file renderer_options.hpp
 * @brief Renderer options
 *
 * Quads vertex order (top-left, top-right, bottom-left, bottom-right):
 *  0 --- 1
 *  |     |
 *  3 --- 2
 *
 * QuadStrip vertex order:
 *  0 --- 2 --- 4
 *  |     |     | ...
 *  1 --- 3 --- 5
 */

#pragma once

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
    Quads,
    QuadStrip,
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
 * @brief Attribute interpolation qualifier
 */
enum class AttrInterpolation
{
    Smooth,
    NoPerspective,
    Flat,
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

    AttrInterpolation attr_interpolation = AttrInterpolation::Smooth; ///< attribute interpolation qualifier
};

}; // namespace asciirast
