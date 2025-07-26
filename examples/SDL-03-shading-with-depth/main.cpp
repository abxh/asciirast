
// based on:
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-3:-Hidden-faces-removal-(z-buffer)

#include "asciirast/framebuffer.h"
#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"

#include <SDL.h>
#include <SDL_pixels.h>

#include "external/tiny_obj_loader/tiny_obj_loader.h"
#include "external/tinyfiledialogs/tinyfiledialogs.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ranges>
#include <vector>

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
        // SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
    }
    ~SDLBuffer()
    {
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    bool test_and_set_depth(const math::Vec2Int& pos, math::Float depth)
    {
        assert(0 <= pos.x && (std::size_t)(pos.x) < m_width);
        assert(0 <= pos.y && (std::size_t)(pos.y) < m_height);

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
        const auto [r, g, b] = std::get<RGB>(targets).array();

        m_rgba_buf[idx].r = static_cast<std::uint8_t>(255.f * r);
        m_rgba_buf[idx].g = static_cast<std::uint8_t>(255.f * g);
        m_rgba_buf[idx].b = static_cast<std::uint8_t>(255.f * b);
        m_rgba_buf[idx].a = 255;
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

class SDLClock
{
public:
    SDLClock(int ms_per_update = 100)
    {
        m_previous_time_ms = SDL_GetTicks64();
        m_lag_ms = 0;
        m_ms_per_update = ms_per_update;
    }

    template<typename F>
        requires(std::invocable<F, float>)
    void update(F callback)
    {
        while (m_lag_ms >= 0) {
            const auto dt_sec = m_ms_per_update / 1000.f;
            callback(dt_sec);
            m_lag_ms -= m_ms_per_update;
        }
    }

    void tick()
    {
        const uint64_t current_time_ms = SDL_GetTicks64();
        const int elapsed_ms = static_cast<int>(current_time_ms - m_previous_time_ms);
        m_previous_time_ms = current_time_ms;
        m_lag_ms += elapsed_ms;
    }

private:
    std::uint64_t m_previous_time_ms;
    int m_lag_ms;
    int m_ms_per_update;
};

struct MyUniform
{
    const math::Rot3D& rot;
    math::Float z_near;
    math::Float z_far;
};

struct MyVertex
{
    math::Vec3 pos;
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
    using FragmentContext = asciirast::FragmentContextType<>;
    using ProgramTokenGenerator = std::generator<asciirast::ProgramToken>;

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        const auto pos = u.rot.to_mat() * vert.pos;

        const auto depth_scalar = u.z_far / (u.z_far - u.z_near);
        const auto depth = pos.z * depth_scalar - u.z_near * depth_scalar;

        out.pos = { pos.xy, 1.f - depth, 1 };
        out.attrs = { math::Vec3{ depth, depth, depth } };
    }
    auto on_fragment(FragmentContext&,
                     [[maybe_unused]] const Uniform& u,
                     const ProjectedFragment& pfrag,
                     Targets& out) const -> ProgramTokenGenerator
    {
        out = { pfrag.attrs.color };
        co_return;
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>);

void
handle_events(bool& running)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if ((SDL_QUIT == ev.type) || (SDL_KEYDOWN == ev.type && SDL_SCANCODE_ESCAPE == ev.key.keysym.scancode)) {
            running = false;
            break;
        }
    }
}

std::optional<std::string>
find_obj()
{
    const auto default_path = ".";
    const auto patterns = std::to_array<char const*>({ "*.obj" });
    const char* patterns_desc = nullptr;
    const bool multi_select_enabled = false;
    const char* ptr = tinyfd_openFileDialog(
            "Specify .obj File", default_path, patterns.size(), patterns.data(), patterns_desc, multi_select_enabled);

    return ptr ? std::make_optional(ptr) : std::nullopt;
}

