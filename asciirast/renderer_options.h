/**
 * @file renderer_options.h
 * @brief Renderer options
 */

#pragma once

namespace asciirast {

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

};
