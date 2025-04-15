
// Note: Build this in Release mode for smooth viewing

// Test inspired by:
// https://www.youtube.com/watch?v=4s30cnqxJ-0 (javidx9)

#include "asciirast/framebuffer.h"
#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"
#include "asciirast/sampler.h"

#include <SDL.h>
#include <SDL_pixels.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace math = asciirast::math;

using RGBA = math::Vec4;

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
    using Targets = std::tuple<RGBA>;

    SDLBuffer(const unsigned tex_width,
              const unsigned tex_height,
              const int win_width = 512,
              const int win_height = 512)
    {
        m_width = tex_width;
        m_height = tex_height;

        m_screen_to_window = asciirast::Renderer::SCREEN_BOUNDS //
                                     .to_transform()
                                     .reversed()
                                     .reflectY()
                                     .translate(0, 1.f)
                                     .scale(m_width - 1, m_height - 1);

        m_rgba_buf.resize(m_width * m_height);
        m_depth_buf.resize(m_width * m_height);

        SDL_Init(SDL_INIT_EVERYTHING);
        m_window = SDL_CreateWindow(
                "SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, win_width, win_height, SDL_WINDOW_SHOWN);
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

        const std::uint32_t pixel_format = SDL_PIXELFORMAT_ARGB8888; // use SDL_GetRendererInfo to get more info
        m_texture = SDL_CreateTexture(m_renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, m_width, m_height);
        SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
    }
    ~SDLBuffer()
    {
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    bool test_and_set_depth(const math::Vec2Int& pos, math::Float depth)
    {
        assert(0 <= pos.x && (std::size_t)(pos.x) <= m_width);
        assert(0 <= pos.y && (std::size_t)(pos.y) <= m_height);

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        depth = std::clamp<math::Float>(depth, 0, 1);

        if (depth < m_depth_buf[idx]) {
            m_depth_buf[idx] = depth;
            return true;
        }
        return false;
    }

    const math::Transform2D& screen_to_window() { return m_screen_to_window; }

    void plot(const math::Vec2Int& pos, const Targets& targets)
    {
        assert(0 <= pos.x && (std::size_t)pos.x < m_width);
        assert(0 <= pos.y && (std::size_t)pos.y < m_height);

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        const auto [r, g, b, a] = std::get<RGBA>(targets).array();

        m_rgba_buf[idx].r = static_cast<std::uint8_t>(255.f * r);
        m_rgba_buf[idx].g = static_cast<std::uint8_t>(255.f * g);
        m_rgba_buf[idx].b = static_cast<std::uint8_t>(255.f * b);
        m_rgba_buf[idx].a = static_cast<std::uint8_t>(255.f * a);
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
            m_depth_buf[i] = 2; // or +infty
        }
    }

private:
    std::size_t index(const std::size_t y, const std::size_t x) const { return m_width * y + x; }

    std::size_t m_width;
    std::size_t m_height;
    math::Transform2D m_screen_to_window;

    std::vector<RGBA_uint8> m_rgba_buf;
    std::vector<math::Float> m_depth_buf;

    SDL_Texture* m_texture = nullptr;
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};

static_assert(asciirast::FrameBufferInterface<SDLBuffer>);

struct MyUniform
{
    const asciirast::Sampler<>& sampler;
    const math::Transform2D& transform;
};

struct MyVertex
{
    math::Vec2 pos;
    math::Vec2 uv;
};

struct MyVarying
{
    math::Vec2 uv;

    MyVarying operator+(const MyVarying& that) const { return { this->uv + that.uv }; }
    MyVarying operator*(const math::Float scalar) const { return { this->uv * scalar }; }
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
        return Fragment{ .pos = math::Vec4{ vert.pos, 0, 1 }, .attrs = Varying{ u.transform.apply(vert.uv) } };
    }
    Targets on_fragment(const Uniform& u, const ProjectedFragment& pfrag) const
    {
        return { asciirast::texture(u.sampler, pfrag.attrs.uv) };
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>);

