#include "math/types.h"
#include "program.h"

#include <iostream>

using namespace asciirast;
using namespace asciirast::math;

extern void get_terminal_size(int& out_width, int& out_height);

int main() {
    auto v = Vec<3, int>{1, 2, 3};

    // v.xyz = v.xyz + v.xyz;
    // v.y += v.y;

    // Vec<2, int> w = v.xy;

    std::cout << v.xyy + v << "\n";

    struct Attr {
        float a;
        Attr operator+(const Attr& other) { return Attr{this->a + other.a}; }
        Attr operator*(const float& other) { return Attr{this->a * other}; }
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
}
