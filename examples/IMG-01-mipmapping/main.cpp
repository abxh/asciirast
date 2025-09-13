#include <asciirast.hpp>

#include <tinyfiledialogs.h>

#include <filesystem>
#include <iostream>

std::optional<std::string>
find_img()
{
    const auto default_path = ".";
    const auto patterns = std::to_array<char const*>(
        { "*.jpg", "*.jpeg", "*.png", "*.tga", "*.bmp", "*.psd", "*.gif", "*.hdr", "*.pic", "*.pnm" });
    const char* patterns_desc = nullptr;
    const bool multi_select_enabled = false;
    const char* ptr = tinyfd_openFileDialog(
        "Specify Image File", default_path, patterns.size(), patterns.data(), patterns_desc, multi_select_enabled);

    return ptr ? std::make_optional(ptr) : std::nullopt;
}

int
main(int argc, char* argv[])
{
    std::string path_to_img;
    if (argc < 2) {
        const char* program_name = (argc == 1) ? argv[0] : "<program>";
        const char* arg1_str = "path-to-image";

        std::cout << "usage:"
                  << " " << program_name << " "
                  << "<" << arg1_str << ">\n";
        std::cout << "specified " << arg1_str << ": " << std::flush;

        if (const auto opt_path = find_img(); !opt_path.has_value()) {
            std::cerr << "tinyfiledialogs failed. exiting."
                      << "\n";
            return EXIT_FAILURE;
        } else {
            path_to_img = opt_path.value();
            std::cout << path_to_img << "\n";
        }
    } else {
        path_to_img = argv[1];
    }

    std::filesystem::create_directory("images");
    const auto image_path = std::filesystem::path("images");

    auto texture = asciirast::Texture();
    texture.load(path_to_img);
    texture.generate_mipmaps();
    texture.save_as_png(image_path / (std::filesystem::path(path_to_img).stem().string() + ".png"));

    return 0;
}
