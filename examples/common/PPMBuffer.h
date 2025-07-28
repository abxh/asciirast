#pragma once

#include "asciirast/framebuffer.h"
#include "asciirast/math/types.h"
#include "asciirast/renderer.h"

#include <cassert>
#include <fstream>

namespace math = asciirast::math;
using RGBFloat = math::Vec3;

enum class ImageType
{
    RED_CHANNEL,
    BLUE_CHANNEL,
    GREEN_CHANNEL,
    RGB,
    DEPTH_CHANNEL
};

struct RGB
{
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
};

class PPMBuffer
{
    static constexpr math::Float default_depth = 2; // or +infty

public:
    using Targets = RGBFloat;

    PPMBuffer(const std::size_t width, const std::size_t height)
            : m_width{ width }
            , m_height{ height }
    {
        m_screen_to_window = asciirast::SCREEN_BOUNDS //
                                     .to_transform()
                                     .reversed()
                                     .reflectY()
                                     .translate(0, 1.f)
                                     .scale(m_width - 1, m_height - 1);

        m_rgb_buf.resize(m_width * m_height);
        m_depth_buf.resize(m_width * m_height);

        this->clear();
    }

    void save_to(const std::string& fp, const ImageType type = ImageType::RGB)
    {
        // int casting is neccessary:
        // https://stackoverflow.com/questions/19562103/uint8-t-cant-be-printed-with-cout

        std::ofstream out;
        out.open(fp, std::ofstream::out | std::ofstream::trunc);

        out << "P3\n" << m_width << " " << m_height << "\n255\n"; // note: there are other formats than P3

        if (type == ImageType::RGB) {
            for (std::size_t y = 0; y < m_height; y++) {
                for (std::size_t x = 0; x < m_width; x++) {
                    const auto idx = index(y, x);
                    const auto [r, g, b] = m_rgb_buf[idx];
                    out << int{ r } << ' ' << int{ g } << ' ' << int{ b } << '\n';
                }
            }
        } else if (type == ImageType::DEPTH_CHANNEL) {
            for (std::size_t y = 0; y < m_height; y++) {
                for (std::size_t x = 0; x < m_width; x++) {
                    const auto idx = index(y, x);
                    if (m_depth_buf[idx] != default_depth) {
                        const auto val = static_cast<int>(255.f * (1 - m_depth_buf[idx]));
                        out << val << ' ' << val << ' ' << val << '\n';
                    } else {
                        const auto [r, g, b] = m_rgb_buf[idx];
                        out << int{ r } << ' ' << int{ g } << ' ' << int{ b } << '\n';
                    }
                }
            }
        } else {
            for (std::size_t y = 0; y < m_height; y++) {
                for (std::size_t x = 0; x < m_width; x++) {
                    const auto idx = index(y, x);
                    if (m_depth_buf[idx] != default_depth) {
                        const auto [r, g, b] = m_rgb_buf[idx];
                        out << int(type == ImageType::RED_CHANNEL ? r : 0) << ' ';
                        out << int(type == ImageType::GREEN_CHANNEL ? g : 0) << ' ';
                        out << int(type == ImageType::BLUE_CHANNEL ? b : 0) << '\n';
                    } else {
                        const auto [r, g, b] = m_rgb_buf[idx];
                        out << int{ r } << ' ' << int{ g } << ' ' << int{ b } << '\n';
                    }
                }
            }
        }

        out.close();
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
        const auto [r, g, b] = targets.array();

        m_rgb_buf[idx].r = static_cast<std::uint8_t>(255.f * r);
        m_rgb_buf[idx].g = static_cast<std::uint8_t>(255.f * g);
        m_rgb_buf[idx].b = static_cast<std::uint8_t>(255.f * b);
    }

    void clear()
    {
        for (std::size_t i = 0; i < m_height * m_width; i++) {
            m_rgb_buf[i] = { .r = 128, .g = 128, .b = 128 };
            m_depth_buf[i] = default_depth;
        }
    }

private:
    std::size_t index(const std::size_t y, const std::size_t x) const { return m_width * y + x; }

    std::size_t m_width;
    std::size_t m_height;
    math::Transform2D m_screen_to_window;

    std::vector<RGB> m_rgb_buf;
    std::vector<math::Float> m_depth_buf;
};

static_assert(asciirast::FrameBufferInterface<PPMBuffer>);
