/**
 * @file just_fix_windows_console.cpp
 *
 * Based on:
 * https://github.com/niansa/colohalopp/blob/master/colorama.cpp
 * , which is itself a port of python-colorama.
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include "./terminal_utils.hpp"

namespace terminal_utils {

void
just_fix_windows_console(bool enable)
{
#ifdef _WIN32
    HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(handleOut, &consoleMode);
    if (enable) {
        consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    } else {
        consoleMode &= ~ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    }
    SetConsoleMode(handleOut, consoleMode);
#else
    (void)enable;
#endif
}

}
