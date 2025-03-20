#pragma once

namespace asciirast::detail {

// template<ProjectedFragmentType ProjectedFragment, typename Callable, typename... Args>
//     requires(std::invocable<Callable, const ProjectedFragment &&, Args...>)
// static void
// plot_line(Callable plot, const ProjectedFragment& frag0, const ProjectedFragment& frag1, Args&&... args)
// {
//     // line drawing based on linear interpolation:
//     // https://www.redblobgames.com/grids/line-drawing/#more
//
//     const auto p_delta = frag1.pos - frag0.pos;
//     const auto max_len = static_cast<math::I>(std::max(std::abs(p_delta.x), std::abs(p_delta.y)));
//
//     if (max_len == 0U) {
//         return;
//     }
//
//     auto attr_t_func = [&](math::F t) -> math::F {
//         // perspective-corrected / hyperbolic interpolation
//         if (!std::isfinite(frag0.depth) || !std::isfinite(frag1.depth)) {
//             return t;
//         }
//         return t * frag0.depth / ((1 - t) * frag0.depth + t * frag1.depth);
//     };
//
//     const auto t_step = math::F{ 1 } / max_len;
//     const auto p_step = t_step * p_delta;
//     const auto d_step = t_step * (frag1.depth - frag0.depth);
//
//     auto t_curr = math::F{ 0 };
//     auto p_curr = frag0.pos;
//     auto d_curr = frag0.depth;
//
//     for (math::I i = 0; i <= max_len; i++) {
//         plot(ProjectedFragment{ .pos = math::floor(p_curr),
//                                 .depth = d_curr,
//                                 .attrs = lerp(frag0.attrs, frag1.attrs, attr_t_func(t_curr)) },
//              std::forward<Args>(args)...);
//
//         t_curr += t_step;
//         p_curr += p_step;
//         d_curr += d_step;
//     }
// }

}
