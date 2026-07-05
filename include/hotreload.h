#ifndef HOTRELOAD_H
#define HOTRELOAD_H

#include <time.h>
#include "scene.h"

// Tracks the currently loaded scene .so and everything needed to hot-swap it.
typedef struct {
    void *handle;
    scene_sdf_fn scene_sdf;
    time_t last_mtime;
    int version;
    char current_so_path[64];
} hot_scene_t;

// Compiles and loads scene_src for the first time. Returns 0 on success,
// -1 on failure (build log is printed to stderr).
int hotreload_init(hot_scene_t *hs, const char *scene_src);

// Call once per frame. If scene_src has changed on disk since the last
// call, recompiles it, swaps in the new version, and deletes the old
// .so file. If the rebuild fails, the previously loaded scene keeps
// running and the on-disk error log is left in place for inspection.
void hotreload_poll(hot_scene_t *hs, const char *scene_src);

#endif
