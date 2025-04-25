// Based on:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes.html
// https://tomhultonharrop.com/mathematics/graphics/2023/08/06/reverse-z.html

#include "asciirast/framebuffer.h"
#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"
#include <cassert>
#include <filesystem>
#include <fstream>

namespace math = asciirast::math;
using RGBFloat = math::Vec3;

enum class ImageType
{
    RED_CHANNEL,
    BLUE_CHANNEL,
    GREEN_CHANNEL,
    RGB,
    DEPTH_CHANNEL
};

struct RGB
{
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
};

class PPMBuffer
{
    static constexpr math::Float default_depth = 2; // or +infty

public:
    using Targets = std::tuple<RGBFloat>;

    PPMBuffer(const std::size_t width, const std::size_t height)
            : m_width{ width }
            , m_height{ height }
    {
        m_screen_to_window = asciirast::Renderer::SCREEN_BOUNDS //
                                     .to_transform()
                                     .reversed()
                                     .reflectY()
                                     .translate(0, 1.f)
                                     .scale(m_width - 1, m_height - 1);

        m_rgb_buf.resize(m_width * m_height);
        m_depth_buf.resize(m_width * m_height);

        this->clear();
    }

    void save_to(const std::string& fp, const ImageType type = ImageType::RGB)
    {
        // int casting is neccessary:
        // https://stackoverflow.com/questions/19562103/uint8-t-cant-be-printed-with-cout

        std::ofstream out;
        out.open(fp, std::ofstream::out | std::ofstream::trunc);

        out << "P3\n" << m_width << " " << m_height << "\n255\n"; // note: there are other formats than P3

        if (type == ImageType::RGB) {
            for (std::size_t y = 0; y < m_height; y++) {
                for (std::size_t x = 0; x < m_width; x++) {
                    const auto idx = index(y, x);
                    const auto [r, g, b] = m_rgb_buf[idx];
                    out << int{ r } << ' ' << int{ g } << ' ' << int{ b } << '\n';
                }
            }
        } else if (type == ImageType::DEPTH_CHANNEL) {
            for (std::size_t y = 0; y < m_height; y++) {
                for (std::size_t x = 0; x < m_width; x++) {
                    const auto idx = index(y, x);
                    if (m_depth_buf[idx] != default_depth) {
                        const auto val = static_cast<int>(255.f * (1 - m_depth_buf[idx]));
                        out << val << ' ' << val << ' ' << val << '\n';
                    } else {
                        const auto [r, g, b] = m_rgb_buf[idx];
                        out << int{ r } << ' ' << int{ g } << ' ' << int{ b } << '\n';
                    }
                }
            }
        } else {
            for (std::size_t y = 0; y < m_height; y++) {
                for (std::size_t x = 0; x < m_width; x++) {
                    const auto idx = index(y, x);
                    if (m_depth_buf[idx] != default_depth) {
                        const auto [r, g, b] = m_rgb_buf[idx];
                        out << int(type == ImageType::RED_CHANNEL ? r : 0) << ' ';
                        out << int(type == ImageType::GREEN_CHANNEL ? g : 0) << ' ';
                        out << int(type == ImageType::BLUE_CHANNEL ? b : 0) << '\n';
                    } else {
                        const auto [r, g, b] = m_rgb_buf[idx];
                        out << int{ r } << ' ' << int{ g } << ' ' << int{ b } << '\n';
                    }
                }
            }
        }

        out.close();
    }

    bool test_and_set_depth(const math::Vec2Int& pos, math::Float depth)
    {
        assert(0 <= pos.x && (std::size_t)(pos.x) < m_width);
        assert(0 <= pos.y && (std::size_t)(pos.y) < m_height);

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        depth = std::clamp<math::Float>(depth, 0, 1);

        if (depth < m_depth_buf[idx]) {
            m_depth_buf[idx] = depth;
            return true;
        }
        return false;
    }

    const math::Transform2D& screen_to_window() { return m_screen_to_window; }

    void plot(const math::Vec2Int& pos, const Targets& targets)
    {
        assert(0 <= pos.x && (std::size_t)pos.x < m_width);
        assert(0 <= pos.y && (std::size_t)pos.y < m_height);

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        const auto [r, g, b] = std::get<RGBFloat>(targets).array();

        m_rgb_buf[idx].r = static_cast<std::uint8_t>(255.f * r);
        m_rgb_buf[idx].g = static_cast<std::uint8_t>(255.f * g);
        m_rgb_buf[idx].b = static_cast<std::uint8_t>(255.f * b);
    }

    void clear()
    {
        for (std::size_t i = 0; i < m_height * m_width; i++) {
            m_rgb_buf[i] = { .r = 128, .g = 128, .b = 128 };
            m_depth_buf[i] = default_depth;
        }
    }

private:
    std::size_t index(const std::size_t y, const std::size_t x) const { return m_width * y + x; }

    std::size_t m_width;
    std::size_t m_height;
    math::Transform2D m_screen_to_window;

    std::vector<RGB> m_rgb_buf;
    std::vector<math::Float> m_depth_buf;
};

static_assert(asciirast::FrameBufferInterface<PPMBuffer>);

