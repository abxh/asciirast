/**
 * @file texture.h
 * @brief Texture class
 *
 * Supports all textures stb_image supports. That means: jpeg / png / tga / bmp / psd / gif / hdr / pic / pnm
 * No animated gifs however.
 *
 * stb_image:
 * https://github.com/nothings/stb/blob/master/stb_image.h
 */

// on alpha-blending:
// https://github.com/nothings/stb/blob/master/stb_image_resize2.h

// on mipmap generation:
// https://github.com/nikolausrauch/software-rasterizer/blob/master/rasterizer/texture.h
// https://web.archive.org/web/20250324123030/https://vulkan-tutorial.com/Generating_Mipmaps

#pragma once

#include "./math/types.h"

#include <algorithm>
#include <assert.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <vector>

namespace asciirast {

namespace detail {

#include "external/stb_image/stb_image.h"
#include "external/stb_image/stb_image_write.h"

#include <cassert>
#include <cerrno>

[[maybe_unused]]
static auto
texture_index(math::Int width, math::Int height, math::Int x, math::Int y) -> std::size_t
{
    assert(0 <= y && "index is non-negative");
    assert(0 <= x && "index is non-negative");
    assert(width * y + x < width * height && "index is in bounds");
    (void)(height);

    return static_cast<std::size_t>(width * y + x);
}

}

/**
 * @brief Texture storage class
 *
 * @tparam T The pixel type
 */
template<typename T, typename Allocator = std::allocator<T>>
class TextureStorage
{
    math::Int m_width;
    math::Int m_height;
    std::vector<T, Allocator> m_pixels;

public:
    /**
     * @brief Construct a default texture
     *
     * @param width The width of the texture
     * @param height The height of the texture
     * @param default_color The default color to use
     */
    TextureStorage(const math::Int width = 1, const math::Int height = 1, const T default_color = T()) noexcept
            : m_width{ std::max(1, width) }
            , m_height{ std::max(1, height) }
    {
        m_pixels.resize(static_cast<std::size_t>(m_width * m_height));

        this->fill(default_color);
    }

    /**
     * @brief Get the width of the texture
     *
     * @return The width as math::Int
     */
    math::Int width() const { return m_width; }

    /**
     * @brief Get the height of the texture
     *
     * @return The height as math::Int
     */
    math::Int height() const { return m_height; }

    /**
     * @brief Get the pointer to the underlying data
     *
     * @return The pointer to the first pixel
     */
    T* data() { return m_pixels.data(); }

    /**
     * @brief Get the pointer to the underlying data
     *
     * @return The const pointer to the first pixel
     */
    const T* data() const { return m_pixels.data(); }

    /**
     * @brief Fill the texture with a value
     *
     * @param value The value at hand
     */
    void fill(const T& value) { std::ranges::fill(m_pixels, value); }

    /**
     * @brief Index the texture
     *
     * @param y Which scan line?
     * @param x Which pixel on the scan line?
     * @return The reference to the value at the particular index
     */
    T& operator[](const math::Int y, const math::Int x)
    {
        return m_pixels[detail::texture_index(m_width, m_height, x, y)];
    }

    /**
     * @brief Index the texture
     *
     * @param y Which scan line?
     * @param x Which pixel on the scan line?
     * @return The value at the particular index
     */
    const T& operator[](const math::Int y, const math::Int x) const
    {
        return m_pixels[detail::texture_index(m_width, m_height, x, y)];
    }
};

/**
 * @brief Texutre class
 */
template<typename RGBA_8bit_Allocator = std::allocator<math::RGBA_8bit>,
         typename MipmapAllocator = std::allocator<TextureStorage<math::RGBA_8bit, RGBA_8bit_Allocator>>>
class Texture
{
protected:
    using RGBA_8bit_TextureStorage = TextureStorage<math::RGBA_8bit, RGBA_8bit_Allocator>; ///< mipmap alias
    using Mipmaps = std::vector<RGBA_8bit_TextureStorage, MipmapAllocator>;                ///< mipmaps

