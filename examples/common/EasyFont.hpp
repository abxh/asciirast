
#include <asciirast.hpp>

#include <cassert>
#include <stb_easy_font.h>

namespace math = asciirast::math;

template<std::size_t N = 99999> // ~500 chars
struct EasyFontVertexBuffer
{
    int nverts = -1;
    char buffer[N];
};

struct EasyFontUniform
{
    math::Int font_width;
    math::Int font_height;
    math::Transform2D transform;
};

struct EasyFontVertex
{
    math::Vec3 pos;
    std::uint8_t padding[3];
};

template<typename Uniform, std::size_t N>
[[maybe_unused]] static void
init_string(const std::string& text, Uniform& uniforms, EasyFontVertexBuffer<N>& verts, const float spacing = 1.0f)
    requires(std::derived_from<Uniform, EasyFontUniform>)
{
    assert(text.length() <= 500);

    verts.nverts =
        4 * stb_easy_font_print(0, 0, const_cast<char*>(text.c_str()), nullptr, verts.buffer, sizeof(verts.buffer));
    uniforms.font_height = stb_easy_font_height(const_cast<char*>(text.c_str()));
    uniforms.font_width = stb_easy_font_width(const_cast<char*>(text.c_str()));
    uniforms.transform = asciirast::math::Transform2D()
                             .scale(uniforms.font_width, 2 * uniforms.font_height)
                             .inversed()
                             .reflectY()
                             .scale(2, 2);
    stb_easy_font_spacing(spacing);
}

template<typename Uniform, std::size_t N>
[[maybe_unused]] static void
draw_string(const asciirast::ProgramInterface auto program,
            const Uniform& uniforms,
            const EasyFontVertexBuffer<N>& verts,
            const auto renderer,
            asciirast::FrameBufferInterface auto& out,
            auto& renderer_data)
    requires(std::derived_from<Uniform, EasyFontUniform>)
{
    if (verts.nverts >= 0) {
        renderer.draw(program,
                      uniforms,
                      asciirast::ShapeType::Quads,
                      static_cast<std::size_t>(verts.nverts),
                      reinterpret_cast<const EasyFontVertex*>(&verts.buffer[0]),
                      out,
                      renderer_data);
    }
}
