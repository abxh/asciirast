
#include "asciirast/framebuffer.h"
#include "asciirast/math/types.h"
#include "asciirast/program.h"
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

    const math::Transform2D& screen_to_window() override { return m_screen_to_window; }

    void plot(const math::Vec2Int& pos, math::Float depth, const Targets& targets) override
    {
        assert(0 <= pos.x && (std::size_t)(pos.x) <= m_width);
        assert(0 <= pos.y && (std::size_t)(pos.y) <= m_height);

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        depth = std::clamp(depth, 0.f, 1.f);

        if (m_depthbuf[idx] >= depth) {
            return;
        }

        m_charbuf[idx] = std::get<0>(targets);
        m_depthbuf[idx] = depth;
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
            m_depthbuf[i] = 0.f;
        }
    }

    void clear_and_update_size(const char clear_char = ' ')
    {
        int new_width = 0, new_height = 0;
        terminal_utils::get_terminal_size(new_width, new_height);

        if (m_width == (std::size_t)(new_width - 1) && m_height == (std::size_t)(new_height - 1)) {
            this->clear(clear_char);
            return;
        }
        new_width = std::max(2, new_width - 1);
        new_height = std::max(2, new_height - 1);

        this->reset_printer();

        m_width = (std::size_t)(new_width);
        m_height = (std::size_t)(new_height);

        const math::Vec2 scale = { m_width - 1, m_height - 1 };
        m_screen_to_window =
                asciirast::SCREEN_BOUNDS.to_transform().reversed().reflectY().translate(0, 1.f).scale(scale);

        m_charbuf.resize(m_width * m_height);
        m_depthbuf.resize(m_width * m_height);

        this->offset_printer();
        this->clear(clear_char);
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
    std::vector<char> m_charbuf;
    std::vector<math::Float> m_depthbuf;
    math::Transform2D m_screen_to_window;
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
    using Fragment = asciirast::Fragment<Varying>;
    using ProjectedFragment = asciirast::ProjectedFragment<Varying>;

public:
    Fragment on_vertex(const Uniform& u, const Vertex& vert) const override
    {
        math::Float id = vert.id;
        math::Vec2 v = vert.pos;
        if (u.should_flip) {
            v = u.flip_transform.apply(v);
            v = u.rot.invert(v);
        } else {
            v = u.rot.apply(v);
        }

        return Fragment{ .pos = math::Vec4{ v.x * u.aspect_ratio, v.y, 0, 1 }, // w should be 1 for 2D
                         .attrs = Varying{ id } };
    }
    Targets on_fragment(const Uniform& u, const ProjectedFragment& pfrag) const override
    {
        return { u.palette[std::min((std::size_t)pfrag.attrs.id, u.palette.size() - 1)] };
    }
};

int
main(void)
{
    const std::string palette = "@%#*+=-:. "; // Paul Borke's palette
    const math::Float aspect_ratio = 3.f / 5.f;
    bool flip = false;

    math::Rot2D u_rot{};
    MyUniform u{ u_rot, palette, aspect_ratio, flip };

    asciirast::VertexBuffer<MyVertex> vb;
    {
        /*
           raising a complex number c = a + bi to numbers n=1,2,... ((a+bi)^n) where |a^2+b^2| > 1, gives you a
           so-called logarithmic spiral which goes outwards.
        */
        vb.shape_type = asciirast::ShapeType::LINE_STRIP; // Feel free to try POINTS / LINES / LINE_STRIP
        vb.verticies = {};

        auto id = 0.f;
        auto v = std::complex<float>{ 0.05f, 0.f }; // 0.05f instead of 1.f to scale it down
        auto f = std::complex<float>{ std::polar(1.1f, math::radians(45.f / 2.f)) };

        for (int i = 0; i < 50; i++) {
            id = std::min((id + 0.2f), (float)palette.size() - 1);
            v *= f;
            vb.verticies.push_back(MyVertex{ id, v });
        }
    }
    asciirast::Renderer<MyVarying> r1{ math::AABB2D::from_min_max({ -1.5f, -1.f }, { +0.5f, +1.f }) };
    asciirast::Renderer<MyVarying> r2{ math::AABB2D::from_min_max({ -0.5f, -1.f }, { +1.5f, +1.f }) };

    MyProgram p;
    TerminalBuffer t;

    std::binary_semaphore s{ 0 };

    std::thread check_eof_program{ [&s] {
        while (std::cin.get() != EOF) {
            continue;
        }
        s.release();
    } };

    while (!s.try_acquire()) {
        flip = false;
        r1.draw(p, u, vb, t);

        flip = true;
        r2.draw(p, u, vb, t);

        t.render();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        t.clear_and_update_size();

        u_rot.stack(math::radians(-45.f));
    }
    check_eof_program.join();
}