    Mipmaps m_mipmaps;         ///< mipmaps
    bool m_has_loaded = false; ///< whether a texture has been loaded

public:
    /**
     * @brief Construct a unintialized texture
     */
    Texture() = default;

    /**
     * @brief Construct a texture from a texture file path
     *
     * @exception runtime_error For various errors about the texture
     *
     * @param file_path Path to the texture file
     */
    Texture(const std::filesystem::path& file_path) { this->load(file_path); }

    /**
     * @brief Check if texture has been loaded
     *
     * @return Returns whether the texture has been loaded
     */
    bool has_loaded() const { return m_has_loaded; }

    /**
     * @brief Get all mipmaps
     *
     * @return A const reference to the mipmaps container
     */
    const Mipmaps& mipmaps() const { return m_mipmaps; }

    /**
     * @brief Get the (first) texture storage
     *
     * @return Reference to the first texture storage
     */
    TextureStorage<math::RGBA_8bit, RGBA_8bit_Allocator>& get()
    {
        assert(this->has_loaded());

        return m_mipmaps.front();
    }

    /**
     * @brief Get the (first) texture storage
     *
     * @return Const reference to the first texture storage
     */
    const TextureStorage<math::RGBA_8bit, RGBA_8bit_Allocator>& get() const
    {
        assert(this->has_loaded());

        return m_mipmaps.front();
    }

    /**
     * @brief Get the width of the texture
     *
     * @return The width as math::Int
     */
    math::Int width() const { return get().width(); }

    /**
     * @brief Get the height of the texture
     *
     * @return The height as math::Int
     */
    math::Int height() const { return get().height(); }

    /**
     * @brief Get the pointer to the underlying data
     *
     * @return The pointer to the first pixel
     */
    math::RGBA_8bit* data() { return get().data(); }

    /**
     * @brief Get the pointer to the underlying data
     *
     * @return The const pointer to thfuck e first pixel
     */
    const math::RGBA_8bit* data() const { return get().data(); }

    /**fuck 
     * @brief Fill the texture with a value
     *
     * @param value The value at hand
     */
    void fill(const math::RGBA_8bit& value) { return get().fill(value); }

    /**
     * @brief Index the texture
     *
     * @param y Which scan line?
     * @param x Which pixel on the scan line?
     * @return The reference to the value at the particular index
     */
    math::RGBA_8bit& operator[](const math::Int y, const math::Int x) { return get()[y, x]; }

    /**
     * @brief Index the texture
     *
     * @param y Which scan line?
     * @param x Which pixel on the scan line?
     * @return The value at the particular index
     */
    const math::RGBA_8bit& operator[](const math::Int y, const math::Int x) const { return get()[y, x]; }

    /**
     * @brief Load a texture, given a texture file path
     *
     * @exception runtime_error For various errors about the texture
     *
     * @param file_path Path to the texture file
     */
    void load(const std::filesystem::path& file_path)
    {
        static_assert(4 * sizeof(unsigned char) == sizeof(math::RGBA_8bit));

        int width, height, n;
        detail::stbi_set_flip_vertically_on_load(true);
        unsigned char* ptr = detail::stbi_load(file_path.c_str(), &width, &height, &n, detail::STBI_rgb_alpha);
        if (!ptr) {
            throw std::runtime_error("asciirast::Texture::load() : " + std::string(detail::stbi_failure_reason()));
        }

        const auto mip_levels =
                std::max<unsigned>(1U, static_cast<unsigned>(std::floor(std::log2(std::max(width, height)))));
        const auto ptr_rgba = reinterpret_cast<const math::RGBA_8bit*>(ptr);

        m_mipmaps.resize(mip_levels);
        m_mipmaps[0] = RGBA_8bit_TextureStorage(width, height);

        for (math::Int y = 0; y < height; y++) {
            for (math::Int x = 0; x < width; x++) {
                const std::size_t idx = detail::texture_index(width, height, x, y);

                m_mipmaps[0].data()[idx] = ptr_rgba[idx];
            }
        }

        free(ptr);
        m_has_loaded = true;
    }

