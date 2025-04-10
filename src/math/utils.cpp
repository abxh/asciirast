#include "asciirast/math/types.h"

namespace asciirast::math {

extern template Float
radians(const Float);
extern template Float
degrees(const Float);

extern template bool
almost_equal(const Float, const Float, const unsigned);
extern template bool
almost_less_than(const Float, const Float, const unsigned);

};
