
// based on:
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling

// <random>:
// https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution

#include "examples/common/SDLBuffer.h"

#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"

#include "external/tiny_obj_loader/tiny_obj_loader.h"
#include "external/tinyfiledialogs/tinyfiledialogs.h"

#include <iostream>
#include <random>
#include <ranges>
#include <vector>

struct MyUniform
{
    math::Rot3D rot;
    math::Float z_near = 0.1f;
    math::Float z_dist;
};

struct MyVertex
{
    math::Vec3 pos;
    RGB color;
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
        const auto transform = math::Transform3D()
                                       .rotate(u.rot)
                                       .translate({ 0, 0, 2 })
                                       .stack(asciirast::make_orthographic(u.z_near, u.z_near + u.z_dist + 4));

        out.pos = { transform.apply(vert.pos), 1 };
        out.attrs = { vert.color };
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

        std::cout << "usage:" << " " << program_name << " " << "<" << arg1_str << ">\n";

        if (const auto opt_path = find_obj(); !opt_path.has_value()) {
            std::cerr << "tinyfiledialogs failed. exiting." << "\n";
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

    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0., 1.);

    asciirast::VertexBuffer<MyVertex> vertex_buf{};
    vertex_buf.shape_type = asciirast::ShapeType::Triangles;
    std::vector<math::Vec3> positions =
            attrib.vertices                                                                    //
            | std::ranges::views::take(attrib.vertices.size() - (attrib.vertices.size() % 3U)) //
            | std::ranges::views::chunk(3U)                                                    //
            | std::ranges::views::transform([](auto&& range) {                                 //
                  return math::Vec3{ *range.cbegin(), *(range.cbegin() + 1), *(range.cbegin() + 2) };
              }) //
            | std::ranges::to<decltype(positions)>();

    for (std::size_t s = 0; s < shapes.size(); s++) {
        std::size_t index_offset = 0;
        for (std::size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            const std::size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            if (fv == 3) {
                const auto color = math::Vec3{ dis(gen), dis(gen), dis(gen) };

                tinyobj::index_t idx0 = shapes[s].mesh.indices[index_offset + 0];
                tinyobj::index_t idx1 = shapes[s].mesh.indices[index_offset + 1];
                tinyobj::index_t idx2 = shapes[s].mesh.indices[index_offset + 2];

                const auto p0 = positions[static_cast<std::size_t>(idx0.vertex_index)];
                const auto p1 = positions[static_cast<std::size_t>(idx1.vertex_index)];
                const auto p2 = positions[static_cast<std::size_t>(idx2.vertex_index)];

                vertex_buf.verticies.push_back(MyVertex{ { p0.xy, -p0.z }, color });
                vertex_buf.verticies.push_back(MyVertex{ { p1.xy, -p1.z }, color });
                vertex_buf.verticies.push_back(MyVertex{ { p2.xy, -p2.z }, color });
            }

            index_offset += fv;
        }
    }
    SDLClock clock;
    SDLBuffer screen(512, 512);
    MyProgram program;
    asciirast::Renderer<{ .winding_order = asciirast::WindingOrder::CounterClockwise }> renderer;
    asciirast::RendererData<MyVarying> renderer_data{ screen.screen_to_window() };
    MyUniform uniforms;
    uniforms.z_dist = std::abs(
            std::ranges::fold_left(vertex_buf.verticies | std::ranges::views::transform(
                                                                  [](const MyVertex& vert) { return vert.pos.z; }),
                                   math::Float{},
                                   [](math::Float lhs, math::Float rhs) { return std::max(lhs, rhs); }) -
            std::ranges::fold_left(vertex_buf.verticies | std::ranges::views::transform(
                                                                  [](const MyVertex& vert) { return vert.pos.z; }),
                                   math::Float{},
                                   [](math::Float lhs, math::Float rhs) { return std::min(lhs, rhs); }));

    bool running = true;
    while (running) {
        handle_events(running);

        clock.update([&]([[maybe_unused]] float dt_sec) {
#ifdef NDEBUG
            uniforms.rot.rotateZX(1 * dt_sec);
#endif
        });

        screen.clear();
        vertex_buf.shape_type = asciirast::ShapeType::Lines;
        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data);
        vertex_buf.shape_type = asciirast::ShapeType::Triangles;
        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data);
        screen.render();

        clock.tick();
    }

    return EXIT_SUCCESS;
}
