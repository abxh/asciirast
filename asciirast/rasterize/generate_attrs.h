#pragma once

#include <cfloat>
#include <generator>

#include "../math.h"
#include "../program.h"

namespace asciirast::rasterize {

template<VaryingType Varying>
static std::generator<Varying>
generate_attrs_lerp(const math::F& len,
                      const math::F& len_inv,
                      const ProjectedFragment<Varying>& proj0,
                      const ProjectedFragment<Varying>& proj1)
{
    const Varying inc = len_inv * (proj1.attrs + -1 * proj0.attrs);

    Varying acc = proj0.attrs;

    for (math::I i = 0; i < static_cast<math::I>(len); i++) {
        co_yield acc;

        acc = std::move(acc + inc);
    }
}

template<VaryingType Varying>
static std::generator<Varying>
generate_attrs_perspective_corrected(const math::F& len,
                                     const math::F& len_inv,
                                     const ProjectedFragment<Varying>& proj0,
                                     const ProjectedFragment<Varying>& proj1)
{
    auto perspective_corrected = [&proj0, &proj1](const math::F t) {
        return t * proj0.depth / ((1 - t) * proj0.depth + t * proj1.depth);
    };

    const math::F inc_t = len_inv;

    math::F acc_t = 0;

    for (math::I i = 0; i < static_cast<math::I>(len); i++) {
        co_yield lerp(proj0.attrs, proj1.attrs, perspective_corrected(acc_t));

        acc_t += inc_t;
    }
}

template<VaryingType Varying>
static std::generator<Varying>
generate_attrs(const math::F& len,
               const math::F& len_inv,
               const ProjectedFragment<Varying>& proj0,
               const ProjectedFragment<Varying>& proj1)
{
    if (std::isfinite(proj0.depth) && std::isfinite(proj1.depth)) {
        return generate_attrs_perspective_corrected(len, len_inv, proj0, proj1);
    } else {
        return generate_attrs_lerp(len, len_inv, proj0, proj1);
    }
}

}
