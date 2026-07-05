#include "mathlib.hpp"

typedef float (*scene_sdf_fn)(Vec3 p, float t);
float scene_sdf(Vec3 p, float t);