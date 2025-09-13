
// based on:
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-3:-Hidden-faces-removal-(z-buffer)

#include "../common/SDLBuffer.hpp"

#include <asciirast.hpp>

#include <tiny_obj_loader.h>
#include <tinyfiledialogs.h>

#include <iostream>
#include <ranges>
#include <vector>

struct MyUniform
{
    math::Rot3D rot;
    math::Float z_dist;
    math::Float z_near = 0.1f;
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

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        const auto transform = math::Transform3D().rotate(u.rot).translate({ 0, 0, 4 });
        const auto pos = transform.apply(vert.pos);
        const auto depth = asciirast::compute_reverse_depth_linear(pos.z, u.z_near, u.z_near + u.z_dist + 4);

        out.pos = { pos.xy, depth, 1 };
        out.attrs = { math::Vec3::from_value(depth) };
    }
    void on_fragment([[maybe_unused]] const Uniform& u, const ProjectedFragment& pfrag, Targets& out) const
    {
        out = { pfrag.attrs.color.rgb, 1 };
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
            std::cout << "specified " << arg1_str << ": " << path_to_obj << "\n";
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
    vertex_buf.verticies = attrib.vertices                                                                    //
                           | std::ranges::views::take(attrib.vertices.size() - (attrib.vertices.size() % 3U)) //
                           | std::ranges::views::chunk(3U)                                                    //
                           | std::ranges::views::transform([](auto&& range) {                                 //
                                 const math::Vec3 p = { *range.cbegin(), *(range.cbegin() + 1), *(range.cbegin() + 2) };
                                 return MyVertex{ { p.xy, -p.z } };
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

    SDLClock clock;
    MyUniform uniforms;
    uniforms.z_dist = std::abs(
        std::ranges::fold_left(vertex_buf.verticies |
                                   std::ranges::views::transform([](const MyVertex& vert) { return vert.pos.z; }),
                               math::Float{},
                               [](math::Float lhs, math::Float rhs) { return std::max(lhs, rhs); }) -
        std::ranges::fold_left(vertex_buf.verticies |
                                   std::ranges::views::transform([](const MyVertex& vert) { return vert.pos.z; }),
                               math::Float{},
                               [](math::Float lhs, math::Float rhs) { return std::min(lhs, rhs); }));

    SDLBuffer screen(512, 512);
    MyProgram program;
    asciirast::Renderer<{ .winding_order = asciirast::WindingOrder::CounterClockwise }> renderer;
    asciirast::RendererData<MyVarying> renderer_data;

    bool running = true;
    while (running) {
        handle_events(running);

        clock.update([&]([[maybe_unused]] float dt_sec) {
#ifdef NDEBUG
            uniforms.rot.rotateZX(1 * dt_sec);
#endif
        });

        screen.clear();
        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data);
        screen.render();

        clock.tick();
    }

    return EXIT_SUCCESS;
}
