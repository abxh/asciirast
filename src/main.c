#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "misc.h"
#include "screen.h"

#include "scenes/scene.h"
#include "scenes/star.h"
#include "scenes/spiral.h"

void on_sigint(int sig) {
    screen_deinit();
    exit(0);
}

int main(void) {
    signal(SIGINT, on_sigint);
    screen_init();

    scene_type scene = star_scene;
    void** context_ptr = scene.create();

    bool on_running = true;
    while (on_running) {
        scene.update(context_ptr);

        screen_refresh();
        screen_clear();

        sleep_portable(200);
    }

    scene.destroy(context_ptr);
    screen_deinit();

    return 0;
}
