#include "asciirast/texture.h"

#include <cassert>
#include <filesystem>
#include <iostream>

int
main(int argc, char* argv[])
{
    if (argc < 2) {
        const char* program_name = (argc == 1) ? argv[0] : "<program>";
        std::cout << "usage:" << " " << program_name << " " << "<path-to-image = png_wiki_example.png>\n";
        return EXIT_FAILURE;
    }
    const char* path_to_img = argc >= 2 ? argv[1] : "";

    std::filesystem::create_directory("images");
    const auto image_path = std::filesystem::path("images");

    auto texture = asciirast::Texture();
    texture.load(path_to_img);
    texture.generate_mipmaps();
    texture.save_as_png(image_path / (std::filesystem::path(path_to_img).stem().string() + ".png"));

    return 0;
}
