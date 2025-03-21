#pragma once

#include <generator>

#include "../math.h"

namespace asciirast::rasterize {

static std::generator<math::F>
generate_depth(const math::F& len, const math::F& len_inv, const math::F& depth0, const math::F& depth1)
{
    math::F acc = depth0;
    const math::F inc = (depth1 - depth0) * len_inv;

    const auto len_i = static_cast<math::I>(len);
    for (math::I i = 0; i < len_i; i++) {
        co_yield acc;
        acc += inc;
    }
}

}
