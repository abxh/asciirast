
#pragma once

#include "../../asciirast/framebuffer.h"
#include "../../asciirast/math/types.h"
#include "../../asciirast/utils.h"

#include "./terminal_utils.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace math = asciirast::math;
namespace CSI = terminal_utils::CSI;

struct RGBC
{
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    char c;
};

using RGBFloat = math::Vec3;

class TerminalBuffer
{
public:
    using Targets = std::tuple<char, RGBFloat>;

    TerminalBuffer(const math::Float aspect_ratio_scaling)
            : m_rgbc_buf{}
    {
        terminal_utils::just_fix_windows_console(true);
        std::cout << CSI::ESC << CSI::HIDE_CURSOR;

        m_aspect_ratio_scaling = aspect_ratio_scaling;
        m_oob_error = false;

        m_width = m_height = 0;
        this->clear_and_update_size();
    }
    ~TerminalBuffer()
    {
        std::cout << CSI::ESC << CSI::SHOW_CURSOR;
        std::cout << CSI::ESC << CSI::RESET_COLOR;
        terminal_utils::just_fix_windows_console(false);
    }
    TerminalBuffer(const TerminalBuffer& that) = default;
    TerminalBuffer& operator=(const TerminalBuffer& that) = default;

    bool out_of_bounds_error_occurred() const { return m_oob_error; }

    math::Vec2Int size() const { return { m_width, m_height }; }

    const char& at(const math::Vec2Int& pos) const
    {
        return m_rgbc_buf[index((std::size_t)pos.y, (std::size_t)pos.x)].c;
    }

    math::Float aspect_ratio() const { return m_aspect_ratio_scaling * (math::Float)m_height / (math::Float)m_width; }

    const math::Transform2D& screen_to_window_transform() const { return m_screen_to_window; }

    void plot(const math::Vec2Int& pos, const Targets& targets)
    {
        if (!(0 <= pos.x && (std::size_t)pos.x < m_width && 0 <= pos.y && (std::size_t)pos.y < m_height)) {
            m_oob_error = true;
            return;
        }

        const auto idx = index((std::size_t)pos.y, (std::size_t)pos.x);
        const auto [r, g, b] = std::get<RGBFloat>(targets).array();

        m_rgbc_buf[idx].r = static_cast<std::uint8_t>(255.f * r);
        m_rgbc_buf[idx].g = static_cast<std::uint8_t>(255.f * g);
        m_rgbc_buf[idx].b = static_cast<std::uint8_t>(255.f * b);
        m_rgbc_buf[idx].c = std::get<char>(targets);
    }

    void render() const
    {
        // int casting is neccessary:
        // https://stackoverflow.com/questions/19562103/uint8-t-cant-be-printed-with-cout

        this->reset_printer();

        for (std::size_t y = 0; y < m_height; y++) {
            for (std::size_t x = 0; x < m_width; x++) {
                const auto idx = index(y, x);
                const auto [r, g, b, c] = m_rgbc_buf[idx];

                std::cout << CSI::ESC << CSI::SET_FG_RGB_COLOR << int{ r } << ";" << int{ g } << ";" << int{ b } << "m"
                          << c;
            }
            std::cout << "\n";
        }
        std::cout << CSI::ESC << CSI::RESET_COLOR;

        std::fflush(stdout);
    }

    void clear(const char clear_char = ' ')
    {
        for (std::size_t i = 0; i < m_height * m_width; i++) {
            m_rgbc_buf[i] = { .r = 0, .g = 0, .b = 0, .c = clear_char };
        }
    }

    bool clear_and_update_size(const char clear_char = ' ')
    {
        int new_width = 0, new_height = 0;
        terminal_utils::get_terminal_size(new_width, new_height);

        if (m_width == (std::size_t)(new_width - 1) && m_height == (std::size_t)(new_height - 1)) {
            this->clear(clear_char);
            return false;
        }
        new_width = std::max(2, new_width - 1);
        new_height = std::max(2, new_height - 1);

        this->reset_printer();

        m_width = (std::size_t)new_width;
        m_height = (std::size_t)new_height;

        m_screen_to_window = asciirast::SCREEN_BOUNDS //
                                     .to_transform()
                                     .inversed()
                                     .reflectY()
                                     .translate(0, 1.f)
                                     .scale(m_width - 1, m_height - 1);

        m_rgbc_buf.resize(m_width * m_height);

        this->offset_printer();
        this->clear(clear_char);
        return true;
    }

private:
    std::size_t index(const std::size_t y, const std::size_t x) const { return m_width * y + x; }
    void reset_printer() const
    {
        for (std::size_t y = 0; y < m_height; y++) {
            std::cout << CSI::ESC << CSI::MOVE_UP_LINE << "\r";
        }
    }
    void offset_printer() const
    {
        for (std::size_t y = 0; y < m_height; y++) {
            std::cout << CSI::ESC << CSI::CLEAR_LINE << "\n";
        }
    }
    bool m_oob_error;
    math::Float m_aspect_ratio_scaling;

    std::size_t m_width;
    std::size_t m_height;
    math::Transform2D m_screen_to_window;

    std::vector<RGBC> m_rgbc_buf;
};

static_assert(asciirast::FrameBufferInterface<TerminalBuffer>);
