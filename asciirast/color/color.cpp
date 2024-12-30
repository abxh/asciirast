#include "color.h"

namespace asciirast::color {

uint8_t Color<ColorTypes::VGA>::fg() {
    return static_cast<uint8_t>(m_color);
}

uint8_t Color<ColorTypes::VGA>::bg() {
    return static_cast<uint8_t>(m_color) + 10;
}

};  // namespace asciirast::color
