#include "../example_scenes/example_scenes.h"
#include "misc.h"
#include "scene.h"
#include "screen.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef MS_PER_UPDATE
#define MS_PER_UPDATE 200
#endif

static void on_sigint(int sig) {
    MARK_UNUSED(sig);
    screen_deinit();
    exit(0);
}

int main(void) {
    enable_raw_mode();
    screen_init();
    signal(SIGINT, on_sigint);

    scene_type scene = g_rgb_triangle_scene; // change scene here
    void** context_ptr = scene.create();

    bool on_running = true;
    int previous_time = get_current_time_ms();
    int lag = 0.;

    while (on_running) {
        const int current_time = get_current_time_ms();
        const int elapsed = current_time - previous_time;
        previous_time = current_time;
        lag += elapsed;

        char c;
        if (on_key(&c)) {
            if (c == 'q') {
                on_running = false;
            }
            if (scene.flags & SCENE_OPS_ON_KEY) {
                scene.on_key(context_ptr, c);
            }
        }

        while (lag >= MS_PER_UPDATE) {
            scene.update(context_ptr);
            lag -= MS_PER_UPDATE;
        }

        scene.render(context_ptr);

        printf("q: exit" NEW_LINE);
        g_extra_lines += 1;
        screen_restore_line_cursor();

        screen_refresh();
        screen_clear();
    }

    scene.destroy(context_ptr);
    screen_deinit();

    return 0;
}
