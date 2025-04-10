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

using RGB = math::Vec3;

struct RGBC
{
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    char c;
};

class TerminalBuffer
{
public:
    using Targets = std::tuple<char, RGB>;

    TerminalBuffer()
            : m_rgbc_buf{}
            , m_depth_buf{}
    {
        terminal_utils::just_fix_windows_console(true);

        std::cout << CSI::ESC << CSI::HIDE_CURSOR;
        std::cout << CSI::ESC << CSI::DISABLE_LINE_WRAP;

        m_oob_error = false;

        m_width = m_height = 0;
        this->clear_and_update_size();
    }
    ~TerminalBuffer()
    {
        std::cout << CSI::ESC << CSI::SHOW_CURSOR;
        std::cout << CSI::ESC << CSI::ENABLE_LINE_WRAP;
        std::cout << CSI::ESC << CSI::RESET_COLOR;

        terminal_utils::just_fix_windows_console(false);
    }

    bool out_of_bounds_error_occurred() const { return m_oob_error; }

    math::Float aspect_ratio() const
    {
        // this ratio worked best for my terminal

        return (5.f * (math::Float)m_height) / (3.f * (math::Float)m_width);
    }

    const math::Transform2D& screen_to_window() { return m_screen_to_window; }

    void plot(const math::Vec2Int& pos, math::Float depth, const Targets& targets)
    {
        if (!(0 <= pos.x && (std::size_t)pos.x < m_width && 0 <= pos.y && (std::size_t)pos.y < m_height)) {
            m_oob_error = true;
            return;
        }

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        depth = std::clamp(depth, 0.f, 1.f);

        if (m_depth_buf[idx] >= depth) {
            return;
        }

        const auto rgb = std::get<RGB>(targets);
        m_rgbc_buf[idx].r = static_cast<std::uint8_t>(255.f * rgb.r);
        m_rgbc_buf[idx].g = static_cast<std::uint8_t>(255.f * rgb.g);
        m_rgbc_buf[idx].b = static_cast<std::uint8_t>(255.f * rgb.b);
        m_rgbc_buf[idx].c = std::get<char>(targets);
        m_depth_buf[idx] = depth;
    }

    void render() const
    {
        // int casting is neccessary:
        // https://stackoverflow.com/questions/19562103/uint8-t-cant-be-printed-with-cout

        this->reset_printer();

        for (std::size_t y = 0; y < m_height; y++) {
            for (std::size_t x = 0; x < m_width; x++) {
                const auto idx = index(y, x);
                const auto [r, g, b, c] = m_rgbc_buf[idx];

                std::cout << CSI::ESC << CSI::SET_FG_RGB_COLOR << int{ r } << ";" << int{ g } << ";" << int{ b } << "m"
                          << c;
            }
            std::cout << "\n";
        }
        std::cout << CSI::ESC << CSI::RESET_COLOR;

        std::fflush(stdout);
    }

    void clear(const char clear_char = ' ')
    {
        for (std::size_t i = 0; i < m_height * m_width; i++) {
            m_rgbc_buf[i] = { .r = 0, .g = 0, .b = 0, .c = clear_char };
            m_depth_buf[i] = 0.f;
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

        m_width = (std::size_t)new_width;
        m_height = (std::size_t)new_height;

        const math::Vec2 scale = { m_width - 1, m_height - 1 };
        m_screen_to_window =
                asciirast::SCREEN_BOUNDS.to_transform().reversed().reflectY().translate(0, 1.f).scale(scale);

        m_rgbc_buf.resize(m_width * m_height);
        m_depth_buf.resize(m_width * m_height);

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
    bool m_oob_error;

    std::size_t m_width;
    std::size_t m_height;
    std::vector<RGBC> m_rgbc_buf;
    std::vector<math::Float> m_depth_buf;
    math::Transform2D m_screen_to_window;
};

static_assert(asciirast::FrameBufferInterface<TerminalBuffer>); // alternative

struct MyUniform
{
    const std::string& palette;
    const math::Float& aspect_ratio;
};

struct MyVertex
{
    math::Float id;
    math::Vec2 pos;
    RGB color;

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
    RGB color;

    MyVarying operator+(const MyVarying& that) const { return { this->id + that.id, this->color + that.color }; }
    MyVarying operator*(const math::Float scalar) const { return { this->id * scalar, this->color * scalar }; }
};

class MyProgram
{
    using Fragment = asciirast::Fragment<MyVarying>;
    using ProjectedFragment = asciirast::ProjectedFragment<MyVarying>;

public:
    // alias to fullfill program interface:
    using Uniform = MyUniform;
    using Vertex = MyVertex;
    using Varying = MyVarying;
    using Targets = TerminalBuffer::Targets;

    Fragment on_vertex(const Uniform& u, const Vertex& vert) const
    {
        return Fragment{ .pos = math::Vec4{ vert.pos.x * u.aspect_ratio, vert.pos.y, 0, 1 }, // w should be 1 for 2D.
                         .attrs = Varying{ vert.id, vert.color } };
    }
    Targets on_fragment(const Uniform& u, const ProjectedFragment& pfrag) const
    {
        return { u.palette[std::min((std::size_t)pfrag.attrs.id, u.palette.size() - 1)], pfrag.attrs.color };
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>); // alternative

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
main(void)
{
    const std::string palette = "@%#*+=-:."; // Paul Borke's palette

    auto V1 = MyVertex{ 0, math::Vec2{ -1, -1 }, RGB{ 1, 0, 0 } };
    auto V2 =
            MyVertex{ palette.size() - 1.f, math::Vec2{ 0, 1.f / std::numbers::sqrt2_v<math::Float> }, RGB{ 0, 1, 0 } };
    auto V3 = MyVertex{ 0, math::Vec2{ 1, -1 }, RGB{ 0, 0, 1 } };

    int i = 1;
    int dir = 1;
    asciirast::VertexBuffer<MyVertex> vb{};
    vb.shape_type = asciirast::ShapeType::LINES; // Feel free to try POINTS / LINES
    vb.verticies.clear();
    sierpinski_triangle(vb.verticies, V1, V2, V3, i);

    MyProgram p;
    asciirast::Renderer<MyVarying> r;
    TerminalBuffer t;

    t.clear_and_update_size();
    math::Float aspect_ratio = t.aspect_ratio();
    MyUniform u{ palette, aspect_ratio };

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

        if (t.out_of_bounds_error_occurred()) {
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

        vb.verticies.clear();
        sierpinski_triangle(vb.verticies, V1, V2, V3, i);

        t.clear_and_update_size();
        aspect_ratio = t.aspect_ratio();
    }
    check_eof_program.join();
}
