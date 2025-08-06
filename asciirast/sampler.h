/**
 * @file sampler.h
 * @brief Texture sampler classes
 *
 * @todo bicubic interpolation
 * @todo use mipmaps
 *
 * intro on texture samplers:
 * https://learnopengl.com/Getting-started/Textures
 *
 * reference sampler:
 * https://github.com/nikolausrauch/software-rasterizer/blob/master/rasterizer/sampler.h
 *
 * border wrapping, and sample method:
 * https://www.youtube.com/watch?v=4s30cnqxJ-0 (javidx9)
 *
 * on the use of the dFdx / dFdy functions:
 * https://www.youtube.com/watch?v=J1n1yPjac1c (Ned Makes Games)
 */

#pragma once

#include "./math/types.h"
#include "fragment.h"
#include "texture.h"

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
class Sampler
{
public:
    static constexpr auto blank_color = math::Vec4{ 1, 0, 1, 1 }; ///< default blank color

    WrapMethod wrap_method = WrapMethod::Clamp;                ///< border wrapping method
    SampleMethod sample_method = SampleMethod::Nearest;        ///< pixel sampling method
    SampleMethod mipmap_sample_method = SampleMethod::Nearest; ///< mipmap sampling method

    /**
     * @brief Prepare to sample texture at a uv coordinate
     * @return Return sampled color at the uv coordinate
     */
    template<typename RGBA_8bit_Allocator, typename MipmapAllocator>
    friend math::Vec4 textureLOD(const Sampler&,
                                 const TextureType<RGBA_8bit_Allocator, MipmapAllocator>&,
                                 const math::Vec2&,
                                 const math::Float);

    /**
     * @brief Sample texture at a uv coordinate
     * @return Return a special fragment token to be co-yielded back to the renderer
     */
    template<typename RGBA_8bit_Allocator, typename MipmapAllocator, typename... ValueTypes>
    friend auto texture_init(const FragmentContextType<ValueTypes...>&,
                             const Sampler&,
                             const TextureType<RGBA_8bit_Allocator, MipmapAllocator>&,
                             const math::Vec2&);

    /**
     * @brief Sample texture at a uv coordinate
     * @return Return sampled color at the uv coordinate
     */
    template<typename RGBA_8bit_Allocator, typename MipmapAllocator, typename... ValueTypes>
    friend auto texture(const FragmentContextType<ValueTypes...>&,
                        const Sampler&,
                        const TextureType<RGBA_8bit_Allocator, MipmapAllocator>&,
                        const math::Vec2&);

protected:
    /**
     * @brief Sample the texture at the given uv coordinate
     *
     * @param i Mipmap level
     * @param uv The uv coordinate
     * @return The RGBA pixel color as Vec
     */
    template<typename RGBA_8bit_Allocator, typename MipmapAllocator>
    [[nodiscard]] math::Vec4 sample(const TextureType<RGBA_8bit_Allocator, MipmapAllocator>& t,
                                    const math::Vec2& uv,
                                    const std::size_t i) const
    {
        ASCIIRAST_ASSERT(i < t.mipmaps().size(), "index is inside bounds");

        const auto size_x = t.mipmaps()[i].width();
        const auto size_y = t.mipmaps()[i].height();

        const auto UV = math::Vec2{ size_x - 1, size_y - 1 } * uv;

        switch (sample_method) {
        case SampleMethod::Point: {
            return color_at(math::Vec2Int{ UV }, t, i);
        } break;
        case SampleMethod::Nearest: {
            return color_at(math::Vec2Int{ round(UV - math::Vec2{ 0.5f, 0.5f }) }, t, i);
        } break;
        case SampleMethod::Linear: {
            const auto UVs = UV - math::Vec2{ 0.5f, 0.5f };

            const math::Vec2 UVs_whole = floor(UVs);
            const math::Vec2 UVs_decimal = UVs - UVs_whole;

            const auto c00 = color_at(math::Vec2Int{ UVs_whole } + math::Vec2Int{ 0, 0 }, t, i);
            const auto c01 = color_at(math::Vec2Int{ UVs_whole } + math::Vec2Int{ 0, 1 }, t, i);
            const auto c10 = color_at(math::Vec2Int{ UVs_whole } + math::Vec2Int{ 1, 0 }, t, i);
            const auto c11 = color_at(math::Vec2Int{ UVs_whole } + math::Vec2Int{ 1, 1 }, t, i);

            const auto c0t = lerp(c00, c01, math::Float{ UVs_decimal.y });
            const auto c1t = lerp(c10, c11, math::Float{ UVs_decimal.y });
            const auto cst = lerp(c0t, c1t, math::Float{ UVs_decimal.x });

            return cst;
        } break;
        }
        return blank_color;
    }

