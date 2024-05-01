#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "draw.h"
#include "screen.h"
#include "transform.h"

#define CSI_ESC "\033["
#define CSI_UP "A"
#define CSI_MOVTOFRONT "\r"

void sleep_portable(size_t millisec) {
#ifdef _WIN32
#include <windows.h>
    Sleep(milliSec);
#elif defined(__linux)
#include <unistd.h>
    usleep(millisec * 1000); // microseconds
#endif
}

void on_sigint(int sig) {
    screen_deinit();
    exit(0);
}

int main(void) {

    signal(SIGINT, on_sigint);
    screen_init();

    int64_t angle_deg = 0;

    while (true) {
        for (size_t o = 0; o < 6; o++) {
            for (size_t i = 0; i <= 6; i++) {
                float x1f = rotate_y_around_origo(0.6 - 0.1 * i, 0.6 - 0.1 * i, to_angle_rad(-angle_deg - 20. * i - 60 * o));
                float y1f = rotate_x_around_origo(0.6 - 0.1 * i, 0.6 - 0.1 * i, to_angle_rad(-angle_deg - 20. * i - 60 * o));
                draw_point(x1f, y1f, '*');
            }
        }

        screen_refresh();
        screen_clear();

        if (angle_deg == 350) {
            angle_deg = 0;
        }
        angle_deg += 10;
        sleep_portable(200);
    }

    screen_deinit();

    return 0;
}
