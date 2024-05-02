#include <signal.h>
#include <stdbool.h>
#include <stdint.h>

#include "misc.h"
#include "screen.h"

#include "objects/object.h"
#include "objects/star.h"

void on_sigint(int sig) {
    screen_deinit();
    exit(0);
}

int main(void) {
    signal(SIGINT, on_sigint);
    screen_init();

    void** star_context_ptr = star_ops.create();

    bool on_running = true;
    while (on_running) {
        star_ops.update(star_context_ptr);

        screen_refresh();
        screen_clear();

        sleep_portable(200);
    }

    star_ops.destroy(star_context_ptr);
    screen_deinit();

    return 0;
}
