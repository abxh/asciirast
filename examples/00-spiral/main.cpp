
#include "asciirast/framebuffer.h"
#include "asciirast/math.h"
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
namespace CSI  = terminal_utils::CSI;

class TerminalBuffer : public asciirast::FrameBuffer<char>
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

    const math::Transform2D& viewport_to_window() const override { return m_viewport_to_window; }

    void plot(const math::Vec2Int& pos, const math::F depth, const Targets& targets) override
    {
        assert(0 <= pos.x && pos.x <= m_width);
        assert(0 <= pos.y && pos.y <= m_height);

        const auto idx = index(pos.y, pos.x);

        if (m_depthbuf[idx] > depth) {
            return;
        }

        m_charbuf[idx]  = std::get<0>(targets);
        m_depthbuf[idx] = depth;
    }

    void render() const
    {
        this->reset_printer();

        for (int y = 0; y < m_height; y++) {
            for (int x = 0; x < m_width; x++) {
                std::cout << m_charbuf[index(y, x)];
            }
            std::cout << "\n";
        }

        std::fflush(stdout);
    }

    void clear(const char clear_char = ' ')
    {
        for (int i = 0; i < m_height * m_width; i++) {
            m_charbuf[i]  = clear_char;
            m_depthbuf[i] = -std::numeric_limits<math::F>::infinity();
        }
    }

    void clear_and_update_size(const char clear_char = ' ')
    {
        int new_width = 0, new_height = 0;
        terminal_utils::get_terminal_size(new_width, new_height);

        if (m_width == new_width - 1 && m_height == new_height - 1) {
            this->clear(clear_char);
            return;
        }
        new_width  = std::max(2, new_width - 1);
        new_height = std::max(2, new_height - 1);

        this->reset_printer();

        m_width              = new_width;
        m_height             = new_height;
        m_viewport_to_window = math::Transform2D().reflectY().translate(0, 1.f).scale(m_width - 1, m_height - 1);
        m_charbuf.resize(m_width * m_height);
        m_depthbuf.resize(m_width * m_height);

        this->offset_printer();
        this->clear(clear_char);
    }

private:
    int index(const int y, const int x) const { return m_width * y + x; }
    void reset_printer() const { std::cout << CSI::ESC << m_height << CSI::MOVE_UP_LINES << '\r'; }
    void offset_printer() const
    {
        for (int y = 0; y < m_height; y++) {
            std::cout << CSI::ESC << CSI::CLEAR_LINE << "\n";
        }
    }

    int m_width;
    int m_height;
    std::vector<char> m_charbuf;
    std::vector<math::F> m_depthbuf;
    math::Transform2D m_viewport_to_window;
};

struct Uniform
{
    const math::Rot2D& rot;
    const std::string& palette;
    const math::F& aspect_ratio;
};

struct Vertex
{
    float id;
    math::Vec2 pos;
};

struct Varying
{
    float id;

    Varying operator+(const Varying& that) const { return Varying{ this->id + that.id }; }
    Varying operator*(const float that) const { return Varying{ this->id * that }; }
};

class Program : public asciirast::Program<Uniform, Vertex, Varying, TerminalBuffer>
{
    using Fragment          = asciirast::Fragment<Varying>;
    using ProjectedFragment = asciirast::ProjectedFragment<Varying>;

public:
    Fragment on_vertex(const Uniform& u, const Vertex& vert) const override
    {
        const auto v = u.rot.apply(vert.pos);
        return Fragment{ .pos   = math::Vec4{ v.x * u.aspect_ratio, v.y, 0, 1 }, // w should be 1 for 2D
                         .attrs = Varying{ vert.id } };
    }
    Targets on_fragment(const Uniform& u, const ProjectedFragment& pfrag) const override
    {
        return { u.palette[std::min((std::size_t)pfrag.attrs.id, u.palette.size() - 1)] };
    }
};

int
main(void)
{
    const std::string palette  = "@%#*+=-:. "; // Paul Borke's palette
    const math::F aspect_ratio = 3.f / 5.f;

    Program p;

    math::Rot2D u_rot{};
    Uniform u{ u_rot, palette, aspect_ratio };

    asciirast::VertexBuffer<Vertex> vb;
    {
        /*
           raising a complex number c = a + bi to numbers n=1,2,... ((a+bi)^n) where |a^2+b^2| > 1, gives you a
           so-called logarithmic spiral which goes outwards.
        */
        vb.shape_type = asciirast::ShapeType::LINE_STRIP; // Feel free to try POINTS / LINES / LINE_STRIP
        vb.verticies  = std::move(std::vector<Vertex>{
                { 0, math::Vec2{ 0.05f, 0 } },
        });

        math::Rot2D f{ math::radians(45.f / 2) };

        for (int i = 0; i < 40; i++) {
            const auto last_vertex = vb.verticies[vb.verticies.size() - 1U];

            const auto id  = std::min((last_vertex.id + 0.2f), (float)palette.size());
            const auto pos = 1.1f * f.apply(last_vertex.pos);

            vb.verticies.push_back(Vertex{ id, pos });
        }
    }
    asciirast::Renderer r;

    TerminalBuffer t;

    std::binary_semaphore s{ 0 };

    std::thread check_eof_program{ [&s] {
        while (std::cin.get() != EOF) {
            continue;
        }
        s.release();
    } };

    while (!s.try_acquire()) {
        r.draw(p, u, vb, t);

        t.render();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        t.clear_and_update_size();

        u_rot.stack(math::Rot2D{ math::radians(-45.f) });
    }
    check_eof_program.join();
}
