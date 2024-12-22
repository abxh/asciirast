#include "math/Vec.h"
#include "utils.h"

#include <iostream>

int main()
{
    using namespace asciirast::math;

    auto v = Vec4<int>(1, Vec2<int>(2, 3), 4);

    (v.wzyx = v.xyxy) = v.yyyy;

    for (const auto x : v) {
        std::cout << x << ", ";
    }
}
