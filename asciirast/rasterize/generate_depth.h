#pragma once

#include <generator>

#include "../math.h"

namespace asciirast::rasterize {

static std::generator<math::F>
generate_depth(const math::F& len, const math::F& len_inv, const math::F& depth0, const math::F& depth1)
{
    const math::F inc = (depth1 - depth0) * len_inv;

    math::F acc = depth0;

    for (math::I i = 0; i < static_cast<math::I>(len); i++) {
        co_yield acc;

        acc += inc;
    }
}

}
