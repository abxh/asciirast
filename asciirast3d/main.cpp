#include "vec.hpp"

#include <cmath>
#include <iostream>

using namespace asciirast3d;

int main(void)
{
    auto v = Vec<2, float>({1, 2});
    v.yx = Vec<2, float>(3.f, 1.f);
    std::cout << v.yx.as_vec().x << "\n";
}
