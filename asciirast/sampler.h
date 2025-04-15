/**
 * @file sampler.h
 * @brief Texture sampler classes
 *
 * @todo bicubic interpolation
 * @todo use mipmaps
 */

// intro on texture samplers:
// https://learnopengl.com/Getting-started/Textures

// Border wrapping method:
// https://www.youtube.com/watch?v=4s30cnqxJ-0 (javidx9)

// Sample / Filter method:
// https://www.youtube.com/watch?v=_htjjOdXbmA (javidx9)

// Reference:
// https://github.com/nikolausrauch/software-rasterizer/blob/master/rasterizer/sampler.h

// Calculating LOD:
// https://graemephi.github.io/posts/calculating-lod/

#pragma once

#include "./math/types.h"
#include "texture.h"

#include <cassert>

namespace asciirast {

/**
 * @brief Pixel sampling method
 */
enum class SampleMethod
{
    Point,
    Nearest,
    Linear,
};

/**
 * @brief Border wrapping method
 */
enum class WrapMethod
{
    Blank,
    Clamp,
    Periodic,
    Repeat,
};

/**
 * @brief Texture sampler
 */
template<typename RGBA_8bit_Allocator = std::allocator<math::RGBA_8bit>>
class Sampler
{
protected:
    const Texture<RGBA_8bit_Allocator>& m_texture; ///< texture data

public:
    static constexpr auto blank_color = math::Vec4{ 1, 0, 1, 1 }; ///< default blank color

    SampleMethod sample_method = SampleMethod::Linear; ///< pixel sampling method
    WrapMethod wrap_method = WrapMethod::Clamp;        ///< border wrapping method
    // SampleMethod mipmap_sample_method = SampleMethod::Nearest; ///< mipmap sampling method

    /**
     * @brief Initialize this sampler to sample from a texture
     *
     * @param texture Const reference to the texture at hand
     */
    Sampler(const Texture<RGBA_8bit_Allocator>& texture) noexcept
            : m_texture{ texture } {};

    /**
     * @brief Sample the texture at the given uv coordinate
     *
     * @param uv The uv coordinate
     * @param i The index of the mipmap to use
     * @return The RGBA pixel color as Vec
     */
    math::Vec4 sample(const math::Vec2& uv, const std::size_t i = 0) const
    {
        assert(i == 0); // TODO: 2x2 block processing for calculating screen gradients?...
        assert(i < m_texture.mipmaps().size());

        const auto size_x = m_texture.mipmaps()[i].width();
        const auto size_y = m_texture.mipmaps()[i].height();

        const auto UV = math::Vec2{ size_x - 1, size_y - 1 } * uv;

        switch (sample_method) {
        case SampleMethod::Point: {
            return color_at(math::Vec2Int{ UV }, i);
        } break;
        case SampleMethod::Nearest: {
            return color_at(math::Vec2Int{ math::round(UV - math::Vec2{ 0.5f, 0.5f }) }, i);
        } break;
        case SampleMethod::Linear: {
            const auto UVs = UV - math::Vec2{ 0.5f, 0.5f };

            const math::Vec2 UVs_whole = math::floor(UVs);
            const math::Vec2 UVs_decimal = UVs - UVs_whole;

            const auto c00 = color_at(math::Vec2Int{ UVs_whole } + math::Vec2Int{ 0, 0 }, i);
            const auto c01 = color_at(math::Vec2Int{ UVs_whole } + math::Vec2Int{ 0, 1 }, i);
            const auto c10 = color_at(math::Vec2Int{ UVs_whole } + math::Vec2Int{ 1, 0 }, i);
            const auto c11 = color_at(math::Vec2Int{ UVs_whole } + math::Vec2Int{ 1, 1 }, i);

            const auto c0t = math::lerp(c00, c01, math::Float{ UVs_decimal.y });
            const auto c1t = math::lerp(c10, c11, math::Float{ UVs_decimal.y });
            const auto cst = math::lerp(c0t, c1t, math::Float{ UVs_decimal.x });

            return cst;
        } break;
        }
        return blank_color;
    }

