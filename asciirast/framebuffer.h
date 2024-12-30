/**
 * @file framebuffer.h
 * @brief The framebuffer interface.
 */

#pragma once

#include "color/color.h"

namespace asciirast {

template<typename ...Outputs>
class Framebuffer {
public:
    virtual ~Framebuffer() = default;

    virtual bool plot(std::tuple<...Outputs> attrs);
};

}  // namespace asciirast