    /**
     * @brief Save texture as png to file path
     *
     * @exception runtime_error When the texture cannot be saved
     *
     * @param file_path The path to save to
     * @param overwrite Overwrite if file exists
     * @param min_mipmap_level The minimum mipmap level of files
     * @param max_mipmap_level The maxmimum mipmap level of files
     */
    void save_as_png(const std::filesystem::path& file_path,
                     bool overwrite = true,
                     const std::size_t min_mipmap_level = 0,
                     const std::size_t max_mipmap_level = std::numeric_limits<std::size_t>::max())
    {
        assert(this->has_loaded());

        if (file_path.extension().string() != ".png") {
            throw std::runtime_error("asciirast::Texture::save() : " + file_path.string() + " is not a .png file");
        }
        for (std::size_t i = min_mipmap_level; i < std::min(m_mipmaps.size(), max_mipmap_level); i++) {
            const auto width = static_cast<int>(m_mipmaps[i].width());
            const auto height = static_cast<int>(m_mipmaps[i].height());
            const auto stride = 4 * width;

            const auto width_str = std::to_string(width);
            const auto height_str = std::to_string(height);

            detail::stbi_flip_vertically_on_write(true);

            const auto dir = file_path.parent_path();
            const auto path_str = ((dir / file_path.stem()).string() + "_" + width_str + "x" + height_str + ".png");

            if (overwrite && std::filesystem::exists(path_str) && std::filesystem::is_regular_file(path_str)) {
                std::filesystem::remove(path_str);
            }

            errno = 0;
            const int ret_val = detail::stbi_write_png(path_str.c_str(), width, height, 4, m_mipmaps[i].data(), stride);
            if (ret_val == 0) {
                if (errno != 0) {
                    throw std::runtime_error("asciirast::Texture::save() : " + std::string(std::strerror(errno)));
                } else {
                    throw std::runtime_error("asciirast::Texture::save() : stb_write_png() failed for some reason!");
                }
            }
        }
    }

    /**
     * @brief Generate mipmaps from the loaded texture
     */
    void generate_mipmaps()
    {
        const auto extract_2x2_pixels = [](const RGBA_8bit_TextureStorage& mipmap,
                                           const math::Int x,
                                           const math::Int y) -> std::array<math::Vec4Int, 4> {
            return { math::Vec4Int{ mipmap[y + 0, x + 0] }, //
                     math::Vec4Int{ mipmap[y + 0, x + 1] },
                     math::Vec4Int{ mipmap[y + 1, x + 0] },
                     math::Vec4Int{ mipmap[y + 1, x + 1] } };
        };

        assert(this->has_loaded());

        math::Int mip_width = m_mipmaps[0].width();
        math::Int mip_height = m_mipmaps[0].height();

        for (std::size_t i = 1; i < m_mipmaps.size(); i++) {
            mip_width = std::max(1, mip_width / 2);
            mip_height = std::max(1, mip_height / 2);

            m_mipmaps[i] = RGBA_8bit_TextureStorage(mip_width, mip_height);

            for (math::Int y = 0; y < m_mipmaps[i].height(); y++) {
                for (math::Int x = 0; x < m_mipmaps[i].width(); x++) {
                    const auto c_arr = extract_2x2_pixels(m_mipmaps[i - 1], 2 * x, 2 * y);
                    const auto a_sum = c_arr[0].a + c_arr[1].a + c_arr[2].a + c_arr[3].a;

                    if (a_sum == 0) {
                        m_mipmaps[i][y, x] = math::RGBA_8bit{ 0 };
                    } else {
                        math::Vec4Int res{};
                        for (std::size_t j = 0; j < 4; j++) {
                            res.rgb += c_arr[j].rgb * c_arr[j].a;
                        }
                        res.rgb /= a_sum;
                        res.a = a_sum / 4;
                        m_mipmaps[i][y, x] = math::RGBA_8bit{ res };
                    }
                }
            }
        }
    }
};

} // namespace asciirast
