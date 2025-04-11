#include "asciirast/framebuffer.h"
#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"

#include <SDL.h>
#include <SDL_pixels.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

// sdl code based on:
// https://stackoverflow.com/questions/33304351/sdl2-fast-pixel-manipulation

namespace math = asciirast::math;

using RGB = math::Vec3;

struct RGBA_uint8
{
    // this order should match SDL pixel format
    std::uint8_t b;
    std::uint8_t g;
    std::uint8_t r;
    std::uint8_t a;
};

class SDLBuffer
{
public:
    using Targets = std::tuple<RGB>;

    SDLBuffer(const unsigned tex_width,
              const unsigned tex_height,
              const int win_width = 512,
              const int win_height = 512)
    {
        m_width = tex_width;
        m_height = tex_height;

        const math::Vec2 scale = { m_width - 1, m_height - 1 };
        m_screen_to_window =
                asciirast::SCREEN_BOUNDS.to_transform().reversed().reflectY().translate(0, 1.f).scale(scale);

        m_rgba_buf.resize(m_width * m_height);
        m_depth_buf.resize(m_width * m_height);

        SDL_Init(SDL_INIT_EVERYTHING);
        m_window = SDL_CreateWindow(
                "SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, win_width, win_height, SDL_WINDOW_SHOWN);
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

        const std::uint32_t pixel_format = SDL_PIXELFORMAT_ARGB8888; // use SDL_GetRendererInfo to get more info
        m_texture = SDL_CreateTexture(m_renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, m_width, m_height);
    }
    ~SDLBuffer()
    {
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    bool test_depth(const math::Vec2Int& pos, math::Float depth)
    {
        assert(0 <= pos.x && (std::size_t)(pos.x) <= m_width);
        assert(0 <= pos.y && (std::size_t)(pos.y) <= m_height);

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        depth = std::clamp(depth, 0.f, 1.f);

        return depth < m_depth_buf[idx];
    }

    const math::Transform2D& screen_to_window() { return m_screen_to_window; }

    void plot(const math::Vec2Int& pos, math::Float depth, const Targets& targets)
    {
        assert(0 <= pos.x && (std::size_t)pos.x < m_width);
        assert(0 <= pos.y && (std::size_t)pos.y < m_height);

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        depth = std::clamp(depth, 0.f, 1.f);

        if (depth < m_depth_buf[idx]) {
            const auto [r, g, b] = std::get<RGB>(targets).array();

            m_rgba_buf[idx].r = static_cast<std::uint8_t>(255.f * r);
            m_rgba_buf[idx].g = static_cast<std::uint8_t>(255.f * g);
            m_rgba_buf[idx].b = static_cast<std::uint8_t>(255.f * b);
            m_rgba_buf[idx].a = 255;
            m_depth_buf[idx] = depth;
        }
    }

    void render() const
    {
        SDL_UpdateTexture(m_texture, nullptr, m_rgba_buf.data(), m_width * 4);
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
    }

    void clear()
    {
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(m_renderer);

        for (std::size_t i = 0; i < m_height * m_width; i++) {
            m_rgba_buf[i] = { .b = 0, .g = 0, .r = 0, .a = 0 };
            m_depth_buf[i] = asciirast::DEFAULT_DEPTH;
        }
    }

private:
    std::size_t index(const std::size_t y, const std::size_t x) const { return m_width * y + x; }

    std::size_t m_width;
    std::size_t m_height;
    std::vector<RGBA_uint8> m_rgba_buf;
    std::vector<math::Float> m_depth_buf;
    math::Transform2D m_screen_to_window;

    SDL_Texture* m_texture = nullptr;
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};

static_assert(asciirast::FrameBufferInterface<SDLBuffer>);

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

    MyVarying operator+(const MyVarying& that) const { return { this->color + that.color }; }
    MyVarying operator*(const math::Float scalar) const { return { this->color * scalar }; }
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
    using Targets = SDLBuffer::Targets;

    Fragment on_vertex(const Uniform& u, const Vertex& vert) const
    {
        (void)u;
        return Fragment{ .pos = math::Vec4{ vert.pos.x, vert.pos.y, 0, 1 }, // w should be 1 for 2D.
                         .attrs = Varying{ vert.color } };
    }
    Targets on_fragment(const Uniform& u, const ProjectedFragment& pfrag) const
    {
        (void)u;
        return { pfrag.attrs.color };
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>);

int
main(int, char**)
{
    const MyVertex v0 = { .pos = { -0.5f, -0.5f, 1.f }, .color = { 1.f, 0.f, 0.f } };
    const MyVertex v1 = { .pos = { +0.0f, +0.5f, 1.f }, .color = { 0.f, 1.f, 0.f } };
    const MyVertex v2 = { .pos = { +0.5f, -0.5f, 1.f }, .color = { 0.f, 0.f, 1.f } };

    asciirast::VertexBuffer<MyVertex> vb;
    vb.shape_type = asciirast::ShapeType::TRIANGLES;
    vb.verticies = { v0, v1, v2 };

    MyProgram program;
    asciirast::Renderer<MyVarying> renderer;
    SDLBuffer screen(512, 512);
    MyUniform u{};

    bool running = true;
    while (running) {
        screen.clear();

        // handle events
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if ((SDL_QUIT == ev.type) || (SDL_KEYDOWN == ev.type && SDL_SCANCODE_ESCAPE == ev.key.keysym.scancode)) {
                running = false;
                break;
            }
        }

        renderer.draw(program, u, vb, screen);
        screen.render();
    }

    return 0;
}
