#include "math/vec.h"
#include "utils.h"

#include <iostream>

int main()
{
    using namespace asciirast::math;

    Vec4<int> a{1, 2, 3, 4};
    std::cout << "dot product: " << a.dot(a) << std::endl;

    // a = a + a;

    int width, height;
    get_terminal_size(width, height);
    std::cout << "terminal size: " << width << " x " << height << std::endl;

    std::cout << a.x + a.y << std::endl;
    // a.xy = a.xy.as_vec() + a.zw.as_vec();

    for (const auto x : a) {
        std::cout << x << ", ";
    }
}