struct MyUniform
{
    math::Float z_near;
    math::Float z_far;
};

struct MyVertex
{
    math::Vec3 pos;
    RGBFloat color;
    math::Vec2 uv;
};

struct MyVarying
{
    RGBFloat color;
    math::Vec2 uv;

    MyVarying operator+(const MyVarying& that) const { return { this->color + that.color, this->uv + that.uv }; }
    MyVarying operator*(const math::Float scalar) const { return { this->color * scalar, this->uv * scalar }; }
};

class RGBProgram
{
    using Fragment = asciirast::Fragment<MyVarying>;
    using PFragment = asciirast::ProjectedFragment<MyVarying>;
    using OnFragmentRes = std::generator<asciirast::SpecialFragmentToken>;

public:
    using Uniform = MyUniform;
    using Vertex = MyVertex;
    using Varying = MyVarying;
    using Targets = PPMBuffer::Targets;
    using FragmentContext = asciirast::FragmentContextType<>;

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        const auto depth_scalar = u.z_far / (u.z_far - u.z_near);

        out.pos = { vert.pos.xy, vert.pos.z * depth_scalar - u.z_near * depth_scalar, vert.pos.z };
        out.attrs = { vert.color, vert.uv };
    }
    OnFragmentRes on_fragment(FragmentContext&, const Uniform& u, const PFragment& pfrag, Targets& out) const
    {
        (void)(u);
        out = { pfrag.attrs.color };
        co_return;
    }
};

static_assert(asciirast::ProgramInterface<RGBProgram>);

class CheckerboardProgram
{
    using Fragment = asciirast::Fragment<MyVarying>;
    using PFragment = asciirast::ProjectedFragment<MyVarying>;
    using OnFragmentRes = std::generator<asciirast::SpecialFragmentToken>;

public:
    using Uniform = MyUniform;
    using Vertex = MyVertex;
    using Varying = MyVarying;
    using Targets = PPMBuffer::Targets;
    using FragmentContext = asciirast::FragmentContextType<>;

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        const auto depth_scalar = u.z_far / (u.z_far - u.z_near);

        out.pos = { vert.pos.xy, vert.pos.z * depth_scalar - u.z_near * depth_scalar, vert.pos.z };
        out.attrs = { math::Vec3{ 1.f, 1.f, 1.f }, vert.uv };
    }

    OnFragmentRes on_fragment(FragmentContext&, const Uniform& u, const PFragment& pfrag, Targets& out) const
    {
        (void)(u);

        const auto uv = pfrag.attrs.uv;
        const auto M = 10.f;
        const auto uM_decimal_part = std::fmod(uv.r * M, 1.f);
        const auto vM_decimal_part = std::fmod(uv.g * M, 1.f);
        const auto pattern = (uM_decimal_part > 0.5f) ^ (vM_decimal_part < 0.5f);

        out = { pfrag.attrs.color * pattern };
        co_return;
    }
};

static_assert(asciirast::ProgramInterface<CheckerboardProgram>);

int
main(int, char**)
{
    const math::Vec3 v2 = { -48, -10, 82 };
    const math::Vec3 v1 = { 29, -15, 44 };
    const math::Vec3 v0 = { 13, 34, 114 };

    const math::Vec3 c2 = { 1, 0, 0 };
    const math::Vec3 c1 = { 0, 1, 0 };
    const math::Vec3 c0 = { 0, 0, 1 };

    const math::Vec2 st2 = { 0, 0 };
    const math::Vec2 st1 = { 1, 0 };
    const math::Vec2 st0 = { 0, 1 };

    asciirast::VertexBuffer<MyVertex> vertex_buf;
    vertex_buf.shape_type = asciirast::ShapeType::Triangles;
    vertex_buf.verticies = { MyVertex{ v2, c2, st2 }, MyVertex{ v0, c0, st0 }, MyVertex{ v1, c1, st1 } };

    MyUniform uniforms{ .z_near = std::min({ v0.z, v1.z, v2.z }), .z_far = std::max({ v0.z, v1.z, v2.z }) };

    PPMBuffer screen(512, 512);
    asciirast::Renderer renderer;
    asciirast::RendererData<MyVarying> renderer_data{ screen.screen_to_window() };

    std::filesystem::create_directory("images");
    const auto image_path = std::filesystem::path("images");

    RGBProgram p1;
    renderer.draw(p1, uniforms, vertex_buf, screen, renderer_data);
    screen.save_to(image_path / "rgb.ppm", ImageType::RGB);
    screen.save_to(image_path / "red.ppm", ImageType::RED_CHANNEL);
    screen.save_to(image_path / "green.ppm", ImageType::GREEN_CHANNEL);
    screen.save_to(image_path / "blue.ppm", ImageType::BLUE_CHANNEL);
    screen.save_to(image_path / "depth.ppm", ImageType::DEPTH_CHANNEL);
    screen.clear();

    CheckerboardProgram p2;
    renderer.draw(p2, uniforms, vertex_buf, screen, renderer_data);
    screen.save_to(image_path / "checkerboard.ppm", ImageType::RGB);
    screen.clear();

    return 0;
}
