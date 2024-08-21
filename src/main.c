
#include "engine.h"

#include "scenes/ascii_palettes.h"

#include "scenes/spiral_scene.h"
#include "scenes/star_scene.h"
#include "scenes/cube_scene.h"

int main(void) {
    engine_run(g_star_scene, sizeof(ASCII_SHORT_PALETTE), ASCII_SHORT_PALETTE);
    return 0;
}
