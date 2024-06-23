
#include "misc.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

int get_current_time_ms(void) {
    // taken from:
    // https://stackoverflow.com/a/44896326

    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (int)((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

// reference:
// https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html

static struct termios orig_termios;

static void disable_raw_mode(void) {
    // restore previous value
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);

    // Enable line buffering
    setvbuf(stdout, NULL, _IOLBF, 0);
}

void enable_raw_mode(void) {
    // store previous value
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;

    // disable some things
    raw.c_oflag &= (unsigned int)~(OPOST);  // turn off output processing
    raw.c_lflag &= (unsigned int)~(ECHO);   // turn off echoing, when characters are typed
    raw.c_lflag &= (unsigned int)~(ICANON); // read byte by byte - disable cannonical mode

    raw.c_cc[VMIN] = 0;  // minimum number of characters before read() can return
    raw.c_cc[VTIME] = 1; // wait 100 millisec before timeout read()

    // use new value
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    // Disable output buffering
    setvbuf(stdout, NULL, _IONBF, 0);
}

bool on_key(char* char_dest) {
    if (read(STDIN_FILENO, char_dest, 1) == 1) {
        return true;
    }
    return false;
}
