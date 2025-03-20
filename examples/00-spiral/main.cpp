
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
namespace CSI = terminal_utils::CSI;

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

    Transform2WrappedView get_viewport_to_window() const override { return m_transform; }

    void plot(const math::Vec2Int& pos, const math::F depth, const Targets& targets) override
    {
        if (!(0 <= pos.x && pos.x <= m_width && 0 <= pos.y && pos.y <= m_height)) {
            std::cout << pos << "\n";
            std::abort();
        }

        const auto idx = index(pos.y, pos.x);

        if (!(m_depthbuf[idx] < depth)) {
            return;
        }

        m_buf[idx] = std::get<0>(targets);
        m_depthbuf[idx] = depth;
    }

    void render() const
    {
        this->reset_printer();

        for (int y = 0; y < m_height; y++) {
            for (int x = 0; x < m_width; x++) {
                std::cout << m_buf[index(y, x)];
            }
            std::cout << "\n";
        }

        std::fflush(stdout);
    }

    void clear(const char clear_char = ' ')
    {
        for (int i = 0; i < m_height * m_width; i++) {
            m_buf[i] = clear_char;
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
        new_width = std::max(2, new_width - 1);
        new_height = std::max(2, new_height - 1);

        this->reset_printer();

        m_width = new_width;
        m_height = new_height;
        m_transform = std::move(math::Transform2()
                                        .reflectY()
                                        .translate(0, asciirast::Renderer::VIEWPORT_BOUNDS.size_get().y)
                                        .scale(m_width - 1, m_height - 1));
        m_buf.resize(new_width * new_height);
        m_depthbuf.resize(new_width * new_height);

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
    std::vector<char> m_buf;
    std::vector<math::F> m_depthbuf;
    Transform2Wrapped m_transform;
};

class Uniform
{
public:
    const math::Rot2& rot;
    const std::string& palette;
};

class Vertex
{
public:
    float id;
    math::Vec2 pos;
    Vertex(float id, math::Vec2 pos2)
            : id{ id }
            , pos{ pos2 } {};
};

class Varying
{
public:
    float id;

    explicit Varying(float id)
            : id{ id } {};

    friend Varying operator+(const Varying& lhs, const Varying& rhs) { return Varying{ lhs.id + rhs.id }; }
    friend Varying operator*(const float scalar, const Varying& v) { return Varying{ scalar * v.id }; }
};

class Program : public asciirast::Program<Uniform, Vertex, Varying, TerminalBuffer>
{
public:
    using Fragment = asciirast::Fragment<Varying>;
    using ProjectedFragment = asciirast::ProjectedFragment<Varying>;

    Fragment on_vertex(const Uniform& u, const Vertex& vert) const override
    {
        return Fragment{ .pos = math::Vec4{ u.rot.apply(vert.pos), 0, 1 }, // w should be 1 for 2D.
                         .attrs = Varying{ vert.id } };
    }
    Targets on_fragment(const Uniform& u, const ProjectedFragment& pfrag) const override
    {
        return { u.palette[std::min((std::size_t)pfrag.attrs.id, u.palette.size())] };
    }
};

int
main(void)
{
    const std::string palette = "@%#*+=-:. "; // Paul Borke's palette

    Program p;

    math::Rot2 u_rot{};
    Uniform u{ u_rot, palette };

    asciirast::VertexBuffer<Vertex> vb;
    {
        /*
           raising a complex number c = a + bi to numbers n=1,2,... ((a+bi)^n) where |a^2+b^2| > 1, gives you a
           so-called logarithmic spiral which goes outwards.
        */
        vb.shape_type = asciirast::ShapeType::LINE_STRIP;
        vb.verticies = std::move(std::vector<Vertex>{
                { 0, math::Vec2{ 0.05f, 0 } },
        });
        math::Rot2 f{ math::angle_as_radians(45.f / 2) };
        f.dir *= 1.1; // hack which works since it uses complex numbers
        for (int i = 0; i < 40; i++) {
            Vertex last_vertex = vb.verticies[vb.verticies.size() - 1];
            vb.verticies.push_back(
                    Vertex{ std::min((last_vertex.id + 0.2f), (float)palette.size()), f.apply(last_vertex.pos) });
        }
    }
    asciirast::Renderer r1{ math::AABB2::from_min_max(math::Vec2{ 0.0f, 0.0f }, math::Vec2{ 0.5f, 0.5f }) };
    asciirast::Renderer r2{ math::AABB2::from_min_max(math::Vec2{ 0.5f, 0.5f }, math::Vec2{ 1.0f, 1.0f }) };
    asciirast::Renderer r3{ math::AABB2::from_min_max(math::Vec2{ 0.5f, 0.0f }, math::Vec2{ 1.0f, 0.5f }) };
    asciirast::Renderer r4{ math::AABB2::from_min_max(math::Vec2{ 0.0f, 0.5f }, math::Vec2{ 0.5f, 1.0f }) };

    TerminalBuffer t;

    std::binary_semaphore s{ 0 };

    std::thread check_eof_program{ [&s] {
        while (std::cin.get() != EOF) {
            continue;
        }
        s.release();
    } };

    while (!s.try_acquire()) {
        r1.draw(p, u, vb, t);
        r2.draw(p, u, vb, t);
        r3.draw(p, u, vb, t);
        r4.draw(p, u, vb, t);

        t.render();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        t.clear_and_update_size();

        u_rot.stack(math::Rot2{ math::angle_as_radians(-45.f) });
    }
    check_eof_program.join();
}
