#include "misc.h"
#include "screen.h"
#include "scene.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// hotplug:
#include "../example_scenes/example_scenes.h"

static void on_sigint(int sig) {
    MARK_UNUSED(sig);
    screen_deinit();
    exit(0);
}

int main(void) {
    signal(SIGINT, on_sigint);
    screen_init();

    scene_type scene = diamond_triangle_scene;
    void** context_ptr = scene.create();

    bool on_running = true;
    while (on_running) {
        scene.update(context_ptr);

        screen_refresh();
        screen_clear();

        sleep_ms(200);
    }

    scene.destroy(context_ptr);
    screen_deinit();

    return 0;
}
