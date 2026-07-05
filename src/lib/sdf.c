#include <math.h>

#include "sdf.h"

float sdf_sphere(Vec3 p, float r) {
    return vec3_length(p) - r;
}

float sdf_box(Vec3 p, Vec3 b) {
    Vec3 d = vec3_sub(vec3_abs(p), b);
    return fminf(fmaxf(d.x, fmaxf(d.y, d.z)), 0.0f) + vec3_length(vec3_max(d, (Vec3){0.0f, 0.0f, 0.0f}));
}

float sdf_torus(Vec3 p, Vec2 t) {
    Vec2 q = (Vec2){vec2_length((Vec2){p.x, p.z}) - t.x, p.y};
    return vec2_length(q) - t.y;
}

float sdf_death_star(Vec3 p2, float ra, float rb, float d) {
    float a = (ra*ra - rb*rb + d*d) / (2.0f * d);
    float b = sqrtf(fmaxf(ra*ra-a*a, 0.0f));

    Vec2 p = (Vec2){p2.x, vec2_length((Vec2){p2.y, p2.z})};
    if(p.x*b-p.y*a > d*fmaxf(b-p.y, 0.0f)) {
        return vec2_length((Vec2){p.x - a, p.y - b});
    }
    return fmaxf(vec2_length(p)-ra,-(vec2_length((Vec2){p.x - d, p.y - 0}))-rb);
}
