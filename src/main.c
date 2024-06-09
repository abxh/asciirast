#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "misc.h"
#include "screen.h"

#include "scenes/scene.h"
#include "scenes/star_scene.h"
#include "scenes/spiral_scene.h"
#include "scenes/cube_scene.h"
#include "scenes/wavy_triangle_scene.h"
#include "scenes/rotating_triangle_scene.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

void on_sigint(int sig) {
    screen_deinit();
    exit(0);
}

#pragma GCC diagnostic pop

int main(void) {
    signal(SIGINT, on_sigint);
    screen_init();

    scene_type scene = rotating_triangle_scene;
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
