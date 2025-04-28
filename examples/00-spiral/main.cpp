
#include "asciirast/framebuffer.h"
#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"
#include "external/terminal_utils/terminal_utils.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <semaphore>
#include <stdexcept>
#include <thread>
#include <vector>

namespace math = asciirast::math;
namespace CSI = terminal_utils::CSI;

class TerminalBuffer : public asciirast::AbstractFrameBuffer<char>
{
public:
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

    const math::Transform2D& screen_to_window() { return m_screen_to_window; }

    void plot(const math::Vec2Int& pos, const Targets& targets) override
    {
        if (!(0 <= pos.x && (std::size_t)pos.x < m_width && 0 <= pos.y && (std::size_t)pos.y < m_height)) {
            std::cerr << pos << "\n";
            throw std::logic_error("error: point plotted outside of border! the library should not allow this.");
        }

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);

        m_charbuf[idx] = std::get<0>(targets);
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
    std::size_t m_width;
    std::size_t m_height;
    math::Transform2D m_screen_to_window;
    std::vector<char> m_charbuf;
};

struct MyUniform
{
    const math::Rot2D& rot;
    const std::string& palette;
    const math::Float& aspect_ratio;
    const bool& should_flip;
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

    MyVarying operator+(const MyVarying& that) const { return { this->id + that.id }; }
    MyVarying operator*(const math::Float scalar) const { return { this->id * scalar }; }
};

class MyProgram : public asciirast::AbstractProgram<MyUniform, MyVertex, MyVarying, TerminalBuffer>
{
    using Fragment = asciirast::Fragment<MyVarying>;
    using PFragment = asciirast::ProjectedFragment<MyVarying>;
    using OnFragmentRes = std::generator<asciirast::ProgramToken>;

public:
    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const override
    {
        math::Float id = vert.id;
        math::Vec2 v = vert.pos;

        if (u.should_flip) {
            v = u.flip_transform.apply(v);
            v = u.rot.apply_inv(v);
        } else {
            v = u.rot.apply(v);
        }

        out.pos = { v.x * u.aspect_ratio, v.y, 0, 1 }; // w should be 1 for 2D
        out.attrs = { id };
    }
    OnFragmentRes on_fragment(FragmentContext&, const Uniform& u, const PFragment& pfrag, Targets& out) const override
    {
        out = { u.palette[std::min((std::size_t)pfrag.attrs.id, u.palette.size() - 1)] };
        co_return;
    }
};

int
main(int, char**)
{
    const std::string palette = "@%#*+=-:. "; // Paul Borke's palette
    const math::Float aspect_ratio = 3.f / 5.f;
    bool flip = false;

    math::Rot2D u_rot{};
    MyUniform uniforms{ u_rot, palette, aspect_ratio, flip };

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
            id = std::min((id + 0.2f), (float)palette.size() - 1);
            v *= f;
            vertex_buf.verticies.push_back(MyVertex{ id, math::Vec2{ v.real(), v.imag() } });
        }
    }

    MyProgram program;
    TerminalBuffer framebuffer;

    asciirast::Renderer r1{ math::AABB2D::from_min_max({ -1.5f, -1.f }, { +0.5f, +1.f }) };
    asciirast::Renderer r2{ math::AABB2D::from_min_max({ -0.5f, -1.f }, { +1.5f, +1.f }) };
    asciirast::RendererData<MyVarying> renderer_data{ framebuffer.screen_to_window() };

    std::binary_semaphore sem{ 0 };

    std::thread check_eof_program{ [&sem] {
        while (std::cin.get() != EOF) {
            continue;
        }
        sem.release();
    } };

    while (!sem.try_acquire()) {
        flip = false;
        r1.draw(program, uniforms, vertex_buf, framebuffer, renderer_data);

        flip = true;
        r2.draw(program, uniforms, vertex_buf, framebuffer, renderer_data);

        framebuffer.render();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (framebuffer.clear_and_update_size()) {
            renderer_data.screen_to_window = framebuffer.screen_to_window();
        }

        u_rot.stack(math::radians(-45.f));
    }
    check_eof_program.join();
}
