#pragma once
#include "mathlib.hpp"   // gets Vec3, Vec4, Mat4 C structs

#ifdef __cplusplus
extern "C" {
#endif

Vec3 render_transform_point(Mat4 m, Vec3 v);
Vec3 render_transform_dir(Mat4 m, Vec3 v);
Mat4 render_look_at(Vec3 eye, Vec3 center, Vec3 up);
Mat4 render_perspective(float fovy, float aspect, float znear, float zfar);
Mat4 render_orthographic(float left, float right, float bottom,
                          float top, float znear, float zfar);
Mat4 render_translate(Vec3 t);
Mat4 render_scale(Vec3 s);
Mat4 render_rotate_x(float angle);
Mat4 render_rotate_y(float angle);
Mat4 render_rotate_z(float angle);

#ifdef __cplusplus
}
#endif