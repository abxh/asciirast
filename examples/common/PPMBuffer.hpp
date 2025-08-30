#pragma once

#include <asciirast.hpp>

#include <cassert>
#include <fstream>
#include <vector>

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
public:
    using Targets = RGBFloat;
    static constexpr math::Float DEFAULT_DEPTH = -1; // or -infty

    PPMBuffer(const std::size_t width, const std::size_t height)
            : m_width{ width }
            , m_height{ height }
    {
        m_screen_to_window = asciirast::SCREEN_BOUNDS //
                                     .to_transform()
                                     .inversed()
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
                    if (m_depth_buf[idx] != DEFAULT_DEPTH) {
                        const auto val = static_cast<int>(255.f * m_depth_buf[idx]);
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
                    if (m_depth_buf[idx] != DEFAULT_DEPTH) {
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

    bool test_and_set_depth(const math::Vec2Int& pos, const math::Float depth)
    {
        assert(0 <= pos.x && (std::size_t)(pos.x) < m_width);
        assert(0 <= pos.y && (std::size_t)(pos.y) < m_height);
        assert(0 <= depth && depth <= 1);

        // reverse depth:
        // 1: near
        // 0: far

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        if (depth > m_depth_buf[idx]) {
            m_depth_buf[idx] = depth;
            return true;
        }
        return false;
    }

    const math::Transform2D& screen_to_window_transform() const { return m_screen_to_window; }

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
            m_depth_buf[i] = DEFAULT_DEPTH;
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
static_assert(asciirast::FrameBuffer_DepthSupport<PPMBuffer>);
