
#include "examples/common/TerminalBuffer.h"

#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>

struct MyUniform
{
    math::Rot2D rot;
    std::string palette;
    math::Float aspect_ratio;
    static const inline auto flip_transform = math::Transform2D().rotate(math::radians(180.f)).reflectX();
};

struct MyVertex
{
    math::Float id;
    math::Vec2 pos;
};

struct MyVarying
{
    math::Float id;
};

DERIVE_VARYING_OPS(MyVarying);

class MyProgram
{
    using Fragment = asciirast::Fragment<MyVarying>;
    using ProjectedFragment = asciirast::ProjectedFragment<MyVarying>;

public:
    using Uniform = MyUniform;
    using Vertex = MyVertex;
    using Varying = MyVarying;
    using Targets = TerminalBuffer::Targets;

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        math::Float id = vert.id;
        math::Vec2 v = u.rot.apply(vert.pos);

        out.pos.xy = { v.x * u.aspect_ratio, v.y };
        out.attrs = { id };
    }
    void on_fragment(const Uniform& u, const ProjectedFragment& pfrag, Targets& out) const
    {
        out = { u.palette[std::min((std::size_t)pfrag.attrs.id, u.palette.size() - 1)], math::Vec3{ 1, 1, 1 } };
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>);

int
main(int, char**)
{
    MyUniform uniforms;

    uniforms.palette = "@%#*+=-:. "; // Paul Borke's palette
    uniforms.aspect_ratio = 3.f / 5.f;

    asciirast::VertexBuffer<MyVertex> vertex_buf;
    {
        /*
           raising a complex number c = a + bi to numbers n=1,2,... ((a+bi)^n) where |a^2+b^2| > 1, gives you a
           so-called logarithmic spiral which goes outwards.
        */
        vertex_buf.shape_type = asciirast::ShapeType::LineStrip; // Feel free to try Points / Lines / LineStrip
        vertex_buf.verticies = {};

        auto id = 0.f;
        auto v = std::complex<float>{ 0.05f, 0.f }; // 0.05f instead of 1.f to scale it down
        auto f = std::complex<float>{ std::polar(1.1f, math::radians(45.f / 2.f)) };

        for (int i = 0; i < 50; i++) {
            id = std::min((id + 0.2f), (float)uniforms.palette.size() - 1);
            v *= f;
            vertex_buf.verticies.push_back(MyVertex{ id, math::Vec2{ v.real(), v.imag() } });
        }
    }

    MyProgram program;
    TerminalBuffer framebuffer(5 / 2.f);

    asciirast::Renderer<{ .attr_interpolation = asciirast::AttrInterpolation::NoPerspective }> renderer;
    asciirast::RendererData<MyVarying> renderer_data;

    std::binary_semaphore sem{ 0 };

    std::thread check_eof_program{ [&sem] {
        while (std::cin.get() != EOF) {
            continue;
        }
        sem.release();
    } };

    while (!sem.try_acquire()) {
        renderer.draw(program, uniforms, vertex_buf, framebuffer, renderer_data);

        framebuffer.render();

        if (framebuffer.out_of_bounds_error_occurred()) {
            std::cout << "error: point plotted outside of border! the library should not allow this.\n";
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        framebuffer.clear_and_update_size();

        uniforms.rot.stack(math::radians(-45.f));
    }
    check_eof_program.join();
}
