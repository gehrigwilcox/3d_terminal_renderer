#ifndef RENDER_H
#define RENDER_H

#include <stddef.h>
#include "scene.h"
#include "camera.h"

// Monotonic clock in seconds, used to compute the animation time t.
double now_seconds(void);

// Raymarches the scene with scene_sdf from the given camera's point of
// view, and writes one ANSI-art frame straight to stdout. start is the
// process start time (from now_seconds()), used for scene animation
// time; buf/buf_size is scratch space for building the frame. The
// camera is owned by the caller (main.c) since its position/orientation
// must persist and accumulate across frames as the user moves it.
void render_scene(scene_sdf_fn scene_sdf, double start, Camera cam, char *buf, size_t buf_size);

#endif
