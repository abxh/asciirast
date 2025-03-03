#pragma once

#define CSI_ESC "\033["
#define CSI_MOVEUPLINES "A"
#define CSI_SHOWCURSOR "?25h"
#define CSI_HIDECURSOR "?25l"
#define CSI_CLEARLINE "2K"
#define CSI_SETBG_RGBCOLOR "48;2;"
#define CSI_SETFG_RGBCOLOR "38;2;"
#define CSI_RESETCOLOR "0m"

void
get_terminal_size(int& out_width, int& out_height);

void
just_fix_windows_console(bool enable);
