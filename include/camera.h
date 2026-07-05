#ifndef CAMERA_H
#define CAMERA_H

#include "mathlib.hpp"
#include "input.h"

// Unlike before (where render.c rebuilt a fixed camera every frame),
// this camera's position and orientation persist across frames so
// movement and look input can accumulate over time.
typedef struct {
    Vec3 pos;
    float yaw;   // rotation around the world Y axis: looking left/right
    float pitch; // tilt up/down, clamped to avoid flipping over the pole

    // Derived each frame by camera_update() from yaw/pitch -- render.c
    // just reads these, it never computes orientation itself.
    Vec3 forward, right, up;
} Camera;

// Starting position/orientation: matches the original fixed camera
// (looking at the origin from ten units back on -Z).
Camera camera_init(void);

// Applies WASD movement and IJKL look input, scaled by dt (seconds
// since last frame) so speed doesn't depend on frame rate.
void camera_update(Camera *cam, InputState in, float dt);

#endif
