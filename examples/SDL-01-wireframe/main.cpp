
// On the obj file:
// - https://www.youtube.com/watch?v=iClme2zsg3I
// - https://github.com/tinyobjloader/tinyobjloader

// based on:
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-1:-Bresenham%E2%80%99s-Line-Drawing-Algorithm

#include "../common/SDLBuffer.hpp"

#include <asciirast.hpp>

#include <tiny_obj_loader.h>
#include <tinyfiledialogs.h>

#include <iostream>
#include <ranges>
#include <vector>

struct MyUniform
{
    bool lhs_side = true;
    int split_x = 0;
    math::Rot3D rot;
};

struct MyVertex
{
    math::Vec3 pos;
};

struct MyVarying
{
    RGB color;
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
    using FragmentContext = asciirast::FragmentContextGeneric<>;

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        out.pos.xy = { (u.rot.to_mat() * vert.pos).xy };
        out.attrs = { math::Vec3{ 1, 1, 1 } };
    }
    auto on_fragment([[maybe_unused]] FragmentContext& c,
                     [[maybe_unused]] const Uniform& u,
                     const ProjectedFragment& pfrag,
                     Targets& out) const -> asciirast::FragmentTokenGenerator
    {
#ifndef NDEBUG
        out = { pfrag.attrs.color.rgb, 1 };
        co_return;
#else
        if ((u.lhs_side && pfrag.pos.x <= u.split_x) || (!u.lhs_side && pfrag.pos.x > u.split_x)) {
            out = { pfrag.attrs.color.rgb, 1 };
            co_yield asciirast::FragmentToken::Keep;
        } else {
            co_yield asciirast::FragmentToken::Discard;
        }
#endif
    }
};

static_assert(asciirast::ProgramInterface<MyProgram>);

void
handle_events(bool& running, int& split_x)
{
    int split_y;
    [[maybe_unused]] uint32_t mouse_state = SDL_GetMouseState(&split_x, &split_y);

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

        std::cout << "usage:"
                  << " " << program_name << " "
                  << "<" << arg1_str << ">\n";
        std::cout << "specified " << arg1_str << ": " << std::flush;

        if (const auto opt_path = find_obj(); !opt_path.has_value()) {
            std::cout << "\n";
            std::cerr << "tinyfiledialogs failed. exiting."
                      << "\n";
            return EXIT_FAILURE;
        } else {
            path_to_obj = opt_path.value();
            std::cout << path_to_obj << "\n";
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
    vertex_buf.shape_type = asciirast::ShapeType::Lines;
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

            for (std::size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx0 = shapes[s].mesh.indices[index_offset + v];
                tinyobj::index_t idx1 = shapes[s].mesh.indices[index_offset + ((v + 1) % fv)];

                vertex_buf.indicies.push_back(static_cast<std::size_t>(idx0.vertex_index));
                vertex_buf.indicies.push_back(static_cast<std::size_t>(idx1.vertex_index));
            }

            index_offset += fv;
        }
    }

    SDLClock clock;
    SDLBuffer screen(512, 512);
    screen.brush_extent = 2;
    MyProgram program;
    asciirast::Renderer<{ .attr_interpolation = asciirast::AttrInterpolation::Flat }> renderer;
    asciirast::RendererData<MyVarying> renderer_data;
    MyUniform uniforms;

    bool running = true;
    while (running) {
        handle_events(running, uniforms.split_x);

        clock.update([&]([[maybe_unused]] float dt_sec) {
#ifdef NDEBUG
            uniforms.rot.rotateZX(1 * dt_sec);
#endif
        });

        screen.clear();

#ifndef NDEBUG
        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data);
#else
        screen.custom_brush_enabled = true;
        vertex_buf.shape_type = asciirast::ShapeType::Points;
        uniforms.lhs_side = false;
        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data);

        screen.custom_brush_enabled = false;
        vertex_buf.shape_type = asciirast::ShapeType::Lines;
        uniforms.lhs_side = true;
        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data);
#endif

        screen.render();

        clock.tick();
    }

    return EXIT_SUCCESS;
}
