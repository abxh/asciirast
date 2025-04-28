
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

using RGBFloat = math::Vec3;

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
    using Targets = std::tuple<char, RGBFloat>;

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

    math::Vec2Int size() const { return { m_width, m_height }; }

    const char& at(const math::Vec2Int& pos) const
    {
        return m_rgbc_buf[index((std::size_t)pos.y, (std::size_t)pos.x)].c;
    }

    math::Float aspect_ratio() const
    {
        // this ratio worked best for my terminal

        return (5.f * (math::Float)m_height) / (2.f * (math::Float)m_width);
    }

    bool test_and_set_depth(const math::Vec2Int& pos, math::Float depth)
    {
        if (!(0 <= pos.x && (std::size_t)pos.x < m_width && 0 <= pos.y && (std::size_t)pos.y < m_height)) {
            m_oob_error = true;
            return false;
        }

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        depth = std::clamp<math::Float>(depth, 0, 1);

        if (depth < m_depth_buf[idx]) {
            m_depth_buf[idx] = depth;
            return true;
        }
        return false;
    }

    const math::Transform2D& screen_to_window() const { return m_screen_to_window; }

    void plot(const math::Vec2Int& pos, const Targets& targets)
    {
        if (!(0 <= pos.x && (std::size_t)pos.x < m_width && 0 <= pos.y && (std::size_t)pos.y < m_height)) {
            m_oob_error = true;
            return;
        }

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        const auto [r, g, b] = std::get<RGBFloat>(targets).array();

        m_rgbc_buf[idx].r = static_cast<std::uint8_t>(255.f * r);
        m_rgbc_buf[idx].g = static_cast<std::uint8_t>(255.f * g);
        m_rgbc_buf[idx].b = static_cast<std::uint8_t>(255.f * b);
        m_rgbc_buf[idx].c = std::get<char>(targets);
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
            m_depth_buf[i] = 2; // or +infty
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

        m_width = (std::size_t)new_width;
        m_height = (std::size_t)new_height;

        m_screen_to_window = asciirast::Renderer::SCREEN_BOUNDS //
                                     .to_transform()
                                     .reversed()
                                     .reflectY()
                                     .translate(0, 1.f)
                                     .scale(m_width - 1, m_height - 1);

        m_rgbc_buf.resize(m_width * m_height);
        m_depth_buf.resize(m_width * m_height);

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
    bool m_oob_error;

    std::size_t m_width;
    std::size_t m_height;
    math::Transform2D m_screen_to_window;

    std::vector<RGBC> m_rgbc_buf;
    std::vector<math::Float> m_depth_buf;
};

static_assert(asciirast::FrameBufferInterface<TerminalBuffer>); // alternative

using ctable = std::array<std::array<char, 3>, 3>;

struct MyUniform
{
    math::Float aspect_ratio;
    bool draw_horizontal;
    math::Rot2D& rot;
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
    using PFragment = asciirast::ProjectedFragment<MyVarying>;
    using OnFragmentRes = std::generator<asciirast::SpecialToken>;

public:
    // alias to fullfill program interface:
    using Uniform = MyUniform;
    using Vertex = MyVertex;
    using Varying = MyVarying;
    using Targets = TerminalBuffer::Targets;
    using FragmentContext = asciirast::FragmentContextType<math::Vec2Int>;

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        const auto pos = u.rot.apply(vert.pos);

        out.pos = { pos.x * u.aspect_ratio, pos.y, 0, 1 };
        out.attrs = { vert.color };
    }
    OnFragmentRes on_fragment(FragmentContext& context, const Uniform& u, const PFragment& pfrag, Targets& out) const
    {
        co_yield context.init(math::Vec2Int{ pfrag.pos });

        const math::Vec2Int dv =
                (context.type() == FragmentContext::Type::LINE) ? context.dFdv<math::Vec2Int>() : math::Vec2Int{ 0, 0 };

        const char ch = u.table[(size_t)(std::clamp<math::Int>(dv.y, -1, 1) + 1)]
                               [(size_t)(std::clamp<math::Int>(dv.x, -1, 1) + 1)];

        const bool keep = (u.draw_horizontal && ch == '_') ||  //
                          (!u.draw_horizontal && ch != '_') || //
                          context.type() == FragmentContext::Type::POINT;

        if (keep) {
            out = { ch, pfrag.attrs.color };
            co_yield asciirast::SpecialToken::Keep;
        } else {
            co_yield asciirast::SpecialToken::Discard;
        }
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>);

