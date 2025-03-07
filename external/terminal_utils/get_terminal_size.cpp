/**
 * @file get_terminal_size.cpp
 *
 * Based on:
 * https://stackoverflow.com/a/62485211
 */

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <cstdio>
#include <sys/ioctl.h>
#endif

#include "terminal_utils.h"

void
get_terminal_size(int& out_width, int& out_height)
{
    out_width = out_height = 0;
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    out_width = (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    out_height = (int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
#elif defined(__linux__) || defined(__APPLE__)
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    out_width = (int)(w.ws_col);
    out_height = (int)(w.ws_row);
#endif // Windows/Linux
}
