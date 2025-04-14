#include "asciirast/texture.h"

#include <cassert>
#include <filesystem>
#include <iostream>

int
main(int argc, char* argv[])
{
    if (argc < 2) {
        const char* program_name = (argc == 1) ? argv[0] : "<program>";
        std::cout << "usage:" << " " << program_name << " " << "<path-to-obj>";
        return EXIT_FAILURE;
    }
    const char* path_to_obj = argc >= 2 ? argv[1] : "";
    // const char* path_to_obj = "examples/data/png_wiki_example.png";

    std::filesystem::create_directory("images");
    const auto image_path = std::filesystem::path("images");

    auto texture = asciirast::Texture();
    texture.load(path_to_obj);
    texture.generate_mipmaps();
    texture.save_as_png(image_path / (std::filesystem::path(path_to_obj).stem().string() + ".png"));

    return 0;
}
