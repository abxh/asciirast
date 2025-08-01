
#pragma once

#include "asciirast/framebuffer.h"
#include "asciirast/math/types.h"
#include "asciirast/renderer.h"

#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <vector>

// sdl code based on:
// https://stackoverflow.com/questions/33304351/sdl2-fast-pixel-manipulation

namespace math = asciirast::math;

using RGB = math::Vec3;
using RGBA = math::Vec4;

struct RGBA_uint8
{
    // this order should match SDL pixel format
    std::uint8_t b;
    std::uint8_t g;
    std::uint8_t r;
    std::uint8_t a;
};

class SDLBuffer
{
public:
    using Targets = RGBA;

    SDLBuffer(const unsigned width, const unsigned height)
    {
        m_width = width;
        m_height = height;

        m_screen_to_window = asciirast::SCREEN_BOUNDS //
                                     .to_transform()
                                     .inversed()
                                     .reflectY()
                                     .translate(0, 1.f)
                                     .scale(m_width - 1, m_height - 1);

        m_rgba_buf.resize(m_width * m_height);
        m_depth_buf.resize(m_width * m_height);

        SDL_Init(SDL_INIT_EVERYTHING);
        m_window = SDL_CreateWindow(
                "SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)width, (int)height, SDL_WINDOW_SHOWN);
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

        const std::uint32_t pixel_format = SDL_PIXELFORMAT_ARGB8888; // use SDL_GetRendererInfo to get more info
        m_texture = SDL_CreateTexture(m_renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, m_width, m_height);
        SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
    }
    ~SDLBuffer()
    {
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    bool test_and_set_depth(const math::Vec2Int& pos, math::Float depth)
    {
        assert(0 <= pos.x && (std::size_t)(pos.x) < m_width);
        assert(0 <= pos.y && (std::size_t)(pos.y) < m_height);

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        depth = std::clamp<math::Float>(depth, 0, 1);

        if (depth < m_depth_buf[idx]) {
            m_depth_buf[idx] = depth;
            return true;
        }
        return false;
    }

    const math::Transform2D& screen_to_window() { return m_screen_to_window; }

    void plot(const math::Vec2Int& pos, const Targets& targets)
    {
        assert(0 <= pos.x && (std::size_t)pos.x < m_width);
        assert(0 <= pos.y && (std::size_t)pos.y < m_height);

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        const auto [r, g, b, a] = targets.array();

        m_rgba_buf[idx].r = static_cast<std::uint8_t>(255.f * r);
        m_rgba_buf[idx].g = static_cast<std::uint8_t>(255.f * g);
        m_rgba_buf[idx].b = static_cast<std::uint8_t>(255.f * b);
        m_rgba_buf[idx].a = static_cast<std::uint8_t>(255.f * a);
    }

    void render()
    {
        update();
        show();
    }

    void update()
    {
        SDL_UpdateTexture(m_texture, nullptr, m_rgba_buf.data(), m_width * 4);
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
    }

    void show() { SDL_RenderPresent(m_renderer); }

    void clear()
    {
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(m_renderer);

        for (std::size_t i = 0; i < m_height * m_width; i++) {
            m_rgba_buf[i] = { .b = 0, .g = 0, .r = 0, .a = 0 };
            m_depth_buf[i] = 2; // or +infty
        }
    }

    friend class SDLFont;
    friend class SDLStaticText;

private:
    std::size_t index(const std::size_t y, const std::size_t x) const { return m_width * y + x; }

    std::size_t m_width;
    std::size_t m_height;
    math::Transform2D m_screen_to_window;

    std::vector<RGBA_uint8> m_rgba_buf;
    std::vector<math::Float> m_depth_buf;

    SDL_Texture* m_texture = nullptr;
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};

static_assert(asciirast::FrameBufferInterface<SDLBuffer>);

class SDLFont
{
public:
    SDLFont(const char* path, const int font_size = 36)
    {
        int ret = TTF_Init();
        if (ret == -1) {
            throw std::runtime_error(std::string("TTF_Init failed: ") + TTF_GetError());
        }
        m_font = TTF_OpenFont(path, font_size);
        if (!m_font) {
            throw std::runtime_error(std::string("TTF_OpenFont failed: ") + TTF_GetError());
        }
    }
    ~SDLFont()
    {
        TTF_CloseFont(m_font);
        TTF_Quit();
    }

    friend class SDLStaticText;

private:
    TTF_Font* m_font = nullptr;
};

class SDLStaticText
{
public:
    SDLStaticText(SDLBuffer& screen, const SDLFont& font, const char* text, const SDL_Color color = { 255, 0, 0, 255 })
            : m_screen{ screen }
    {
        m_surface = TTF_RenderText_Solid_Wrapped(font.m_font, text, color, (std::uint32_t)m_screen.m_width);
        if (!m_surface) {
            throw std::runtime_error(std::string("TTF_RenderText_Solid_Wrapped failed: ") + TTF_GetError());
        }
        m_texture = SDL_CreateTextureFromSurface(m_screen.m_renderer, m_surface);
        if (!m_texture) {
            throw std::runtime_error(std::string("SDL_CreateTextureFromSurface failed: ") + TTF_GetError());
        }
    }

    ~SDLStaticText()
    {
        SDL_DestroyTexture(m_texture);
        SDL_FreeSurface(m_surface);
    }

    void render(const math::Vec2Int pos = { 10, 10 }) const
    {
        const SDL_Rect rect = { .x = pos.x, .y = pos.y, .w = m_surface->w, .h = m_surface->h };
        SDL_RenderCopy(m_screen.m_renderer, m_texture, nullptr, &rect);
    }

private:
    SDLBuffer& m_screen;
    SDL_Surface* m_surface = nullptr;
    SDL_Texture* m_texture = nullptr;
};
