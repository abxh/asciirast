/**
 * @file color.h
 * @brief Color abstraction
 *
 * See:
 * https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
 */

#pragma once

#include "math/types.h"

#include <cstdint>

namespace asciirast::color {

enum class ColorTypes { VGA = 0, ColorTypesCount };  // more yet to be supported

template <ColorTypes>
class Color;

template <>
class Color<ColorTypes::VGA> {
public:
    enum class VGAColors {
        None,
        Black = 30,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        BrightBlack = 90,
        BrightRed,
        BrightGreen,
        BrightYellow,
        BrightBlue,
        BrightMagenta,
        BrightCyan,
        BrightWhite,
    };

public:
    VGAColors m_color;

public:
    Color(VGAColors color) : m_color{color} {}

    /**
     * @brief Get the foreground ansi color code.
     */
    uint8_t fg();

    /**
     * @brief Get the background ansi color code.
     */
    uint8_t bg();
};

}  // namespace asciirast::color
