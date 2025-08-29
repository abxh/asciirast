
// based on latter part of:
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-3:-Hidden-faces-removal-(z-buffer)

// and:
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-4:-Perspective-projection

#include "examples/common/SDLBuffer.h"

#include "asciirast/math/types.h"
#include "asciirast/program.h"
#include "asciirast/renderer.h"
#include "asciirast/sampler.h"

#include "external/tiny_obj_loader/tiny_obj_loader.h"
#include "external/tinyfiledialogs/tinyfiledialogs.h"

#include <iostream>
#include <ranges>
#include <vector>

struct MyUniform
{
    asciirast::Texture texture;
    asciirast::Sampler sampler;
    math::Rot3D rot;
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
};

DERIVE_VARYING_OPS(MyVarying);

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
    using FragmentContext = asciirast::FragmentContextType<math::Vec2>;

    void on_vertex(const Uniform& u, const Vertex& vert, Fragment& out) const
    {
        const auto transform = math::Transform3D()
                                       .rotate(u.rot)
                                       .translate({ 0, 0, 2 })
                                       .stack(asciirast::make_perspective(u.z_near, u.z_far));

        out.pos = transform.apply({ vert.pos, 1 });
        out.attrs = { vert.uv };
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
find_tga()
{
    const auto default_path = ".";
    const auto patterns = std::to_array<char const*>({ "*.tga" });
    const char* patterns_desc = nullptr;
    const bool multi_select_enabled = false;
    const char* ptr = tinyfd_openFileDialog(
            "Specify .tga File", default_path, patterns.size(), patterns.data(), patterns_desc, multi_select_enabled);

    return ptr ? std::make_optional(ptr) : std::nullopt;
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
    std::string path_to_tga;
    if (argc < 3) {
        const char* program_name = (argc == 1) ? argv[0] : "<program>";
        const char* arg1_str = "path-to-obj";
        const char* arg2_str = "path-to-tga";

        std::cout << "usage:" << " " << program_name << " <" << arg1_str << "> <" << arg2_str << ">\n";

        if (const auto opt_obj_path = find_obj(); !opt_obj_path.has_value()) {
            std::cerr << "tinyfiledialogs failed. exiting." << "\n";
            return EXIT_FAILURE;
        } else {
            path_to_obj = opt_obj_path.value();
            std::cout << "specified " << arg1_str << ": " << path_to_obj << "\n";
        }
        if (const auto opt_tga_path = find_tga(); !opt_tga_path.has_value()) {
            std::cerr << "tinyfiledialogs failed. exiting." << "\n";
            return EXIT_FAILURE;
        } else {
            path_to_tga = opt_tga_path.value();
            std::cout << "specified " << arg2_str << ": " << path_to_tga << "\n";
        }
    } else {
        path_to_obj = argv[1];
        path_to_tga = argv[2];
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
    MyUniform uniforms;
    uniforms.texture = asciirast::Texture(path_to_tga);

    const tinyobj::attrib_t& attrib = obj_reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = obj_reader.GetShapes();

    asciirast::VertexBuffer<MyVertex> vertex_buf{};
    vertex_buf.shape_type = asciirast::ShapeType::Triangles;

    if constexpr (true) {
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

                    const auto p0 = positions[static_cast<std::size_t>(idx0.vertex_index)];
                    const auto p1 = positions[static_cast<std::size_t>(idx1.vertex_index)];
                    const auto p2 = positions[static_cast<std::size_t>(idx2.vertex_index)];

                    const auto tc0 = texcoords[static_cast<std::size_t>(idx0.texcoord_index)];
                    const auto tc1 = texcoords[static_cast<std::size_t>(idx1.texcoord_index)];
                    const auto tc2 = texcoords[static_cast<std::size_t>(idx2.texcoord_index)];

                    vertex_buf.verticies.push_back(MyVertex{ { p0.xy, -p0.z }, tc0 });
                    vertex_buf.verticies.push_back(MyVertex{ { p1.xy, -p1.z }, tc1 });
                    vertex_buf.verticies.push_back(MyVertex{ { p2.xy, -p2.z }, tc2 });
                }

                index_offset += fv;
            }
        }
    } else {
        vertex_buf.verticies.push_back(MyVertex{ { -1, -1, 0 }, { 0, 0 } });
        vertex_buf.verticies.push_back(MyVertex{ { +1, -1, 0 }, { 1, 0 } });
        vertex_buf.verticies.push_back(MyVertex{ { -1, +1, 0 }, { 0, 1 } });

        vertex_buf.verticies.push_back(MyVertex{ { -1, +1, 0 }, { 0, 1 } });
        vertex_buf.verticies.push_back(MyVertex{ { +1, -1, 0 }, { 1, 0 } });
        vertex_buf.verticies.push_back(MyVertex{ { +1, +1, 0 }, { 1, 1 } });
    }
    SDLClock clock;
    SDLBuffer screen(512, 512);
    MyProgram program;
    asciirast::Renderer<{ .winding_order = asciirast::WindingOrder::CounterClockwise }> renderer;
    asciirast::RendererData<MyVarying> renderer_data;

    bool running = true;
    while (running) {
        handle_events(running);

        clock.update([&]([[maybe_unused]] float dt_sec) {
#ifdef NDEBUG
            uniforms.rot.rotateZX(+1 * dt_sec);
#endif
        });

        screen.clear();
        renderer.draw(program, uniforms, vertex_buf, screen, renderer_data);
        screen.render();

        clock.tick();
    }

    return EXIT_SUCCESS;
}
