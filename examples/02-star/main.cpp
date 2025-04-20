
#include "asciirast/framebuffer.h"
#include "asciirast/math/types.h"
#include "asciirast/renderer.h"
#include "external/terminal_utils/terminal_utils.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>

namespace math = asciirast::math;
namespace CSI = terminal_utils::CSI;

class TerminalBuffer
{
public:
    using Targets = std::tuple<char>;

    TerminalBuffer()
    {
        terminal_utils::just_fix_windows_console(true);

        std::cout << CSI::ESC << CSI::HIDE_CURSOR;
        std::cout << CSI::ESC << CSI::DISABLE_LINE_WRAP;

        m_width = m_height = 0;
        this->clear_and_update_size();
    }

    ~TerminalBuffer()
    {
        std::cout << CSI::ESC << CSI::SHOW_CURSOR;
        std::cout << CSI::ESC << CSI::ENABLE_LINE_WRAP;

        terminal_utils::just_fix_windows_console(false);
    }

    bool test_and_set_depth(const math::Vec2Int&, math::Float) { return true; }

    const math::Transform2D& screen_to_window() { return m_screen_to_window; }

    void plot(const math::Vec2Int& pos, const Targets& targets)
    {
        assert(0 <= pos.x && (std::size_t)(pos.x) < m_width);
        assert(0 <= pos.y && (std::size_t)(pos.y) < m_height);

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);

        m_charbuf[idx] = std::get<char>(targets);
    }

    void render() const
    {
        this->reset_printer();

        for (std::size_t y = 0; y < m_height; y++) {
            for (std::size_t x = 0; x < m_width; x++) {
                std::cout << m_charbuf[index(y, x)];
            }
            std::cout << "\n";
        }

        std::fflush(stdout);
    }

    void clear(const char clear_char = ' ')
    {
        for (std::size_t i = 0; i < m_height * m_width; i++) {
            m_charbuf[i] = clear_char;
        }
    }

    bool clear_and_update_size(const char clear_char = ' ')
    {
        int new_width = 0, new_height = 0;
        terminal_utils::get_terminal_size(new_width, new_height);

        if (m_width == (std::size_t)(new_width - 1) && m_height == (std::size_t)(new_height - 1)) {
            this->clear(clear_char);
            return false;
        }
        new_width = std::max(2, new_width - 1);
        new_height = std::max(2, new_height - 1);

        this->reset_printer();

        m_width = (std::size_t)(new_width);
        m_height = (std::size_t)(new_height);

        m_screen_to_window = asciirast::Renderer::SCREEN_BOUNDS //
                                     .to_transform()
                                     .reversed()
                                     .reflectY()
                                     .translate(0, 1.f)
                                     .scale(m_width - 1, m_height - 1);

        m_charbuf.resize(m_width * m_height);

        this->offset_printer();
        this->clear(clear_char);
        return true;
    }

    std::size_t m_width;
    std::size_t m_height;

private:
    std::size_t index(const std::size_t y, const std::size_t x) const { return m_width * y + x; }
    void reset_printer() const
    {
        for (std::size_t y = 0; y < m_height; y++) {
            std::cout << CSI::ESC << CSI::MOVE_UP_LINE << "\r";
        }
    }
    void offset_printer() const
    {
        for (std::size_t y = 0; y < m_height; y++) {
            std::cout << CSI::ESC << CSI::CLEAR_LINE << "\n";
        }
    }

    math::Transform2D m_screen_to_window;
    std::vector<char> m_charbuf;
};

static_assert(asciirast::FrameBufferInterface<TerminalBuffer>);

struct MyUniform
{
    math::Float aspect_ratio;
    bool draw_horizontal;
    math::Rot2D& rot;
    static constexpr std::array<std::array<char, 3>, 3> table = { //
        { { '\\', '|', '/' },                                     //
          { '_', ':', '_' },
          { '/', '|', '\\' } }
    };
};

struct MyVertex
{
    std::size_t idx;
    math::Vec2 pos;
};

