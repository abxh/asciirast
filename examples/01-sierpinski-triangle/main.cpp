#include "examples/common/TerminalBuffer.h"

#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>

struct MyUniform
{
    std::string palette;
    math::Float aspect_ratio;
};

struct MyVertex
{
    math::Float id;
    math::Vec2 pos;
    RGBFloat color;

    MyVertex operator+(const MyVertex& that) const
    {
        return { this->id + that.id, this->pos + that.pos, this->color + that.color };
    }
    MyVertex operator/(const math::Float scalar) const
    {
        return { this->id / scalar, this->pos / scalar, this->color / scalar };
    }
};

struct MyVarying
{
    math::Float id;
    RGBFloat color;
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
        out.pos.xy = { vert.pos.x * u.aspect_ratio, vert.pos.y };
        out.attrs = { vert.id, vert.color };
    }
    void on_fragment(const Uniform& u, const ProjectedFragment& pfrag, Targets& out) const
    {
        out = { u.palette[std::min((std::size_t)pfrag.attrs.id, u.palette.size() - 1)], pfrag.attrs.color };
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>);

void
sierpinski_triangle(std::vector<MyVertex>& v,
                    const MyVertex& V1,
                    const MyVertex& V2,
                    const MyVertex& V3,
                    const int depth = 1)
{
    if (depth <= 0) {
        return;
    }
    v.push_back(V1);
    v.push_back(V2);

    v.push_back(V2);
    v.push_back(V3);

    v.push_back(V3);
    v.push_back(V1);

    const auto V1V2 = (V1 + V2) / 2.f;
    const auto V1V3 = (V1 + V3) / 2.f;
    const auto V2V3 = (V2 + V3) / 2.f;

    sierpinski_triangle(v, V1, V1V2, V1V3, depth - 1);
    sierpinski_triangle(v, V1V2, V2, V2V3, depth - 1);
    sierpinski_triangle(v, V1V3, V2V3, V3, depth - 1);
}

int
main(int, char**)
{
    MyUniform uniforms;

    uniforms.palette = "@%#*+=-:."; // Paul Borke's palette

    auto V1 = MyVertex{ 0, math::Vec2{ -1, -1 }, RGBFloat{ 1, 0, 0 } };
    auto V2 = MyVertex{ uniforms.palette.size() - 1.f,
                        math::Vec2{ 0, 1.f / std::numbers::sqrt2_v<math::Float> },
                        RGBFloat{ 0, 1, 0 } };
    auto V3 = MyVertex{ 0, math::Vec2{ 1, -1 }, RGBFloat{ 0, 0, 1 } };

    int i = 1;
    int dir = 1;
    asciirast::VertexBuffer<MyVertex> vertex_buf{};
    vertex_buf.shape_type = asciirast::ShapeType::Lines; // Feel free to try Points / Lines
    vertex_buf.verticies.clear();
    sierpinski_triangle(vertex_buf.verticies, V1, V2, V3, i);

    MyProgram program;
    TerminalBuffer framebuffer(5 / 3.f);
    asciirast::Renderer renderer;
    asciirast::RendererData<MyVarying> renderer_data{ framebuffer.screen_to_window() };

    framebuffer.clear_and_update_size();
    uniforms.aspect_ratio = framebuffer.aspect_ratio();

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

        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (i <= 1) {
            dir = 1;
        } else if (i >= 5) {
            dir = -1;
        }
        i += dir;

        vertex_buf.verticies.clear();
        sierpinski_triangle(vertex_buf.verticies, V1, V2, V3, i);

        if (framebuffer.clear_and_update_size()) {
            renderer_data.screen_to_window = framebuffer.screen_to_window();
        }
        uniforms.aspect_ratio = framebuffer.aspect_ratio();
    }
    check_eof_program.join();
}