    template<typename Allocator>
    friend math::Vec4 textureLOD(const Sampler<Allocator>&, const math::Vec2&, const math::Float);

protected:
    /**
     * @brief Get the color at the denormalized uv coordinate
     *
     * @param pos The denormalized uv coordinate
     * @param i The index of the mipmap to use
     * @return The color as Vec4
     */
    math::Vec4 color_at(math::Vec2Int pos, const std::size_t i) const
    {
        assert(i < m_texture.mipmaps().size());

        const auto size_x = m_texture.mipmaps()[i].width();
        const auto size_y = m_texture.mipmaps()[i].height();

        const auto UV_max = math::Vec2Int{ size_x - 1, size_y - 1 };

        const auto remainder = [](const math::Int x, const math::Int y, const math::Int default_val) {
            if (x != std::numeric_limits<math::Int>::min()) [[likely]] {
                return std::abs(x) % y;
            } else {
                return default_val;
            }
        };

        switch (wrap_method) {
        case WrapMethod::Blank: {
            if (!(0 <= pos.x && pos.x < size_x) || //
                !(0 <= pos.y && pos.y < size_y)) {
                return blank_color;
            }
        } break;
        case WrapMethod::Clamp: {
            pos = math::clamp(pos, math::Vec2Int{ 0, 0 }, UV_max);
        } break;
        case WrapMethod::Periodic: {
            pos.x = remainder(pos.x, size_x, UV_max.x);
            pos.y = remainder(pos.y, size_y, UV_max.y);
        } break;
        case WrapMethod::Repeat: {
            pos.x = (pos.x >= 0) ? (pos.x % size_x) : remainder(UV_max.x + pos.x, size_x, UV_max.x);
            pos.y = (pos.y >= 0) ? (pos.y % size_y) : remainder(UV_max.y + pos.y, size_y, UV_max.y);
        } break;
        }
        return math::Vec4{ m_texture.mipmaps()[i][pos.y, pos.x] } / 255.f;
    }
};

/**
 * @brief Sample texture at a uv coordinate
 *
 * Naming scheme after OpenGL
 *
 * @param sampler The sampler
 * @param uv The uv coordinate
 * @return Color at the uv coordinate
 */
template<typename Allocator>
[[maybe_unused]]
static math::Vec4
texture(const Sampler<Allocator>& sampler, const math::Vec2& uv)
{
    return sampler.sample(uv);
}

/**
 * @brief Sample texture at a uv coordinate with a specific Level-Of-Detail
 *
 * @param sampler The sampler
 * @param uv The uv coordinate
 * @param lod The level of details a float to be used as a index
 * @return Color at the uv coordinate
 */
/*
template<typename Allocator>
[[maybe_unused]]
static math::Vec4
textureLOD(const Sampler<Allocator>& sampler, const math::Vec2& uv, const math::Float lod)
{
    const math::Float LOD = std::clamp(lod, 0.f, static_cast<math::Float>(sampler.m_texture.mipmaps().size() - 1));

    switch (sampler.mipmap_sample_method) {
    case SampleMethod::Point: {
        return sampler.sample(uv, static_cast<std::size_t>(LOD));
    } break;
    case SampleMethod::Nearest: {
        return sampler.sample(uv, static_cast<std::size_t>(std::round(LOD)));
    } break;
    case SampleMethod::Linear: {
        const math::Float LOD_floor = std::floor(LOD);
        const math::Float LOD_ceil = std::ceil(LOD);
        const math::Float t = LOD_ceil - LOD;

        const math::Vec4 sample_floor = sampler.sample(uv, static_cast<std::size_t>(LOD_floor));
        const math::Vec4 sample_ceil = sampler.sample(uv, static_cast<std::size_t>(LOD_ceil));

        return math::lerp(sample_floor, sample_ceil, t);
    } break;
    }
    return Sampler<Allocator>::blank_color;
}
*/

}
