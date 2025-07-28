
#include "examples/common/TerminalBuffer.h"

#include "./ctables.hpp"

#include "asciirast/math/types.h"
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

using ctable = std::array<std::array<char, 3>, 3>;

struct MyUniform
{
    math::Float aspect_ratio;
    bool draw_horizontal;
    math::Rot2D rot;
    static constexpr ctable table = { //
        { { '\\', '|', '/' },         //
          { '_', ' ', '_' },
          { '/', '|', '\\' } }
    };
};

struct MyVertex
{
    math::Vec2 pos;
    math::Vec3 color;
};

struct MyVarying
{
    math::Vec3 color;

    MyVarying operator+(const MyVarying& that) const { return { this->color + that.color }; }
    MyVarying operator*(const math::Float scalar) const { return { this->color * scalar }; }
};

class MyProgram
{
    using Fragment = asciirast::Fragment<MyVarying>;
    using ProjectedFragment = asciirast::ProjectedFragment<MyVarying>;

public:
    using Uniform = MyUniform;
    using Vertex = MyVertex;
    using Varying = MyVarying;
    using Targets = TerminalBuffer::Targets;
    using FragmentContext = asciirast::FragmentContextType<math::Vec2Int>;

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        const auto pos = u.rot.apply(vert.pos);

        out.pos.xy = { pos.x * u.aspect_ratio, pos.y };
        out.attrs = { vert.color };
    }

    auto on_fragment(FragmentContext& c, const Uniform& u, const ProjectedFragment& pfrag, Targets& out) const
            -> asciirast::ProgramTokenGenerator
    {
        co_yield c.init(math::Vec2Int{ pfrag.pos });

        const math::Vec2Int dv =
                (c.type() == FragmentContext::Type::LINE) ? c.dFdv<math::Vec2Int>() : math::Vec2Int{ 0, 0 };

        const char ch = u.table[(size_t)(std::clamp<math::Int>(dv.y, -1, 1) + 1)]
                               [(size_t)(std::clamp<math::Int>(dv.x, -1, 1) + 1)];

        const bool keep = (u.draw_horizontal && ch == '_') ||  //
                          (!u.draw_horizontal && ch != '_') || //
                          c.type() == FragmentContext::Type::POINT;

        if (keep) {
            out = { ch, pfrag.attrs.color };
            co_yield asciirast::ProgramToken::Keep;
        } else {
            co_yield asciirast::ProgramToken::Discard;
        }
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>);

FramebufferPoint
fix_corners(const TerminalBuffer& t, const math::Vec2Int& pos)
{
    ctable inp;
    for (std::size_t dy = 0; dy < 3; dy++) {
        for (std::size_t dx = 0; dx < 3; dx++) {
            inp[dy][dx] = t.at(clamp(pos + math::Vec2Int{ -1, -1 } + math::Vec2Int{ dx, dy },
                                     math::Vec2Int{ 0, 0 },
                                     t.size() + math::Vec2Int{ -1, -1 }));
        }
    }

    // char c = '@';
    char c = ' ';
    for (std::size_t i = 0; i < ctables.size(); i++) {
        bool match = true;
        for (std::size_t dy = 0; dy < 3; dy++) {
            for (std::size_t dx = 0; dx < 3; dx++) {
                if ((dy == 1 && dx == 1) || ctables[i][dy][dx] == ign) continue;

                match &= ctables[i][dy][dx] == inp[dy][dx];
                match &= !(inp[dy][dx] == '_' && dy > 0) || inp[dy - 1][dx] != '_';
            }
        }
        if (match == true) {
            c = ctables[i][1][1];
            break;
        }
    }
    return FramebufferPoint{ pos, TerminalBuffer::Targets{ c, math::Vec3{ 1, 1, 1 } } };
}

template<asciirast::RendererOptions Options>
std::vector<FramebufferPoint>&
fix_corners(const MyUniform& u,
            const TerminalBuffer& t,
            const asciirast::Renderer<Options>& r,
            const std::vector<MyVertex>& verticies,
            std::vector<FramebufferPoint>& out)
{
    out.clear();
    for (auto vert : verticies) {
        const auto pos0 = u.rot.apply(vert.pos);
        const auto pos1 = math::Vec4{ pos0.x * u.aspect_ratio, pos0.y, 0, 1 };
        if (!asciirast::renderer::point_in_frustum(pos1)) {
            continue;
        }
        const auto frag0 = asciirast::project_fragment(asciirast::Fragment{ pos1, asciirast::EmptyVarying() });
        const auto frag1 = asciirast::Renderer<>::apply_scale_to_viewport(r.scale_to_viewport(), frag0);
        const auto frag2 = asciirast::Renderer<>::apply_screen_to_window(t.screen_to_window(), frag1);
        const auto pos2 = math::Vec2Int{ frag2.pos };
        if (math::Vec2Int{ 0, 0 } <= pos2 - math::Vec2Int{ 1, 1 } && //
            pos2 + math::Vec2Int{ 1, 1 } <= t.size()) {
            out.push_back(fix_corners(t, pos2));
        }
    }
    return out;
}

