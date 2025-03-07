#pragma once

#include <string>

namespace terminal_utils {

namespace CSI {

static constexpr std::string ESC = "\033[";
static constexpr std::string MOVEUPLINES = "A";
static constexpr std::string SHOWCURSOR = "?25h";
static constexpr std::string HIDECURSOR = "?25l";
static constexpr std::string CLEARLINE = "2K";
static constexpr std::string SETBG_RGBCOLOR = "48;2;";
static constexpr std::string SETFG_RGBCOLOR = "38;2;";
static constexpr std::string RESETCOLOR = "0m";

};

void
get_terminal_size(int& out_width, int& out_height);

void
just_fix_windows_console(bool enable);

}
