#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "hotreload.h"
#include "render.h"
#include "camera.h"
#include "input.h"

static void cleanup(void) {
    input_restore();
    printf("\x1b[?25h"); // show the cursor again
    fflush(stdout);
}

int main(void) {
    atexit(cleanup);
    input_init();

    hot_scene_t hs;
    if (hotreload_init(&hs, "src/scene.c") != 0) {
        return 1;
    }

    Camera cam = camera_init();
    double start = now_seconds();
    double last_frame = start;
    static char buf[1 << 22];

    printf("\x1b[?25l\x1b[2J");
    fflush(stdout);

    for (;;) {
        double now = now_seconds();
        float dt = (float)(now - last_frame);
        last_frame = now;

        InputState in = input_poll();
        if (in.quit) break;
        camera_update(&cam, in, dt);

        hotreload_poll(&hs, "src/scene.c");
        render_scene(hs.scene_sdf, start, cam, buf, sizeof(buf));
    }

    return 0;
}
