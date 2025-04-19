
// based on latter part of:
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-3:-Hidden-faces-removal-(z-buffer)

// and:
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-4:-Perspective-projection

#include "asciirast/framebuffer.h"
#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"
#include "asciirast/sampler.h"

#include <SDL.h>
#include <SDL_pixels.h>

#include "external/tiny_obj_loader/tiny_obj_loader.h"

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

struct MyUniform
{
    const asciirast::Sampler<>& sampler;
    math::Float z_near = 0.1f;
    math::Float z_far = 100.f;
};

struct MyVertex
{
    math::Vec3 pos;
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
    using Result = asciirast::FragmentResult<typename SDLBuffer::Targets>;

public:
    // alias to fullfill program interface:
    using Uniform = MyUniform;
    using Vertex = MyVertex;
    using Varying = MyVarying;
    using Targets = SDLBuffer::Targets;
    using FragmentContext = asciirast::FragmentContextType<math::Vec2>;

    Fragment on_vertex(const Uniform& u, const Vertex& vert) const
    {
        static const auto depth_scalar = u.z_far / (u.z_far - u.z_near);
        const auto depth = vert.pos.z * depth_scalar - u.z_near * depth_scalar;

        static const auto fov_angle = math::radians(60.f);
        static const auto fov_scalar = tanf(fov_angle / 2.f);
        static const auto fov_scalar_inv = 1 / fov_scalar;

        const auto z_shf = 2.f;

        return Fragment{ .pos = math::Vec4{ fov_scalar_inv * vert.pos.x,
                                            fov_scalar_inv * vert.pos.y,
                                            u.z_far - depth,
                                            -vert.pos.z + z_shf },
                         .attrs = Varying{ vert.uv } };
    }

    std::generator<Result> on_fragment(FragmentContext& context, const Uniform& u, const ProjectedFragment& pfrag) const
    {
        co_yield asciirast::texture_init(context, u.sampler, pfrag.attrs.uv, std::type_identity<Targets>());
        co_yield Targets{ asciirast::texture(context, u.sampler, pfrag.attrs.uv) };
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

int
main(int argc, char* argv[])
{
    if (argc < 3) {
        const char* program_name = (argc == 1) ? argv[0] : "<program>";
        std::cout << "usage:" << " " << program_name << " " << "<path-to-obj>" << " " << "<path-to-tga>\n";
        return EXIT_FAILURE;
    }
    const char* path_to_obj = argc >= 3 ? argv[1] : ""; // examples/data/african_head.obj
    const char* path_to_tga = argc >= 3 ? argv[2] : ""; // examples/data/african_head_diffuse.tga

    tinyobj::ObjReader obj_reader;
    if (!obj_reader.ParseFromFile(path_to_obj)) {
        if (!obj_reader.Error().empty()) {
            std::cerr << "tinyobj::ObjReader : " << obj_reader.Error();
        }
        return EXIT_FAILURE;
    } else if (!obj_reader.Warning().empty()) {
        std::cout << "tinyobj::ObjReader : " << obj_reader.Warning();
    }
    const auto texture = asciirast::Texture(path_to_tga);
    const asciirast::Sampler sampler{ texture };

    const tinyobj::attrib_t& attrib = obj_reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = obj_reader.GetShapes();

    asciirast::VertexBuffer<MyVertex> vertex_buf{};
    vertex_buf.shape_type = asciirast::ShapeType::Triangles;

    if (true) {
        const auto positions = attrib.vertices                                                                    //
                               | std::ranges::views::take(attrib.vertices.size() - (attrib.vertices.size() % 3U)) //
                               | std::ranges::views::chunk(3U)                                                    //
                               | std::ranges::views::transform([](auto&& range) {                                 //
                                     return math::Vec3{ *range.cbegin(), *(range.cbegin() + 1), *(range.cbegin() + 2) };
                                 }) //
                               | std::ranges::to<std::vector<math::Vec3>>();

        const auto texcoords = attrib.texcoords                                                                     //
                               | std::ranges::views::take(attrib.texcoords.size() - (attrib.texcoords.size() % 2U)) //
                               | std::ranges::views::chunk(2U)                                                      //
                               | std::ranges::views::transform([](auto&& range) {                                   //
                                     return math::Vec2{ *range.cbegin(), *(range.cbegin() + 1) };
                                 }) |
                               std::ranges::to<std::vector<math::Vec2>>();

        for (std::size_t s = 0; s < shapes.size(); s++) {
            std::size_t index_offset = 0;
            for (std::size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                const std::size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                if (fv == 3) {
                    const tinyobj::index_t idx0 = shapes[s].mesh.indices[index_offset + 0];
                    const tinyobj::index_t idx1 = shapes[s].mesh.indices[index_offset + 1];
                    const tinyobj::index_t idx2 = shapes[s].mesh.indices[index_offset + 2];

                    const auto pi0 = static_cast<std::size_t>(idx0.vertex_index);
                    const auto pi1 = static_cast<std::size_t>(idx1.vertex_index);
                    const auto pi2 = static_cast<std::size_t>(idx2.vertex_index);

                    const auto tci0 = static_cast<std::size_t>(idx0.texcoord_index);
                    const auto tci1 = static_cast<std::size_t>(idx1.texcoord_index);
                    const auto tci2 = static_cast<std::size_t>(idx2.texcoord_index);

                    vertex_buf.verticies.push_back(MyVertex{ positions[pi0], texcoords[tci0] });
                    vertex_buf.verticies.push_back(MyVertex{ positions[pi1], texcoords[tci1] });
                    vertex_buf.verticies.push_back(MyVertex{ positions[pi2], texcoords[tci2] });
                }

                index_offset += fv;
            }
        }
    } else {
        vertex_buf.verticies.push_back(MyVertex{ { -1, -1 }, { 0, 0 } });
        vertex_buf.verticies.push_back(MyVertex{ { +1, -1 }, { 1, 0 } });
        vertex_buf.verticies.push_back(MyVertex{ { -1, +1 }, { 0, 1 } });

        vertex_buf.verticies.push_back(MyVertex{ { -1, +1 }, { 0, 1 } });
        vertex_buf.verticies.push_back(MyVertex{ { +1, -1 }, { 1, 0 } });
        vertex_buf.verticies.push_back(MyVertex{ { +1, +1 }, { 1, 1 } });
    }
    MyUniform uniforms{ sampler };
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
    asciirast::RendererOptions<asciirast::WindingOrder::CounterClockwise> renderer_options;

    bool running = true;
    while (running) {
        screen.clear();

        handle_events(running);

        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data, renderer_options);

        screen.render();
    }

    return EXIT_SUCCESS;
}