int
main(int argc, char* argv[])
{
    std::string path_to_obj;
    if (argc < 2) {
        const char* program_name = (argc == 1) ? argv[0] : "<program>";
        const char* arg1_str = "path-to-obj";

        std::cout << "usage:" << " " << program_name << " " << "<" << arg1_str << ">\n";

        if (const auto opt_path = find_obj(); !opt_path.has_value()) {
            std::cerr << "tinyfiledialogs failed. exiting." << "\n";
            return EXIT_FAILURE;
        } else {
            path_to_obj = opt_path.value();
            std::cerr << "specified " << arg1_str << ": " << path_to_obj << "\n";
        }
    } else {
        path_to_obj = argv[1];
    }

    tinyobj::ObjReader obj_reader;
    if (!obj_reader.ParseFromFile(path_to_obj)) {
        if (!obj_reader.Error().empty()) {
            std::cerr << "tinyobj::ObjReader : " << obj_reader.Error();
        }
        return EXIT_FAILURE;
    } else if (!obj_reader.Warning().empty()) {
        std::cout << "tinyobj::ObjReader : " << obj_reader.Warning();
    }

    const tinyobj::attrib_t& attrib = obj_reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = obj_reader.GetShapes();

    asciirast::IndexedVertexBuffer<MyVertex> vertex_buf{};
    vertex_buf.shape_type = asciirast::ShapeType::Triangles;
    vertex_buf.verticies =
            attrib.vertices                                                                    //
            | std::ranges::views::take(attrib.vertices.size() - (attrib.vertices.size() % 3U)) //
            | std::ranges::views::chunk(3U)                                                    //
            | std::ranges::views::transform([](auto&& range) {                                 //
                  return MyVertex{ math::Vec3{ *range.cbegin(), *(range.cbegin() + 1), *(range.cbegin() + 2) } };
              }) //
            | std::ranges::to<decltype(vertex_buf.verticies)>();
    for (std::size_t s = 0; s < shapes.size(); s++) {
        std::size_t index_offset = 0;
        for (std::size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            const std::size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            if (fv == 3) {
                tinyobj::index_t idx0 = shapes[s].mesh.indices[index_offset + 0];
                tinyobj::index_t idx1 = shapes[s].mesh.indices[index_offset + 1];
                tinyobj::index_t idx2 = shapes[s].mesh.indices[index_offset + 2];

                vertex_buf.indicies.push_back(static_cast<std::size_t>(idx0.vertex_index));
                vertex_buf.indicies.push_back(static_cast<std::size_t>(idx1.vertex_index));
                vertex_buf.indicies.push_back(static_cast<std::size_t>(idx2.vertex_index));
            }

            index_offset += fv;
        }
    }

    math::Rot3D rot;
    SDLClock clock;
    MyUniform uniforms{ rot, {}, {} };
    uniforms.z_near = std::ranges::fold_left(
            vertex_buf.verticies | std::ranges::views::transform([](const MyVertex& vert) { return vert.pos.z; }),
            math::Float{},
            [](math::Float lhs, math::Float rhs) { return std::min(lhs, rhs); });
    uniforms.z_far = std::ranges::fold_left(
            vertex_buf.verticies | std::ranges::views::transform([](const MyVertex& vert) { return vert.pos.z; }),
            math::Float{},
            [](math::Float lhs, math::Float rhs) { return std::max(lhs, rhs); });

    SDLBuffer screen(512, 512);
    MyProgram program;
    asciirast::Renderer renderer;
    asciirast::RendererData<MyVarying> renderer_data{ screen.screen_to_window() };
    asciirast::RendererOptions renderer_options{ .winding_order = asciirast::WindingOrder::CounterClockwise };

    bool running = true;
    while (running) {
        handle_events(running);

        clock.update([&]([[maybe_unused]] float dt_sec) {
#ifdef NDEBUG
            rot.rotateXZ(-1.f * dt_sec);
#endif
        });

        screen.clear();
        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data, renderer_options);
        screen.render();

        clock.tick();
    }

    return EXIT_SUCCESS;
}