int
main(int, char**)
{
    asciirast::VertexBuffer<MyVertex> circle_buf;
    {
        math::Rot2D rot{ math::radians(360 / 30.f) };
        math::Vec2 v = math::Vec2{ 0., 0.8f };

        for (size_t i = 0; i < 30; i++) {
            circle_buf.verticies.push_back(MyVertex{ v, math::Vec3{ 1.f, 1.f, 1.f } });
            v = rot.apply(v);
        }
    }
    circle_buf.shape_type = asciirast::ShapeType::LineLoop;

    asciirast::VertexBuffer<MyVertex> line_buf;
    {
        math::Rot2D rot{ math::radians(180 - 9.f * 2) };
        math::Vec2 vf = math::Rot2D{ math::radians(9.f) }.apply(math::Vec2{ 0., 0.8f });
        math::Vec2 vr = math::Rot2D{ math::radians(-9.f) }.apply(math::Vec2{ 0., 0.8f });

        line_buf.verticies.push_back(MyVertex{ vf, math::Vec3{ 1.f, 1.f, 1.f } });
        line_buf.verticies.push_back(MyVertex{ rot.apply(vf), math::Vec3{ 1.f, 1.f, 1.f } });
        line_buf.verticies.push_back(MyVertex{ vr, math::Vec3{ 1.f, 1.f, 1.f } });
        line_buf.verticies.push_back(MyVertex{ rot.apply_inv(vr), math::Vec3{ 1.f, 1.f, 1.f } });
    }
    {
        math::Rot2D rot{ math::radians(180 - 9.f * 2) };
        math::Vec2 vf = math::Rot2D{ math::radians(90.f + 9.f) }.apply(math::Vec2{ 0., 0.8f });
        math::Vec2 vr = math::Rot2D{ math::radians(90.f + -9.f) }.apply(math::Vec2{ 0., 0.8f });

        line_buf.verticies.push_back(MyVertex{ vf, math::Vec3{ 1.f, 1.f, 1.f } });
        line_buf.verticies.push_back(MyVertex{ rot.apply(vf), math::Vec3{ 1.f, 1.f, 1.f } });
        line_buf.verticies.push_back(MyVertex{ vr, math::Vec3{ 1.f, 1.f, 1.f } });
        line_buf.verticies.push_back(MyVertex{ rot.apply_inv(vr), math::Vec3{ 1.f, 1.f, 1.f } });
    }
    line_buf.shape_type = asciirast::ShapeType::Lines;

    MyProgram program;
    TerminalBuffer framebuffer;
    MyUniform uniforms;
    uniforms.aspect_ratio = framebuffer.aspect_ratio();
    std::vector<FramebufferPoint> points;

    constexpr asciirast::RendererOptions circle_options{
        .line_drawing_direction = asciirast::LineDrawingDirection::Downwards,
        .line_ends_inclusion = asciirast::LineEndsInclusion::IncludeBoth,
    };

    constexpr asciirast::RendererOptions line_options{
        .line_drawing_direction = asciirast::LineDrawingDirection::Downwards,
        .line_ends_inclusion = asciirast::LineEndsInclusion::ExcludeBoth,
    };

    asciirast::Renderer<circle_options> cr0{ math::AABB2D::from_min_max({ -1, -1 }, { 1, 1 }) };
    asciirast::Renderer<circle_options> cr1{
        math::AABB2D::from_min_max({ -1, -1 }, { 1, 1 }).size_set(math::Vec2{ 1.5, 1.5 })
    };
    asciirast::Renderer<line_options> lr{
        math::AABB2D::from_min_max({ -1, -1 }, { 1, 1 }).size_set(math::Vec2{ 1.5, 1.5 })
    };
    asciirast::RendererData<MyVarying> renderer_data{ framebuffer.screen_to_window() };

    std::binary_semaphore sem{ 0 };

    std::thread check_eof_program{ [&sem] {
        while (std::cin.get() != EOF) {
            continue;
        }
        sem.release();
    } };

    while (!sem.try_acquire()) {
        uniforms.draw_horizontal = false; // prefer other chars over '_':

        cr0.draw(program, uniforms, circle_buf, framebuffer, renderer_data);
        cr1.draw(program, uniforms, circle_buf, framebuffer, renderer_data);
        lr.draw(program, uniforms, line_buf, framebuffer, renderer_data);

        uniforms.draw_horizontal = true;
        cr0.draw(program, uniforms, circle_buf, framebuffer, renderer_data);
        cr1.draw(program, uniforms, circle_buf, framebuffer, renderer_data);
        lr.draw(program, uniforms, line_buf, framebuffer, renderer_data);

        for (auto [pos, targets] : fix_corners(uniforms, framebuffer, cr0, circle_buf.verticies, points)) {
            framebuffer.plot(pos, targets);
        }
        for (auto [pos, targets] : fix_corners(uniforms, framebuffer, cr1, circle_buf.verticies, points)) {
            framebuffer.plot(pos, targets);
        }

        framebuffer.render();

        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        if (framebuffer.clear_and_update_size()) {
            renderer_data.screen_to_window = framebuffer.screen_to_window();
        }
        uniforms.aspect_ratio = framebuffer.aspect_ratio();
        uniforms.rot.stack(math::radians(-10.f));
    }
    check_eof_program.join();
}
