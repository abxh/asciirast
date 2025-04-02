#pragma once

#include <string>

namespace terminal_utils {

namespace CSI {

inline const std::string ESC = "\033[";
inline const std::string RESET_COLOR = "0m";

inline const std::string SHOW_CURSOR = "?25h";
inline const std::string HIDE_CURSOR = "?25l";

inline const std::string ENABLE_LINE_WRAP = "7h";
inline const std::string DISABLE_LINE_WRAP = "7l";

inline const std::string MOVE_UP_LINE = "A";
inline const std::string CLEAR_LINE = "2K";

inline const std::string SET_BG_RGB_COLOR = "48;2;";
inline const std::string SET_FG_RGB_COLOR = "38;2;";

};

void
get_terminal_size(int& out_width, int& out_height);

void
just_fix_windows_console(bool enable);

}
