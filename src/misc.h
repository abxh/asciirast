#pragma once

#include <stdio.h>

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

void sleep_portable(size_t millisec);
