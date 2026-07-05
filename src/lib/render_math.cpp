// render_math.cpp — self-contained, no separate _cpp.hpp needed
#include "render_math.hpp"
#include "linear.hpp"
#include <cmath>
#include <cstring>

// ---- C++ implementation (was render_math_cpp.hpp) ----
static la::Mat4 cpp_look_at(la::Vec3 eye, la::Vec3 center, la::Vec3 up) {
    la::Vec3 f = la::normalize(center - eye);
    la::Vec3 r = la::normalize(la::cross(f, up));
    la::Vec3 u = la::cross(r, f);
    la::Mat4 result{};
    result.m[0][0] =  r.x; result.m[0][1] =  r.y; result.m[0][2] =  r.z; result.m[0][3] = -la::dot(r, eye);
    result.m[1][0] =  u.x; result.m[1][1] =  u.y; result.m[1][2] =  u.z; result.m[1][3] = -la::dot(u, eye);
    result.m[2][0] = -f.x; result.m[2][1] = -f.y; result.m[2][2] = -f.z; result.m[2][3] =  la::dot(f, eye);
    result.m[3][0] =  0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;
    return result;
}

static la::Mat4 cpp_perspective(float fovy, float aspect, float znear, float zfar) {
    float t = tanf(fovy * 0.5f);
    la::Mat4 m = la::Mat4::zero();
    m.m[0][0] =  1.0f / (aspect * t);
    m.m[1][1] =  1.0f / t;
    m.m[2][2] = -(zfar + znear) / (zfar - znear);
    m.m[2][3] = -(2.0f * zfar * znear) / (zfar - znear);
    m.m[3][2] = -1.0f;
    return m;
}

static la::Mat4 cpp_orthographic(float l, float r, float b, float t, float zn, float zf) {
    la::Mat4 m = la::Mat4::zero();
    m.m[0][0] =  2.0f / (r - l);
    m.m[1][1] =  2.0f / (t - b);
    m.m[2][2] = -2.0f / (zf - zn);
    m.m[0][3] = -(r + l) / (r - l);
    m.m[1][3] = -(t + b) / (t - b);
    m.m[2][3] = -(zf + zn) / (zf - zn);
    m.m[3][3] =  1.0f;
    return m;
}

static la::Vec3 cpp_transform_point(const la::Mat4& m, const la::Vec3& v) {
    la::Vec4 r = m * la::Vec4(v, 1.0f);
    return la::Vec3::from_homogeneous(r);
}

static la::Vec3 cpp_transform_dir(const la::Mat4& m, const la::Vec3& v) {
    return la::Vec3(m * la::Vec4(v, 0.0f));
}

static la::Mat4 cpp_translate(la::Vec3 t) {
    la::Mat4 m = la::Mat4::identity();
    m.m[0][3] = t.x; m.m[1][3] = t.y; m.m[2][3] = t.z;
    return m;
}

static la::Mat4 cpp_scale(la::Vec3 s) {
    la::Mat4 m = la::Mat4::identity();
    m.m[0][0] = s.x; m.m[1][1] = s.y; m.m[2][2] = s.z;
    return m;
}

static la::Mat4 cpp_rotate_x(float a) {
    la::Mat4 m = la::Mat4::identity();
    float c = cosf(a), s = sinf(a);
    m.m[1][1] =  c; m.m[1][2] = -s;
    m.m[2][1] =  s; m.m[2][2] =  c;
    return m;
}

static la::Mat4 cpp_rotate_y(float a) {
    la::Mat4 m = la::Mat4::identity();
    float c = cosf(a), s = sinf(a);
    m.m[0][0] =  c; m.m[0][2] =  s;
    m.m[2][0] = -s; m.m[2][2] =  c;
    return m;
}

static la::Mat4 cpp_rotate_z(float a) {
    la::Mat4 m = la::Mat4::identity();
    float c = cosf(a), s = sinf(a);
    m.m[0][0] =  c; m.m[0][1] = -s;
    m.m[1][0] =  s; m.m[1][1] =  c;
    return m;
}

// ---- C bridge (was the extern "C" wrappers) ----
static inline const la::Vec3& as_cpp(const Vec3& v) { return reinterpret_cast<const la::Vec3&>(v); }
static inline const la::Mat4& as_cpp(const Mat4& m) { return reinterpret_cast<const la::Mat4&>(m); }
static inline Vec3 as_c(const la::Vec3& v) { return {v.x, v.y, v.z}; }
static inline Mat4 as_c(const la::Mat4& m) { Mat4 out; memcpy(out.m, m.m, sizeof(out.m)); return out; }

extern "C" {

Vec3 render_transform_point(Mat4 m, Vec3 v) { return as_c(cpp_transform_point(as_cpp(m), as_cpp(v))); }
Vec3 render_transform_dir(Mat4 m, Vec3 v)   { return as_c(cpp_transform_dir(as_cpp(m), as_cpp(v))); }
Mat4 render_look_at(Vec3 eye, Vec3 center, Vec3 up) { return as_c(cpp_look_at(as_cpp(eye), as_cpp(center), as_cpp(up))); }
Mat4 render_perspective(float fovy, float aspect, float zn, float zf) { return as_c(cpp_perspective(fovy, aspect, zn, zf)); }
Mat4 render_orthographic(float l, float r, float b, float t, float zn, float zf) { return as_c(cpp_orthographic(l, r, b, t, zn, zf)); }
Mat4 render_translate(Vec3 t)  { return as_c(cpp_translate(as_cpp(t))); }
Mat4 render_scale(Vec3 s)      { return as_c(cpp_scale(as_cpp(s))); }
Mat4 render_rotate_x(float a)  { return as_c(cpp_rotate_x(a)); }
Mat4 render_rotate_y(float a)  { return as_c(cpp_rotate_y(a)); }
Mat4 render_rotate_z(float a)  { return as_c(cpp_rotate_z(a)); }

} // extern "C"