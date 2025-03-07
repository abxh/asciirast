
#include "asciirast/framebuffer.h"
#include "asciirast/math/types.h"
#include "asciirast/math/utils.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"
#include "external/terminal_utils/terminal_utils.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>

using namespace asciirast;

class TerminalAdapter : public asciirast::Framebuffer<char>
{
public:
    TerminalAdapter()
    {
        just_fix_windows_console(true);

        m_width = m_height = 0;
        this->check_terminal_size();

        std::cout << CSI_ESC CSI_HIDECURSOR;
        this->clear_lines();
        this->clear_buffer();
    }
    ~TerminalAdapter()
    {
        std::cout << CSI_ESC CSI_SHOWCURSOR;

        just_fix_windows_console(false);
    }
    void clear_lines()
    {
        for (int y = 0; y < m_height; y++) {
            std::cout << CSI_ESC CSI_CLEARLINE "\n";
        }
    }
    void clear_buffer()
    {
        for (int i = 0; i < m_height * m_width; i++) {
            m_buf[i] = ' ';
        }
    }

    void check_terminal_size()
    {
        int width = 0, height = 0;
        get_terminal_size(width, height);
        if (m_width == width && m_height == height - 1) {
            return;
        };
        std::cout << CSI_ESC << m_height << CSI_MOVEUPLINES << '\r';

        m_width = std::max(1, width);
        m_height = std::max(1, height - 1);
        m_viewport_to_window = math::Transform2().reflectY().translate(0, 1.f).scale(m_width - 1, m_height - 1);
        m_buf.reserve(m_width * m_height);

        this->clear_lines();
        this->clear_buffer();
    }

    void render() const
    {
        std::cout << CSI_ESC << m_height << CSI_MOVEUPLINES << '\r';
        for (int y = 0; y < m_height; y++) {
            for (int x = 0; x < m_width; x++) {
                std::cout << m_buf[index(y, x)];
            }
            std::cout << "\n";
        }
        std::fflush(stdout);
    }

    math::Transform2& get_viewport_to_window_transform() override { return m_viewport_to_window; }

    void plot(const math::Vec2Int& pos, std::tuple<char> targets) override
    {
        m_buf[index(pos.y, pos.x)] = std::get<0>(targets);
    }

private:
    int index(const int y, const int x) const { return m_width * y + x; }

    int m_width;
    int m_height;
    math::Transform2 m_viewport_to_window;
    std::vector<char> m_buf;
};

class CustomUniform
{
public:
    math::Rot2& rot;
    std::string& palette;
};

class CustomVertex
{
public:
    float id;
    math::Vec2 pos2;
    CustomVertex(float id, math::Vec2 pos2)
            : id{ id }
            , pos2{ pos2 } {};
};

class CustomVarying
{
public:
    float id;
    math::Vec4 pos;

    CustomVarying(float id, const math::Vec4& position)
            : id{ id }
            , pos{ position } {};

    friend CustomVarying operator+(const CustomVarying& lhs, const CustomVarying& rhs)
    {
        return { std::min(lhs.id, rhs.id), lhs.pos + rhs.pos };
    }
    friend CustomVarying operator*(const float scalar, const CustomVarying& v) { return { v.id, scalar * v.pos }; }
};

class CustomProgram : public Program<CustomUniform, CustomVertex, CustomVarying, TerminalAdapter>
{
public:
    CustomVarying on_vertex(const CustomUniform& u, const CustomVertex& vert) const override
    {
        math::Rot2& rot = u.rot;
        math::Vec2 pos = rot.apply(vert.pos2);

        return { vert.id, math::Vec4{ pos, 0, 1 } }; // last component should be kept 1
    }
    std::tuple<char> on_fragment(const CustomUniform& u, const CustomVarying& frag) const override
    {
        auto [id, pos] = frag;

        return u.palette[(int)id];
    }
};

int
main(void)
{
    Renderer r;
    TerminalAdapter t;
    CustomProgram p;

    math::Rot2 rot{};
    math::Rot2 inc{ math::angle_as_radians(-45.f) };

    std::string palette = "@%#*+=-:."; // Paul Borke's palette
    CustomUniform u{ rot, palette };

    VertexBuffer<CustomVertex> vb;
    vb.shape_type = asciirast::ShapeType::POINTS; // only type now.. :7
    vb.verticies = std::move(std::vector<CustomVertex>{
            { 0, math::Vec2{ 0.1f, 0 } },
    });

    math::Rot2 f{ math::angle_as_radians(45.f / 2) };
    f.dir *= 1.1; // hack which works since it uses complex numbers

    for (int i = 0; i < 20; i++) {
        CustomVertex last_vertex = vb.verticies[vb.verticies.size() - 1];
        vb.verticies.push_back(
                CustomVertex{ std::min((last_vertex.id + 0.4f), (float)palette.size()), f.apply(last_vertex.pos2) });
    }

    std::binary_semaphore s{ 0 };

    std::thread peek_inp{ [&] {
        while (std::cin.peek() != EOF) {
            continue;
        }
        s.release();
    } };

    while (!s.try_acquire()) {
        r.apply(p, u, vb, t);
        t.render();
        t.clear_buffer();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        t.check_terminal_size();
        rot.stack(inc);
    }
    peek_inp.join();
}
