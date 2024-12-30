/**
 * @file program.h
 * @brief The shader program interface.
 *
 * Inspiration:
 * https://github.com/nikolausrauch/software-rasterizer/
 */

#include "color/color.h"
#include "framebuffer.h"

namespace asciirast {

template <typename VertexAttributes>
concept is_interpolable = requires(VertexAttributes v) {
    v + v;
    v * (-1.f);
};

template <typename Uniform,
          typename VertexAttributes,
          typename PixelAttributes,
          Framebuffer,
          typename... Outputs>
    requires(is_interpolable<PixelAttributes>)
class Program {
public:
    virtual ~Program() = default;

    virtual void on_vertex(const Uniform& uniform,
                           const VertexAttributes& in,
                           PixelAttributes& out) = 0;

    virtual void on_pixel(const Uniform& uniform,
                          const PixelAttributes& in_attr,
                          math::Vec4f out) = 0;
};

}  // namespace asciirast