using MyVarying = asciirast::EmptyVarying;
using Fragment = asciirast::Fragment<MyVarying>;
using ProjectedFragment = asciirast::ProjectedFragment<MyVarying>;
using Result = asciirast::FragmentResult<typename TerminalBuffer::Targets>;

class MyProgram
{
public:
    using Uniform = MyUniform;
    using Vertex = MyVertex;
    using Varying = MyVarying;
    using Targets = TerminalBuffer::Targets;
    using FragmentContext = asciirast::FragmentContextType<math::Vec2Int>;

    Fragment on_vertex(const Uniform& u, const Vertex& vert) const
    {
        const auto pos = u.rot.apply(vert.pos);

        return Fragment{ math::Vec4{ pos.x * u.aspect_ratio, pos.y, 0, 1 }, asciirast::EmptyVarying() };
    }
    std::generator<Result> on_fragment(FragmentContext& c, const Uniform& u, const ProjectedFragment& pfrag) const
    {
        co_yield c.init(math::Vec2Int{ pfrag.pos }, std::type_identity<Targets>());

        const math::Vec2Int dv =
                (c.type() == FragmentContext::Type::LINE) ? c.dFdv<math::Vec2Int>() : math::Vec2Int{ 0, 0 };

        const char ch = u.table[(size_t)(std::clamp<math::Float>(dv.y, -1, 1) + 1)]
                               [(size_t)(std::clamp<math::Float>(dv.x, -1, 1) + 1)];

        if (ch == '_') {
            if (u.draw_horizontal) {
                co_yield Result{ '_' };
            } else {
                co_yield asciirast::FragmentResultDiscard();
            }
        }
        co_yield Result{ ch };
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>);

int
main(int, char**)
{
    const math::Float aspect_ratio = 3.f / 5.f;

    asciirast::VertexBuffer<MyVertex> vertex_buf;
    {
        math::Rot2D rot{ math::radians(2 * 360 / 5.f) };
        math::Vec2 v = math::Vec2{ 0., 0.8f };

        for (size_t i = 0; i < 5; i++) {
            vertex_buf.verticies.push_back(MyVertex{ i, v });
            v = rot.apply(v);
        }
    }
    vertex_buf.shape_type = asciirast::ShapeType::LineLoop;

    MyProgram program;
    TerminalBuffer framebuffer;
    math::Rot2D rot;
    MyUniform uniforms{ aspect_ratio, true, rot };

    asciirast::Renderer renderer;
    asciirast::RendererData<MyVarying> renderer_data{ framebuffer.screen_to_window() };
    asciirast::RendererOptions options;

    // ensure lines always points downwards:
    options.line_drawing_direction = asciirast::LineDrawingDirection::Downwards;

    // e.g. with:
    // vertex_buf.shape_type = asciirast::ShapeType::Lines;
    // for (size_t i = 0; i < 6; i++) {
    //     auto& v = vertex_buf.verticies[2 * i + 0];
    //     auto& w = vertex_buf.verticies[2 * i + 1];
    //     if (v.pos.vector_to(w.pos).y > 0) {
    //         std::swap(v, w);
    //     }
    // }

    std::binary_semaphore sem{ 0 };

    std::thread check_eof_program{ [&sem] {
        while (std::cin.get() != EOF) {
            continue;
        }
        sem.release();
    } };

    while (!sem.try_acquire()) {
        // prefer other chars over '_':
        uniforms.draw_horizontal = true;
        renderer.draw(program, uniforms, vertex_buf, framebuffer, renderer_data, options);

        uniforms.draw_horizontal = false;
        renderer.draw(program, uniforms, vertex_buf, framebuffer, renderer_data, options);

        framebuffer.render();

        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        if (framebuffer.clear_and_update_size()) {
            renderer_data.screen_to_window = framebuffer.screen_to_window();
        }

        rot.stack(math::radians(-10.f));
    }
    check_eof_program.join();
}
