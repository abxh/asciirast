#include "math/Vec.h"
// #include "math/Mat.h"

#include <iostream>

int main()
{
    using namespace asciirast::math;

    auto v = Vec<3, float>(1.f, 2.f, 3.f);
    auto w = Vec<3, float>(3.f, -2.f, 1.f);

    std::cout << w.angle(v);

    // auto m = Mat<3, 3, int>(1);

    // for (auto vec : m.columns()) {
    //     std::cout << vec << "\n";
    // }


}
