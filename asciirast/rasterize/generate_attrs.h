#pragma once

#include <cfloat>
#include <generator>

#include "../math.h"
#include "../program.h"

namespace asciirast::rasterize {

template<VaryingType Varying>
static std::generator<Varying>
generate_attrs(const math::F& len,
               const math::F& len_inv,
               const ProjectedFragment<Varying>& proj0,
               const ProjectedFragment<Varying>& proj1)
{
    auto perspective_corrected = [&proj0, &proj1](const math::F t) {
        if (!std::isfinite(proj0.depth)) {
            return t;
        }
        return t * proj0.depth / ((1 - t) * proj0.depth + t * proj1.depth);
    };

    math::F acc_t = 0;
    const math::F inc_t = len_inv;

    const auto len_i = static_cast<math::I>(len);
    for (math::I i = 0; i < len_i; i++) {
        co_yield lerp(proj0.attrs, proj1.attrs, perspective_corrected(acc_t));
        acc_t += inc_t;
    }
}

}
