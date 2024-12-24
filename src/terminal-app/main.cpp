// #include "math/MatBase.h"
#include "math/Vec.h"

#include <iostream>

int main()
{
    using namespace asciirast::math;

    auto v = Vec<3, float>(Vec<2, float>(1.f, 2.f), 3.f);
    auto w = Vec<3, float>(Vec<2, float>(5.f, 0.f), 0.f);

    // std::cout << v.angle() << std::endl;

    //
    // // std::cout << v;
    //
    // auto m = MatBase<3, 2, int>::from_rows(
    //     Vec<2, int>(1, 2), Vec<2, int>(3, 4), Vec<2, int>(5, 6));
    //
    // // m.column_set(1, Vec<5, double>{1., 2., 3., 4., 5.});
    // // m.row_set(2, Vec<3, double>{1., 2., 3.});
    //
    // std::cout << "matrix:\n" << m << "\n\n";
    // // std::cout << "value:\n" << Vec<1, int>{59}.dot(Vec<1, int>{2}) + 2;
}