void
handle_events(bool& running,
              math::Vec2& shift,
              math::Float& zoom,
              math::Transform2D& final_transform,
              asciirast::Sampler<>& sampler)
{
    const auto shf_factor = 0.05f;
    const auto zoom_factor = 1.05f;

    bool change_transform = false;

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if ((SDL_QUIT == ev.type) || (SDL_KEYDOWN == ev.type && SDL_SCANCODE_ESCAPE == ev.key.keysym.scancode)) {
            running = false;
            break;
        }
        if (SDL_KEYDOWN == ev.type) {
            switch (ev.key.keysym.sym) {
            case SDLK_LEFT:
                shift.x -= shf_factor;
                change_transform = true;
                break;
            case SDLK_RIGHT:
                shift.x += shf_factor;
                change_transform = true;
                break;
            case SDLK_UP:
                shift.y += shf_factor;
                change_transform = true;
                break;
            case SDLK_DOWN:
                shift.y -= shf_factor;
                change_transform = true;
                break;
            case SDLK_PLUS:
                zoom /= zoom_factor;
                change_transform = true;
                break;
            case SDLK_MINUS:
                zoom *= zoom_factor;
                change_transform = true;
                break;
            case SDLK_1:
                sampler.sample_method = asciirast::SampleMethod::Point;
                break;
            case SDLK_2:
                sampler.sample_method = asciirast::SampleMethod::Nearest;
                break;
            case SDLK_3:
                sampler.sample_method = asciirast::SampleMethod::Linear;
                break;
            case SDLK_4:
                sampler.wrap_method = asciirast::WrapMethod::Blank;
                break;
            case SDLK_5:
                sampler.wrap_method = asciirast::WrapMethod::Clamp;
                break;
            case SDLK_6:
                sampler.wrap_method = asciirast::WrapMethod::Periodic;
                break;
            case SDLK_7:
                sampler.wrap_method = asciirast::WrapMethod::Repeat;
                break;
            case SDLK_8:
                sampler.mipmap_sample_method = asciirast::SampleMethod::Point;
                break;
            case SDLK_9:
                sampler.mipmap_sample_method = asciirast::SampleMethod::Linear;
                break;
            case SDLK_0:
                sampler.mipmap_sample_method = asciirast::SampleMethod::Nearest;
                break;
            default:
                break;
            }
        }
    }
    if (change_transform) {
        static const math::Transform2D screen_transform = asciirast::Renderer::SCREEN_BOUNDS.to_transform();
        final_transform = math::Transform2D()
                                  .stack(screen_transform)
                                  .scale(zoom, zoom)
                                  .stack(screen_transform.reversed())
                                  .translate(shift);
    }
}

int
main(int argc, char* argv[])
{
    if (argc < 2) {
        const char* program_name = (argc == 1) ? argv[0] : "<program>";
        std::cout << "usage:" << " " << program_name << " " << "<path-to-png>";
        return EXIT_FAILURE;
    }
    const char* path_to_obj = argc >= 2 ? argv[1] : "";
    // const char* path_to_obj = "examples/data/texture_test.png";

#ifndef NDEBUG
    const unsigned screen_size = 256;
#else
    const unsigned screen_size = 1024;
#endif

    std::filesystem::create_directory("images");
    const auto image_path = std::filesystem::path("images");
    const asciirast::Texture texture{ path_to_obj };
    const math::Float aspect_ratio = texture.width() / (math::Float)texture.height();
    math::Vec2 shift = { 0, 0 };
    math::Float zoom = 1.f;
    math::Transform2D final_transform;
    asciirast::Sampler sampler{ texture };
    const MyUniform uniforms{ sampler, final_transform};

    sampler.sample_method = asciirast::SampleMethod::Linear;
    sampler.wrap_method = asciirast::WrapMethod::Repeat;
    sampler.mipmap_sample_method = asciirast::SampleMethod::Nearest;

    asciirast::VertexBuffer<MyVertex> vertex_buf{};
    vertex_buf.shape_type = asciirast::ShapeType::TriangleStrip;
    vertex_buf.verticies.push_back(MyVertex{ { -1, -1 }, { 0, 0 } });
    vertex_buf.verticies.push_back(MyVertex{ { +1, -1 }, { 1, 0 } });
    vertex_buf.verticies.push_back(MyVertex{ { -1, +1 }, { 0, aspect_ratio } });
    vertex_buf.verticies.push_back(MyVertex{ { +1, +1 }, { 1, aspect_ratio } });

    SDLBuffer screen(screen_size, screen_size, 1024, 1024);
    MyProgram program;
    asciirast::Renderer renderer;
    asciirast::RendererData<MyVarying> renderer_data{ screen.screen_to_window() };

    bool running = true;
    while (running) {
        screen.clear();

        handle_events(running, shift, zoom, final_transform, sampler);

        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data);

        screen.render();
    }

    return EXIT_SUCCESS;
}
