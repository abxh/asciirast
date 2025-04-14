#include "asciirast/math/types.h"

namespace asciirast::math {

extern template class Vec<2, Int>;
extern template class Vec<3, Int>;
extern template class Vec<4, Int>;
extern template class Vec<4, std::uint8_t>;

extern template class Vec<2, Float>;
extern template class Vec<3, Float>;
extern template class Vec<4, Float>;

extern template class Mat<2, 2, Float, uses_col_major>;
extern template class Mat<3, 3, Float, uses_col_major>;
extern template class Mat<4, 4, Float, uses_col_major>;

extern template class Rot2<Float, uses_col_major>;
extern template class Rot2<Float, uses_col_major>;

extern template class Transform2<Float, uses_col_major>;
extern template class Transform3<Float, uses_col_major>;

extern template class AABB<2, Float, uses_col_major>;
extern template class AABB<3, Float, uses_col_major>;

};
