#include "math/transform.h"
#include "math/types.h"
#include "program.h"

#include <iostream>

using namespace asciirast;
using namespace asciirast::math;

extern void get_terminal_size(int& out_width, int& out_height);
extern void just_fix_windows_console(bool revert);

int main() {
    just_fix_windows_console(true);

    auto v = Vec<3, int>{1, 2, 0};

    // v.xyz = v.xyz + v.xyz;
    // v.y += v.y;

    // Vec<2, int> w = v.xy;

    std::cout << transform::scaling_inv(0.f, 0.f) * Vec3f{1.f, 2.f, 1.f} << "\n";

    struct Attr {
        float a;
        Attr operator+(const Attr& other) { return Attr{this->a + other.a}; }
        Attr operator*(const float& scalar) { return Attr{this->a * scalar}; }
    };

    class MyProgram : public Program<Attr, Attr, Attr> {
    public:
        void on_vertex(const Attr& uniform,
                       const Attr& in,
                       Attr& out) override {}
        void on_pixel(const Attr& uniform,
                      const Attr& in,
                      math::Vec4f& out) override {}
    };

    just_fix_windows_console(false);
}