    /**
     * @brief Get the color at the denormalized uv coordinate
     *
     * @param pos The denormalized uv coordinate
     * @param i The index of the mipmap to use
     * @return The color as Vec4
     */
    template<typename RGBA_8bit_Allocator, typename MipmapAllocator>
    math::Vec4 color_at(math::Vec2Int pos,
                        const TextureType<RGBA_8bit_Allocator, MipmapAllocator>& t,
                        const std::size_t i) const
    {
        ASCIIRAST_ASSERT(i < t.mipmaps().size(), "debug is inside bounds");

        const auto size_x = t.mipmaps()[i].width();
        const auto size_y = t.mipmaps()[i].height();

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
            pos = clamp(pos, math::Vec2Int{ 0, 0 }, UV_max);
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
        return math::Vec4{ t.mipmaps()[i][pos.y, pos.x] } / 255.f;
    }
};

/**
 * @brief Sample texture at a uv coordinate with a specific Level-Of-Detail
 *
 * @param sampler The sampler
 * @param texture The texture
 * @param uv The uv coordinate
 * @param lod The level of detail as to be used as a index with 0 being the highest and +inf being the lowest.
 * @return Color at the uv coordinate
 */
template<typename RGBA_8bit_Allocator, typename MipmapAllocator>
[[maybe_unused]]
math::Vec4
textureLOD(const Sampler& sampler,
           const TextureType<RGBA_8bit_Allocator, MipmapAllocator>& texture,
           const math::Vec2& uv,
           const math::Float lod)
{
    const math::Float LOD = std::clamp(lod, 0.f, static_cast<math::Float>(texture.mipmaps().size() - 1));

    switch (sampler.mipmap_sample_method) {
    case SampleMethod::Point: {
        return sampler.sample(texture, uv, static_cast<std::size_t>(LOD));
    } break;
    case SampleMethod::Nearest: {
        return sampler.sample(texture, uv, static_cast<std::size_t>(std::round(LOD)));
    } break;
    case SampleMethod::Linear: {
        const math::Float LOD_floor = std::floor(LOD);
        const math::Float LOD_ceil = std::ceil(LOD);
        const math::Float t = LOD_ceil - LOD;

        const math::Vec4 sample_floor = sampler.sample(texture, uv, static_cast<std::size_t>(LOD_floor));
        const math::Vec4 sample_ceil = sampler.sample(texture, uv, static_cast<std::size_t>(LOD_ceil));

        return lerp(sample_floor, sample_ceil, t);
    } break;
    }
    return Sampler::blank_color;
}

/**
 * @brief Prepare to sample texture at a uv coordinate
 *
 * @param context Fragment context
 * @param texture The texture
 * @return Return a special fragment token to be co-yielded back to the renderer
 */
template<typename RGBA_8bit_Allocator, typename MipmapAllocator, typename... ValueTypes>
    requires((std::is_same_v<ValueTypes, math::Vec2> || ...))
[[maybe_unused]]
ProgramToken
texture_init(FragmentContextType<ValueTypes...>& context,
             const TextureType<RGBA_8bit_Allocator, MipmapAllocator>& texture,
             const math::Vec2& uv)
{
    ASCIIRAST_ASSERT(texture.mipmaps_generated(), "mipmaps generated before texture_init is called");

    const auto texture_size = math::Vec2{ texture.width(), texture.height() };

    return context.init(texture_size * uv);
}

/**
 * @brief Sample texture at a uv coordinate
 *
 * @note must co_yield the result of texture_init() before calling texture()
 *
 * @param context Fragment context
 * @param sampler The sampler
 * @param texture The texture
 * @return Return sampled color at the uv coordinate
 */
template<typename RGBA_8bit_Allocator, typename MipmapAllocator, typename... ValueTypes>
    requires((std::is_same_v<ValueTypes, math::Vec2> || ...))
[[maybe_unused]]
math::Vec4
texture(FragmentContextType<ValueTypes...>& context,
        const Sampler& sampler,
        const TextureType<RGBA_8bit_Allocator, MipmapAllocator>& texture,
        const math::Vec2& uv)
{
    using Type = FragmentContextType<ValueTypes...>::Type;

    switch (context.type()) {
    case Type::POINT: {
        return textureLOD(sampler, texture, uv, 0.f);
    } break;
    case Type::LINE: {
        const math::Vec2 dFdv = context.dFdv(std::type_identity<math::Vec2>());
        const math::Float d = dot(dFdv, dFdv);
        const math::Float lod = 0.5f * std::log2(std::max<math::Float>(1, d));

        return textureLOD(sampler, texture, uv, lod);
    } break;
    case Type::FILLED: {
        const math::Vec2 dFdx = context.dFdx(std::type_identity<math::Vec2>());
        const math::Vec2 dFdy = context.dFdy(std::type_identity<math::Vec2>());
        const math::Float d = std::max(dot(dFdx, dFdx), dot(dFdy, dFdy));
        const math::Float lod = 0.5f * std::log2(std::max<math::Float>(1, d));

        return textureLOD(sampler, texture, uv, lod);
    } break;
    }
    return Sampler::blank_color;
}

#ifdef __GNUC__
/**
 * @def TEXTURE(context, sampler, texture_, uv)
 * @brief Initialize and execute the texture call in one go in a GNU C expression statement
 */
#define TEXTURE(context, sampler, texture_, uv)                                                                        \
    __extension__({                                                                                                    \
        co_yield asciirast::texture_init(context, texture_, uv);                                                       \
        asciirast::texture(context, sampler, texture_, uv);                                                            \
    })
#endif

};
