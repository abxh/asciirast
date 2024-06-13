#pragma once

#include <stdbool.h>

#define STR(s) #s
#define XSTR(s) STR(s)

#define CSI_ESC "\033["
#define CSI_UP "A"
#define CSI_SHOWCURSOR "?25h"
#define CSI_HIDECURSOR "?25l"
#define CSI_CLEARLINE "2K"

#define CLEAR_LINE() printf(CSI_ESC CSI_CLEARLINE)
#define MOVE_UP_LINES(num_lines) printf(CSI_ESC XSTR(num_lines) CSI_UP)

#define IMPLIES(a, b) (!(a) || (b))

#define SWAP_UNSAFE(T, a, b) \
    do {                     \
        T t = (a);           \
        (b) = (a);           \
        (a) = t;             \
    } while (false)

#define MARK_UNUSED(x) (void)(x)

void sleep_ms(int milliseconds);
