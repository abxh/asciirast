
#include <iostream>

#include "math/types.h"
#include "math/utils.h"

using namespace asciirast;

int
main()
{
    // math::transform2 t{};
    // t.rotate_counterclockwise(math::angle_as_radians(45.f));
    // t.rotate_counterclockwise(math::angle_as_radians(45.f));
    // t.rotate_counterclockwise(math::angle_as_radians(45.f));
    // t.rotate_counterclockwise(math::angle_as_radians(45.f));

    // auto v = math::vec2{1.f, 0.f};
    // v += -v;
    // std::cout << t.apply(v);

    auto rot = math::Rot3(math::Vec3{ 0, 1, 0 }, math::angle_as_radians(90.f));
    std::cout << rot.to_mat3();

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
