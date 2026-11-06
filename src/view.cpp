#include "view.hpp"

namespace dice
{
Vec4 multiply(const Mat4& a, Vec4 v)
{
    return {a.m[0] * v.x + a.m[4] * v.y + a.m[8] * v.z + a.m[12] * v.w,
            a.m[1] * v.x + a.m[5] * v.y + a.m[9] * v.z + a.m[13] * v.w,
            a.m[2] * v.x + a.m[6] * v.y + a.m[10] * v.z + a.m[14] * v.w,
            a.m[3] * v.x + a.m[7] * v.y + a.m[11] * v.z + a.m[15] * v.w};
}

Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up)
{
    Vec3 f = (center - eye).unit(), s = f.cross(up).unit(), u = s.cross(f);
    Mat4 a;
    a.m = {s.x, u.x, -f.x, 0, s.y,         u.y,         -f.y,       0,
           s.z, u.z, -f.z, 0, -s.dot(eye), -u.dot(eye), f.dot(eye), 1};
    return a;
}

Mat4 perspective(double fov, double aspect, double n, double f)
{
    Mat4 a;
    a.m.fill(0);
    double t = 1 / std::tan(fov * pi / 360);
    a.m[0] = t / aspect;
    a.m[5] = t;
    a.m[10] = (f + n) / (n - f);
    a.m[11] = -1;
    a.m[14] = 2 * f * n / (n - f);
    return a;
}

Mat4 orthographic(double l, double r, double b, double t, double n, double f)
{
    Mat4 a;
    a.m[0] = 2 / (r - l);
    a.m[5] = 2 / (t - b);
    a.m[10] = -2 / (f - n);
    a.m[12] = -(r + l) / (r - l);
    a.m[13] = -(t + b) / (t - b);
    a.m[14] = -(f + n) / (f - n);
    return a;
}

Mat4 cabinet(double factor, double degrees)
{
    Mat4 a;
    a.m[8] = factor * std::cos(degrees * pi / 180);
    a.m[9] = factor * std::sin(degrees * pi / 180);
    return a;
}

ProjectionResult project(Vec3 p, const Mat4& m, const Mat4& v, const Mat4& pr, double w, double h)
{
    ProjectionResult r;
    r.world = multiply(m, {p.x, p.y, p.z, 1});
    r.eye = multiply(v, r.world);
    r.clip = multiply(pr, r.eye);
    if (std::abs(r.clip.w) < 1e-12)
    {
        return r;
    }
    r.valid = true;
    r.ndc = {r.clip.x / r.clip.w, r.clip.y / r.clip.w, r.clip.z / r.clip.w};
    r.screen = {(r.ndc.x + 1) * w / 2, (1 - r.ndc.y) * h / 2, (r.ndc.z + 1) / 2};
    r.inside =
        r.clip.w > 0 && std::abs(r.ndc.x) <= 1 && std::abs(r.ndc.y) <= 1 && std::abs(r.ndc.z) <= 1;
    return r;
}
} // namespace dice
