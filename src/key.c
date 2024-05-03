// TODO:
// Continue following the article. Implement threads for reading and writing at the same time...?

// Refer to:
// https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html

#include <stdbool.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

bool get_key(char* char_dest) {
    if (read(STDIN_FILENO, char_dest, 1) == 1) {
        return true;
    }
    return false;
}
