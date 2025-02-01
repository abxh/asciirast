
#include <iostream>

#include "math/Complex.h"
#include "math/Transform.h"

using namespace asciirast::math;

int main() {
    Transform2D<float> t{};
    t.rotate_counterclockwise(Angle<float>::from_deg(45));
    t.rotate_counterclockwise(Angle<float>::from_deg(45));
    t.rotate_counterclockwise(Angle<float>::from_deg(45));
    t.rotate_counterclockwise(Angle<float>::from_deg(45));
    t.rotate_counterclockwise(Angle<float>::from_deg(45));

    auto v = Vec2<float>{1.f, 0.f};

    std::cout << t.apply(v);

    // auto v = Vec<4, float>{1.f, 2.f, 3.f, 4.f};
    //
    // // std::cout << Vec<3, float>{m.column_range(0)} << "\n";
    // // std::cout << m.column_get(1) << "\n";
    // // std::cout << m.column_get(2) << "\n";
    //
    // auto m = Mat<4, 4, float, true>::from_columns(v, v, v);
    // std::cout << m << "\n\n";
    //
    // auto t = Transform2D();
    // t.translate(0, 0).rotate_clockwise(1).scale(1, 2);
    //
    // std::cout << (m * 5.f) * v << "\n\n";

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
