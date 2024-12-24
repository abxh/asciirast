
#include <array>

#include "types.h"

namespace asciirast::math {

using usize = std::size_t;

template <template <usize, typename> class V, usize N, typename T, usize... Is>
class Swizzled {
private:
    static constexpr usize M = sizeof...(Is);

private:
    T e[N];

public:
    V<M, T> as_vector() const
    {
        return V<M, T>{this->e[Is]...};
    }
    operator V<M, T>() const
    {
        return this->as_vector();
    }
    Swizzled &operator=(const V<M, T> &rhs)
    {
        usize indicies[] = {Is...};
        for (usize i = 0; i < M; i++) {
            this->e[indicies[i]] = rhs.e[i];
        }
        return *this;
    }
};

} // namespace asciirast::math