using FramebufferPoint = std::tuple<math::Vec2Int, TerminalBuffer::Targets>;

static constexpr char ign = '\0'; // ignore

//  possible chars: ' ', '\\', '|', '/', '_'

// static constexpr ctable ctable000 = { { { '_', ' ', '_' },     // _ _
//                                         { ' ', 'V', ' ' },     //  V
//                                         { ign, ign, ign } } }; //

static constexpr ctable ctable001 = { { { ign, '_', ign },     //  _
                                        { '_', '|', ign },     // _|
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable002 = { { { ign, '_', ign },     //  _
                                        { ign, '|', '_' },     //  |_
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable003 = { { { ign, ign, ign },     //
                                        { '_', '_', '_' },     // ___
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable004 = { { { ign, ign, '_' },     //   _
                                        { '_', '/', ign },     // _/
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable005 = { { { '_', ign, ign },     // _
                                        { ign, '\\', '_' },    /*  \_ */
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable006 = { { { ign, '|', ign },     //  |
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; //  |

static constexpr ctable ctable007 = { { { ign, '/', ign },     //  /
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; //  |

static constexpr ctable ctable008 = { { { ign, '|', ign },      //  |
                                        { ign, '|', ign },      //  |
                                        { ign, '\\', ign } } }; /*  \ */

static constexpr ctable ctable009 = { { { ign, '/', ign },      //  /
                                        { ign, '|', ign },      //  |
                                        { ign, '\\', ign } } }; /*  \ */

static constexpr ctable ctable010 = { { { '\\', ign, ign },    /* \  */
                                        { ign, '|', ign },     //  |
                                        { '/', ign, ign } } }; // /

static constexpr ctable ctable011 = { { { '\\', ign, ign },    /* \  */
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; //  |

static constexpr ctable ctable012 = { { { '\\', ign, ign },     /* \  */
                                        { ign, '\\', ign },     /*  \ */
                                        { ign, ign, '\\' } } }; /*   \ */

static constexpr ctable ctable013 = { { { ign, ign, '/' },      //   /
                                        { ign, '|', ign },      //  |
                                        { ign, ign, '\\' } } }; /*   \ */

static constexpr ctable ctable014 = { { { ign, ign, '/' },     //   /
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; //  |

static constexpr ctable ctable015 = { { { ign, ign, '/' },     /*   /  */
                                        { ign, '/', ign },     /*  /   */
                                        { '/', ign, ign } } }; /* /    */

static constexpr ctable ctable016 = { { { ign, '|', ign },     //  |
                                        { ign, '|', ign },     //  |
                                        { '/', ign, ign } } }; // /

static constexpr ctable ctable017 = { { { ign, '|', ign },      /*  |   */
                                        { ign, '|', ign },      /*  |   */
                                        { ign, ign, '\\' } } }; /*   \  */

static constexpr ctable ctable018 = { { { '|', ign, ign },      /* |    */
                                        { ign, '\\', ign },     /*  \   */
                                        { ign, ign, '\\' } } }; /*   \  */

static constexpr ctable ctable019 = { { { ign, ign, '|' },     /*   |   */
                                        { ign, '/', ign },     /*  /    */
                                        { '/', ign, ign } } }; /* /     */

static constexpr ctable ctable020 = { { { '_', ign, ign },      /* _    */
                                        { ign, '\\', ign },     /*  \   */
                                        { ign, ign, '\\' } } }; /*   \  */

static constexpr ctable ctable021 = { { { ign, ign, '_' },     /*   _   */
                                        { ign, '/', ign },     /*  /    */
                                        { '/', ign, ign } } }; /* /     */

static constexpr ctable ctable022 = { { { '\\', ign, ign },    /* \     */
                                        { ign, '\\', '_' },    /*  \_   */
                                        { ign, ign, ign } } }; /*       */

static constexpr ctable ctable023 = { { { ign, ign, '/' },     /*   /   */
                                        { '_', '/', ign },     /* _/    */
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable024 = { { { ign, '\\', ign },    /*  \  */
                                        { ign, '|', ign },     //  |
                                        { '/', ign, ign } } }; // /

static constexpr ctable ctable025 = { { { ign, '\\', ign },     /*  \   */
                                        { ign, '|', ign },      /*  |   */
                                        { ign, ign, '\\' } } }; /*   \  */

static constexpr ctable ctable026 = { { { ign, '/', ign },     /*  /  */
                                        { ign, '|', ign },     //  |
                                        { '/', ign, ign } } }; // /

static constexpr ctable ctable027 = { { { ign, '/', ign },      /*  /   */
                                        { ign, '|', ign },      /*  |   */
                                        { ign, ign, '\\' } } }; /*   \  */

static constexpr ctable ctable028 = { { { '|', ign, ign },     /* |   */
                                        { ign, '\\', ign },    /*  \  */
                                        { ign, ign, '|' } } }; //   |

static constexpr ctable ctable029 = { { { ign, ign, '|' },     /*   |   */
                                        { ign, '/', ign },     /*  /    */
                                        { '|', ign, ign } } }; /* |     */

static constexpr ctable ctable030 = { { { '|', ign, ign },     /* |   */
                                        { ign, '\\', '_' },    /*  \_ */
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable031 = { { { ign, ign, '|' },     /*   |   */
                                        { '_', '/', ign },     /* _/    */
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable032 = { { { ign, ign, '|' },      //   |
                                        { ign, '|', ign },      //  |
                                        { ign, ign, '\\' } } }; /*   \ */

static constexpr ctable ctable033 = { { { ign, ign, '/' },     //   /
                                        { ign, '|', ign },     //  |
                                        { ign, ign, '|' } } }; /*   | */

static constexpr ctable ctable034 = { { { '|', ign, ign },     // |
                                        { ign, '|', ign },     //  |
                                        { '/', ign, ign } } }; /* /   */

static constexpr ctable ctable035 = { { { '|', ign, ign },     /* |   */
                                        { ign, '|', ign },     //  |
                                        { '|', ign, ign } } }; /* |   */

static constexpr ctable ctable036 = { { { '|', ign, ign },     /* |   */
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; /*  |   */

static constexpr ctable ctable037 = { { { ign, ign, '|' },     /*   | */
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; /*  |   */

static constexpr ctable ctable038 = { { { ign, '|', ign },     /*   | */
                                        { '_', '|', ign },     //  _|
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable039 = { { { ign, '/', ign },     /*   / */
                                        { '_', '|', ign },     //  _|
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable040 = { { { ign, '\\', ign },    /*   \ */
                                        { '_', '|', ign },     //  _|
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable041 = { { { ign, '|', ign },     /*   | */
                                        { ign, '|', '_' },     //   |_
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable042 = { { { ign, '/', ign },     /*   / */
                                        { ign, '|', '_' },     //   |_
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable043 = { { { ign, '\\', ign },    /*   \ */
                                        { ign, '|', '_' },     //   |_
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctables[] = { ctable001, ctable002, ctable003, ctable004, ctable005, ctable006, ctable007,
                                      ctable008, ctable009, ctable010, ctable011, ctable012, ctable013, ctable014,
                                      ctable015, ctable016, ctable017, ctable018, ctable019, ctable020, ctable021,
                                      ctable022, ctable023, ctable024, ctable025, ctable026, ctable027, ctable028,
                                      ctable029, ctable030, ctable031, ctable032, ctable033, ctable034, ctable035,
                                      ctable036, ctable037, ctable038, ctable039, ctable040, ctable041, ctable042,
                                      ctable043 };

FramebufferPoint
retroactive_fix(const TerminalBuffer& t, const math::Vec2Int& pos)
{
    // clang-format off
    const ctable inp = { {
        { t.at(pos + math::Vec2Int{ -1, -1 }), t.at(pos + math::Vec2Int{ +0, -1 }), t.at(pos + math::Vec2Int{ +1, -1 }) },
        { t.at(pos + math::Vec2Int{ -1, +0 }), t.at(pos + math::Vec2Int{ +0, +0 }), t.at(pos + math::Vec2Int{ +1, +0 }) },
        { t.at(pos + math::Vec2Int{ -1, +1 }), t.at(pos + math::Vec2Int{ +0, +1 }), t.at(pos + math::Vec2Int{ +1, +1 }) },
    } };
    // clang-format on

    // char c = '@';
    char c = ' ';
    for (std::size_t i = 0; i < sizeof(ctables) / sizeof(ctable); i++) {
        bool match = true;
        for (std::size_t dy = 0; dy < 3; dy++) {
            for (std::size_t dx = 0; dx < 3; dx++) {
                if (dy == 1 && dx == 1) continue;
                if (ctables[i][dy][dx] == ign) continue;

                match &= ctables[i][dy][dx] == inp[dy][dx];
                if (inp[dy][dx] == '_' && dy > 0) {
                    match &= inp[dy - 1][dx] != '_';
                }
            }
        }
        if (match == true) {
            c = ctables[i][1][1];
            break;
        }
    }
    return FramebufferPoint{ pos, TerminalBuffer::Targets{ c, math::Vec3{ 1, 1, 1 } } };
}

std::vector<FramebufferPoint>&
retroactive_fix(const MyUniform& u,
                const TerminalBuffer& t,
                const asciirast::Renderer& r,
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
        const auto frag1 = asciirast::Renderer::apply_scale_to_viewport(r.scale_to_viewport(), frag0);
        const auto frag2 = asciirast::Renderer::apply_screen_to_window(t.screen_to_window(), frag1);
        const auto pos2 = math::Vec2Int{ frag2.pos };
        if (math::Vec2Int{ 0, 0 } <= pos2 - math::Vec2Int{ 1, 1 } && //
            pos2 + math::Vec2Int{ 1, 1 } <= t.size()) {
            out.push_back(retroactive_fix(t, pos2));
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
    math::Rot2D rot;
    math::Float aspect_ratio = framebuffer.aspect_ratio();
    MyUniform uniforms{ aspect_ratio, true, rot };
    std::vector<FramebufferPoint> points;

    asciirast::Renderer r0{ math::AABB2D::from_min_max({ -1, -1 }, { 1, 1 }) };
    asciirast::Renderer r1{ math::AABB2D::from_min_max({ -1, -1 }, { 1, 1 }).size_set(math::Vec2{ 1.5, 1.5 }) };
    asciirast::RendererData<MyVarying> renderer_data{ framebuffer.screen_to_window() };
    asciirast::RendererOptions circle_options;

    circle_options.line_ends_inclusion = asciirast::LineEndsInclusion::IncludeBoth;
    circle_options.line_drawing_direction = asciirast::LineDrawingDirection::Downwards;

    asciirast::RendererOptions line_options;

    line_options.line_ends_inclusion = asciirast::LineEndsInclusion::ExcludeBoth;
    line_options.line_drawing_direction = asciirast::LineDrawingDirection::Downwards;

    std::binary_semaphore sem{ 0 };

    std::thread check_eof_program{ [&sem] {
        while (std::cin.get() != EOF) {
            continue;
        }
        sem.release();
    } };

    while (!sem.try_acquire()) {

        uniforms.draw_horizontal = false; // prefer other chars over '_':

        r0.draw(program, uniforms, circle_buf, framebuffer, renderer_data, circle_options);
        r1.draw(program, uniforms, circle_buf, framebuffer, renderer_data, circle_options);
        r1.draw(program, uniforms, line_buf, framebuffer, renderer_data, line_options);

        uniforms.draw_horizontal = true;
        r0.draw(program, uniforms, circle_buf, framebuffer, renderer_data, circle_options);
        r1.draw(program, uniforms, circle_buf, framebuffer, renderer_data, circle_options);
        r1.draw(program, uniforms, line_buf, framebuffer, renderer_data, line_options);

        for (auto [pos, targets] : retroactive_fix(uniforms, framebuffer, r0, circle_buf.verticies, points)) {
            framebuffer.plot(pos, targets);
        }
        for (auto [pos, targets] : retroactive_fix(uniforms, framebuffer, r1, circle_buf.verticies, points)) {
            framebuffer.plot(pos, targets);
        }

        framebuffer.render();

        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        if (framebuffer.clear_and_update_size()) {
            renderer_data.screen_to_window = framebuffer.screen_to_window();
        }
        aspect_ratio = framebuffer.aspect_ratio();

        rot.stack(math::radians(-10.f));
    }
    check_eof_program.join();
}
