
// Note: Build this in Release mode for smooth viewing

// Test inspired by:
// https://www.youtube.com/watch?v=4s30cnqxJ-0 (javidx9)

#include "examples/common/SDLBuffer.h"

#include <asciirast.hpp>

#include <tiny_obj_loader.hpp>
#include <tinyfiledialogs.hpp>

#include <iostream>

struct MyUniform
{
    asciirast::Texture texture;
    asciirast::Sampler sampler;
    math::Transform2D transform;
};

struct MyVertex
{
    math::Vec2 pos;
    math::Vec2 uv;
};

struct MyVarying
{
    math::Vec2 uv;
};

DERIVE_VARYING_OPS(MyVarying);

class MyProgram
{
    using Fragment = asciirast::Fragment<MyVarying>;
    using ProjectedFragment = asciirast::ProjectedFragment<MyVarying>;

public:
    using Uniform = MyUniform;
    using Vertex = MyVertex;
    using Varying = MyVarying;
    using Targets = SDLBuffer::Targets;
    using FragmentContext = asciirast::FragmentContextType<math::Vec2>;

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        out.pos.xy = { vert.pos };
        out.attrs = { u.transform.apply(vert.uv) };
    }
    auto on_fragment(FragmentContext& context, const Uniform& u, const ProjectedFragment& pfrag, Targets& out) const
            -> asciirast::ProgramTokenGenerator
    {
        const auto color = TEXTURE(context, u.sampler, u.texture, pfrag.attrs.uv);
        out = color.rgba;
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>);

void
handle_events(bool& running,
              math::Vec2& shift,
              math::Float& zoom,
              math::Transform2D& final_transform,
              asciirast::Sampler& sampler)
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
                sampler.mipmap_sample_method = asciirast::SampleMethod::Nearest;
                break;
            case SDLK_0:
                sampler.mipmap_sample_method = asciirast::SampleMethod::Linear;
                break;
            default:
                break;
            }
        }
    }
    if (change_transform) {
        static const math::Transform2D screen_transform = asciirast::SCREEN_BOUNDS.to_transform();
        final_transform = math::Transform2D()
                                  .stack(screen_transform)
                                  .scale(zoom, zoom)
                                  .stack(screen_transform.inversed())
                                  .translate(shift);
    }
}

std::optional<std::string>
find_texture()
{
    const auto default_path = ".";
    const auto patterns = std::to_array<char const*>(
            { "*.jpg", "*.jpeg", "*.png", "*.tga", "*.bmp", "*.psd", "*.gif", "*.hdr", "*.pic", "*.pnm" });
    const char* patterns_desc = nullptr;
    const bool multi_select_enabled = false;
    const char* ptr = tinyfd_openFileDialog("Specify Texture File",
                                            default_path,
                                            patterns.size(),
                                            patterns.data(),
                                            patterns_desc,
                                            multi_select_enabled);

    return ptr ? std::make_optional(ptr) : std::nullopt;
}

std::optional<std::string>
find_ttf()
{
    const auto default_path = ".";
    const auto patterns = std::to_array<char const*>({ "*.ttf" });
    const char* patterns_desc = nullptr;
    const bool multi_select_enabled = false;
    const char* ptr = tinyfd_openFileDialog(
            "Specify .ttf File", default_path, patterns.size(), patterns.data(), patterns_desc, multi_select_enabled);

    return ptr ? std::make_optional(ptr) : std::nullopt;
}

int
main(int argc, char* argv[])
{
    std::string path_to_img;
    std::string path_to_ttf;
    if (argc < 3) {
        const char* program_name = (argc == 1) ? argv[0] : "<program>";
        const char* arg1_str = "path-to-texture";
        const char* arg2_str = "path-to-ttf";

        std::cout << "usage:" << " " << program_name << " <" << arg1_str << "> <" << arg2_str << ">\n";

        if (const auto opt_texture_path = find_texture(); !opt_texture_path.has_value()) {
            std::cerr << "tinyfiledialogs failed. exiting." << "\n";
            return EXIT_FAILURE;
        } else {
            path_to_img = opt_texture_path.value();
            std::cout << "specified " << arg1_str << ": " << path_to_img << "\n";
        }
        if (const auto opt_ttf_path = find_ttf(); !opt_ttf_path.has_value()) {
            std::cerr << "tinyfiledialogs failed. exiting." << "\n";
            return EXIT_FAILURE;
        } else {
            path_to_ttf = opt_ttf_path.value();
            std::cout << "specified " << arg2_str << ": " << path_to_ttf << "\n";
        }
    } else {
        path_to_img = argv[1];
        path_to_ttf = argv[2];
    }

    MyUniform uniforms;
    const unsigned screen_size = 1024;
    const SDLFont font{ path_to_ttf.c_str() };
    uniforms.texture = asciirast::Texture{ path_to_img };
    const math::Float aspect_ratio = uniforms.texture.width() / (math::Float)uniforms.texture.height();
    math::Vec2 shift = { 0, 0 };
    math::Float zoom = 1.f;

    uniforms.sampler.wrap_method = asciirast::WrapMethod::Repeat;
    uniforms.sampler.sample_method = asciirast::SampleMethod::Linear;
    uniforms.sampler.mipmap_sample_method = asciirast::SampleMethod::Linear;

    asciirast::VertexBuffer<MyVertex> vertex_buf{};
    vertex_buf.shape_type = asciirast::ShapeType::TriangleStrip;
    vertex_buf.verticies.push_back(MyVertex{ { -1, -1 }, { 0, 0 } });
    vertex_buf.verticies.push_back(MyVertex{ { +1, -1 }, { 1, 0 } });
    vertex_buf.verticies.push_back(MyVertex{ { -1, +1 }, { 0, aspect_ratio } });
    vertex_buf.verticies.push_back(MyVertex{ { +1, +1 }, { 1, aspect_ratio } });

    SDLBuffer screen(screen_size, screen_size);
    MyProgram program;
    asciirast::Renderer renderer;
    asciirast::RendererData<MyVarying> renderer_data;

    const SDLStaticText text(screen,
                             font,
                             "Sample Method:\n"
                             "    1: Point, 2: Nearest, 3: Linear\n"
                             "Wrap Method:\n"
                             "    4: Blank, 5: Clamp, 6: Periodic, 7: Wrap\n"
                             "Mipmap Sample Method:\n"
                             "    8: Point, 9: Nearest, 0: Linear\n");

    bool running = true;
    while (running) {
        handle_events(running, shift, zoom, uniforms.transform, uniforms.sampler);

        screen.clear();

        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data);
        screen.update();

        text.render();
        screen.show();
    }

    return EXIT_SUCCESS;
}
