#include "math/Mat.h"
#include "math/Vec.h"
#include "math/transformations.h"

#include <iostream>

using namespace asciirast::math;

int main() {
    auto v = Vec<4, int>{1, 2, Vec<2, int>{3, 4}};

    Vec<2, int> w = v.xy;

    // w.x = w.x + w.y;

    // v.yx = Vec<2, int>{} + v.ww + v.ww;

    // std::cout << v.dot(v) << "\n";

    // std::cout << Vec(v.xy + v.zw << "\n";

    auto m0 = mat3_translation();
    auto m = mat3_rotation(0.5f * pi_f);
    std::cout << (m0 * m) << "\n\n";
    std::cout << (m0 * m * m) << "\n\n";
    std::cout << (m0 * m * m * m) << "\n\n";
    std::cout << (m0 * m * m * m * m) << "\n\n";

    // std::cout << "original matrix (5 x 4):\n" << m << "\n\n";
    // std::cout << "m * v:\n" << m * v;
    // std::cout << "m * m.transposed() (should be 5x5):\n"
    //           << m * m.transposed() << "\n\n";
    // std::cout << "m.transposed() * m (should be 4x4):\n"
    //           << m.transposed() * m << "\n\n";

    // v.y = 1;
    // auto w = Vec3f(Vec2f(5.f, 0.55555555555f), -0.1232135555f);
    //
    // std::cout << v.angle(w) << std::endl;
    //
    // std::cout << VecStr(w);

    // std::cout << "matrix:\n"
    //           << MatStr(Mat4x4f(Mat3x3f::scaling(1.f, 2.f, 3.f))) << "\n\n";
    // std::cout << "value:\n" << Vec<1, int>{59}.dot(Vec<1, int>{2}) + 2;
}
