// Based on:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes.html

#include "examples/common/PPMBuffer.h"

#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"

#include <filesystem>

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
};

DERIVE_VARYING_OPS(MyVarying);

class RGBProgram
{
    using Fragment = asciirast::Fragment<MyVarying>;
    using ProjectedFragment = asciirast::ProjectedFragment<MyVarying>;

public:
    using Uniform = MyUniform;
    using Vertex = MyVertex;
    using Varying = MyVarying;
    using Targets = PPMBuffer::Targets;

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        out.pos = { vert.pos.xy, asciirast::compute_reverse_depth(vert.pos.z, u.z_near, u.z_far), vert.pos.z };
        out.attrs = { vert.color, vert.uv };
    }
    void on_fragment([[maybe_unused]] const Uniform& u, const ProjectedFragment& pfrag, Targets& out) const
    {
        out = pfrag.attrs.color.rgb;
    }
};

static_assert(asciirast::ProgramInterface<RGBProgram>);

class CheckerboardProgram
{
    using Fragment = asciirast::Fragment<MyVarying>;
    using ProjectedFragment = asciirast::ProjectedFragment<MyVarying>;

public:
    using Uniform = MyUniform;
    using Vertex = MyVertex;
    using Varying = MyVarying;
    using Targets = PPMBuffer::Targets;

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        out.pos = { vert.pos.xy, asciirast::compute_reverse_depth(vert.pos.z, u.z_near, u.z_far), vert.pos.z };
        out.attrs = { math::Vec3{ 1.f, 1.f, 1.f }, vert.uv };
    }

    void on_fragment([[maybe_unused]] const Uniform& u, const ProjectedFragment& pfrag, Targets& out) const
    {
        const auto uv = pfrag.attrs.uv;
        const auto M = 10.f;
        const auto uM_decimal_part = std::fmod(uv.r * M, 1.f);
        const auto vM_decimal_part = std::fmod(uv.g * M, 1.f);
        const auto pattern = (uM_decimal_part > 0.5f) ^ (vM_decimal_part < 0.5f);

        out = { pfrag.attrs.color * pattern };
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
