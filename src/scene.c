#include <math.h>

#include "scene.h"
#include "sdf.h"
#include "render_math.hpp"

float scene_sdf(Vec3 p, float t) {
    Vec3 pos = (Vec3){0.0f, 0.0f, 0.0f};
    Vec3 scale = (Vec3){1.0f, 1.0f, 1.0f};

    Vec3 local = vec3_sub(p, pos);
    local = render_transform_point(render_rotate_x(-t), local);
    local = render_transform_point(render_rotate_z(-t), local);
    //local = render_transform_point(render_rotate_y(-t), local);

    return sdf_torus(local, (Vec2){scale.x, scale.x/3});
    //return sdf_death_star(local, 6.0f, 2.0f, 4.0f);
    //return sdf_box(local, (Vec3){1.0f, 1.0f, 1.0f});
}