#include "examples/common/SDLBuffer.h"

#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"

#include <vector>

struct MyUniform
{};

struct MyVertex
{
    math::Vec3 pos;
    RGB color;
};

struct MyVarying
{
    RGB color;
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
    using Targets = SDLBuffer::Targets;

    void on_vertex([[maybe_unused]] const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        out.pos.xy = { vert.pos.x, vert.pos.y };
        out.attrs = { vert.color };
    }
    void on_fragment([[maybe_unused]] const Uniform& u, const ProjectedFragment& pfrag, Targets& out) const
    {
        out = { pfrag.attrs.color.rgb, 1 };
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>);

void
handle_events(bool& running)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if ((SDL_QUIT == ev.type) || (SDL_KEYDOWN == ev.type && SDL_SCANCODE_ESCAPE == ev.key.keysym.scancode)) {
            running = false;
            break;
        }
    }
}

int
main(int, char**)
{
    const MyVertex v0 = { .pos = { -0.5f, -0.5f, 1.f }, .color = { 1.f, 0.f, 0.f } };
    const MyVertex v1 = { .pos = { +0.0f, +0.5f, 1.f }, .color = { 0.f, 1.f, 0.f } };
    const MyVertex v2 = { .pos = { +0.5f, -0.5f, 1.f }, .color = { 0.f, 0.f, 1.f } };

    asciirast::VertexBuffer<MyVertex> vertex_buf;
    vertex_buf.shape_type = asciirast::ShapeType::Triangles;
    vertex_buf.verticies = { v0, v1, v2 };

    SDLBuffer screen(512, 512);
    MyProgram program;
    asciirast::Renderer renderer;
    asciirast::RendererData<MyVarying> renderer_data{ screen.screen_to_window() };
    MyUniform uniforms;

    bool running = true;
    while (running) {
        handle_events(running);
        screen.clear();
        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data);
        screen.render();
    }

    return 0;
}
