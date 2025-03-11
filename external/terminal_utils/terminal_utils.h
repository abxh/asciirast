#pragma once

#include <string>

namespace terminal_utils {

namespace CSI {

static const std::string ESC = "\033[";
static const std::string RESET_COLOR = "0m";

static const std::string SHOW_CURSOR = "?25h";
static const std::string HIDE_CURSOR = "?25l";

static const std::string ENABLE_LINE_WRAP = "7h";
static const std::string DISABLE_LINE_WRAP = "7l";

static const std::string MOVE_UP_LINES = "A";
static const std::string CLEAR_LINE = "2K";

static const std::string SET_BG_RGB_COLOR = "48;2;";
static const std::string SET_FG_RGB_COLOR = "38;2;";

};

void
get_terminal_size(int& out_width, int& out_height);

void
just_fix_windows_console(bool enable);

}
