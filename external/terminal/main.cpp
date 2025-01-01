// #include "math/transform.h"
// #include "math/types.h"
// #include "program.h"

#include "math/math/Mat.h"
#include "math/math/Vec.h"

#include <iostream>

using namespace asciirast;
using namespace asciirast::math;

extern void get_terminal_size(int& out_width, int& out_height);
extern void just_fix_windows_console(bool revert);

int main() {
    just_fix_windows_console(true);

    auto v = Vec<3, float>{1.f, 2.f, 3.f};

    v.xyz = v.zyx.to_vec();

    std::cout << v.xyz << "\n";

    // v.xyz = v.xyz + v.xyz;
    // v.y += v.y;

    auto m = Mat<4, 3, float>::from_columns(v, v, v);
    std::cout << m << "\n\n";
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

    just_fix_windows_console(false);
}
