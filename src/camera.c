#include "camera.h"
#include "render_math.hpp"

#define MOVE_SPEED 4.0f   // world units per second
#define LOOK_SPEED 2.0f   // radians per second
#define MAX_PITCH  1.4f   // a bit under 90 degrees; avoids flipping over the pole

Camera camera_init(void) {
    Camera cam;
    cam.pos = (Vec3){0.0f, 0.0f, -10.0f};
    cam.yaw = 0.0f;
    cam.pitch = 0.0f;
    // forward/right/up get filled in properly on the first camera_update()
    // call; these just match yaw=0, pitch=0 so nothing looks wrong for
    // one frame if that were ever skipped.
    cam.forward = (Vec3){0.0f, 0.0f, 1.0f};
    cam.right = (Vec3){-1.0f, 0.0f, 0.0f};
    cam.up = (Vec3){0.0f, 1.0f, 0.0f};
    return cam;
}

void camera_update(Camera *cam, InputState in, float dt) {
    // --- look (IJKL), accumulate into yaw/pitch ---
    if (in.l) cam->yaw -= LOOK_SPEED * dt;
    if (in.j) cam->yaw += LOOK_SPEED * dt;
    if (in.k) cam->pitch += LOOK_SPEED * dt;
    if (in.i) cam->pitch -= LOOK_SPEED * dt;
    if (cam->pitch > MAX_PITCH) cam->pitch = MAX_PITCH;
    if (cam->pitch < -MAX_PITCH) cam->pitch = -MAX_PITCH;

    // Rebuild "forward" from yaw/pitch using render_math's own rotation
    // transforms (Ry(yaw) * Rx(pitch) * base_forward), rather than
    // recomputing rotation by hand -- this is exactly what render_rotate_x
    // / render_rotate_y / render_transform_dir already do, so we just
    // reuse them.
    Vec3 base_forward = (Vec3){0.0f, 0.0f, 1.0f};
    Vec3 pitched = render_transform_dir(render_rotate_x(cam->pitch), base_forward);
    cam->forward = vec3_normalize(render_transform_dir(render_rotate_y(cam->yaw), pitched));

    // right/up are just "what's perpendicular to forward" -- a plain
    // cross product, not a rotation transform, so there's nothing to
    // reuse from render_math here; this is the same derivation the
    // original fixed camera used (right = forward x world-up).
    cam->right = vec3_normalize(vec3_cross(cam->forward, (Vec3){0.0f, 1.0f, 0.0f}));
    cam->up = vec3_cross(cam->right, cam->forward);

    // --- move (WASD), relative to the (now-updated) look direction ---
    if (in.w) cam->pos = vec3_add(cam->pos, vec3_scale(cam->forward, MOVE_SPEED * dt));
    if (in.s) cam->pos = vec3_sub(cam->pos, vec3_scale(cam->forward, MOVE_SPEED * dt));
    if (in.a) cam->pos = vec3_sub(cam->pos, vec3_scale(cam->right, MOVE_SPEED * dt));
    if (in.d) cam->pos = vec3_add(cam->pos, vec3_scale(cam->right, MOVE_SPEED * dt));

    // Use cam->up (not world-Y) so this stays orthogonal to forward/right
    // no matter the pitch -- up = cross(right, forward) is guaranteed
    // perpendicular to both by construction, so it never collapses onto
    // w/s the way world-Y would when looking straight up or down.
    if (in.r) cam->pos = vec3_add(cam->pos, vec3_scale(cam->up, MOVE_SPEED * dt));
    if (in.f) cam->pos = vec3_sub(cam->pos, vec3_scale(cam->up, MOVE_SPEED * dt));
}