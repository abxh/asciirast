
#include <iostream>

#include "math/types/Vec.h"

using namespace asciirast::math;

int main() {
    auto v = Vec<2, float>{1.f, 1.f};
    auto n = Vec<2, float>{-1.f, -1.f};

    std::cout << v.reflect(n) << "\n";

    // auto m = Mat<4, 3, float>::from_columns(v, v, v);
    // std::cout << m << "\n\n";

    // auto t = Transform2D();
    // t.translate(0, 0).rotate_clockwise(1).scale(1, 2);

    // std::cout << m * 5.f * v << "\n\n";

    // Vec<2, int> w = v.xy;

    // std::cout << transform::scaling_inv(0.f, 0.f) * Vec3f{1.f, 2.f, 1.f} <<
    // "\n";
    //
    // struct Attr {
    //     float a;
    //     Attr operator+(const Attr& other) { return Attr{this->a + other.a}; }
    //     Attr operator*(const float& scalar) { return Attr{this->a * scalar};
    //     }
    // };
    //
    // class MyProgram : public Program<Attr, Attr, Attr> {
    // public:
    //     void on_vertex(const Attr& uniform,
    //                    const Attr& in,
    //                    Attr& out) override {}
    //     void on_pixel(const Attr& uniform,
    //                   const Attr& in,
    //                   math::Vec4f& out) override {}
    // };
}
