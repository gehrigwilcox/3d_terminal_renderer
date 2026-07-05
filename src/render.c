#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE

#include "render.h"

#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>

// Characters from darkest to brightest, used to "shade" a pixel based
// on how directly its surface faces the light.
static const char ramp[11] = " .:-=+*#%@";

// How far a ray can travel before we give up and call it a miss.
#define MAX_RAY_DISTANCE 30.0f
// How close to the surface (SDF value near 0) counts as a hit.
#define SURFACE_EPSILON 0.001f
#define MAX_MARCH_STEPS 96

double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// Reads the terminal size, falling back to sane defaults if it can't.
// Reserves one row for the status line printed after each frame.
static void get_terminal_size(int *cols, int *rows) {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    *cols = ws.ws_col > 0 ? ws.ws_col : 80;
    *rows = ws.ws_row > 1 ? ws.ws_row - 1 : 24;
}

// Converts a pixel's screen position (in [-1, 1] normalized device
// coordinates) into a world-space ray direction from the camera.
static Vec3 camera_ray_dir(Camera cam, float ndc_x, float ndc_y, float aspect) {
    Vec3 dir = vec3_add(
        vec3_add(vec3_scale(cam.right, ndc_x * aspect), vec3_scale(cam.up, ndc_y)),
        vec3_scale(cam.forward, 1.5f) // 1.5 = field-of-view "zoom"; smaller = wider FOV
    );
    return vec3_normalize(dir);
}

// Marches a ray forward in steps sized by the SDF's distance estimate,
// stopping when it's close enough to a surface to call it a hit, or
// once it's traveled too far to plausibly hit anything.
// Returns 1 and sets *hit_pos on a hit, 0 on a miss.
static int raymarch(scene_sdf_fn scene_sdf, float t, Vec3 origin, Vec3 dir, Vec3 *hit_pos) {
    float dist = 0.0f;
    for (int step = 0; step < MAX_MARCH_STEPS; step++) {
        Vec3 pos = vec3_add(origin, vec3_scale(dir, dist));
        float d = scene_sdf(pos, t);
        if (d < SURFACE_EPSILON) {
            *hit_pos = pos;
            return 1;
        }
        dist += d;
        if (dist > MAX_RAY_DISTANCE) break;
    }
    return 0;
}

// Estimates the surface normal at a hit point by sampling the SDF just
// off to each side (finite differences) -- the SDF gradient points away
// from the surface, which is exactly what a normal is.
static Vec3 estimate_normal(scene_sdf_fn scene_sdf, Vec3 p, float t) {
    float d = scene_sdf(p, t);
    float eps = SURFACE_EPSILON;
    Vec3 n = {
        scene_sdf(vec3_add(p, (Vec3){eps, 0.0f, 0.0f}), t) - d,
        scene_sdf(vec3_add(p, (Vec3){0.0f, eps, 0.0f}), t) - d,
        scene_sdf(vec3_add(p, (Vec3){0.0f, 0.0f, eps}), t) - d
    };
    return vec3_normalize(n);
}

// Picks a ramp character based on how directly the surface faces the
// light (simple Lambertian diffuse shading).
static char shade(Vec3 normal, Vec3 light_dir) {
    float brightness = fmaxf(0.05f, vec3_dot(light_dir, normal));
    int idx = (int)(brightness * (sizeof(ramp) - 1));
    if (idx < 0) idx = 0;
    if (idx > (int)sizeof(ramp) - 1) idx = (int)sizeof(ramp) - 1;
    return ramp[idx];
}

// Traces one ray through the scene and returns the character to draw
// for that pixel: the shaded surface character on a hit, or a blank
// space on a miss.
static char trace_pixel(scene_sdf_fn scene_sdf, float t, Camera cam, Vec3 dir, Vec3 light_dir) {
    Vec3 hit_pos;
    if (!raymarch(scene_sdf, t, cam.pos, dir, &hit_pos)) {
        return ' ';
    }
    Vec3 normal = estimate_normal(scene_sdf, hit_pos, t);
    return shade(normal, light_dir);
}

void render_scene(scene_sdf_fn scene_sdf, double start, Camera cam, char *buf, size_t buf_size) {
    int cols, rows;
    get_terminal_size(&cols, &rows);

    double t = now_seconds() - start;
    Vec3 light_dir = vec3_normalize((Vec3){-0.5f, 0.8f, -0.6f});
    float aspect = (float)cols / (float)rows * 0.5f;

    int p = 0;
    p += snprintf(buf + p, buf_size - (size_t)p, "\x1b[H"); // move cursor to top-left

    for (int y = 0; y < rows; y++) {
        float ndc_y = 1.0f - 2.0f * ((float)y + 0.5f) / (float)rows;
        for (int x = 0; x < cols; x++) {
            float ndc_x = 2.0f * ((float)x + 0.5f) / (float)cols - 1.0f;
            Vec3 dir = camera_ray_dir(cam, ndc_x, ndc_y, aspect);
            char ch = trace_pixel(scene_sdf, t, cam, dir, light_dir);
            if (p < (int)buf_size - 1) buf[p++] = ch;
        }
        if (p < (int)buf_size - 1) buf[p++] = '\n';
    }

    p += snprintf(buf + p, buf_size - (size_t)p,
                  "\x1b[0Kt=%.2f cols=%d rows=%d  wasd move / rf up-down / ijkl look / q quit\n",
                  t, cols, rows);

    write(STDOUT_FILENO, buf, (size_t)p);
    usleep(33000); // cap the frame rate at ~30fps
}
