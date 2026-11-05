#pragma once
#include "lab.hpp"

namespace dice {
struct Vec4 { double x=0,y=0,z=0,w=1; };
Vec4 multiply(const Mat4& matrix,Vec4 v);
Mat4 lookAt(Vec3 eye,Vec3 center,Vec3 up);
Mat4 perspective(double fovDegrees,double aspect,double nearPlane,double farPlane);
Mat4 orthographic(double left,double right,double bottom,double top,double nearPlane,double farPlane);
Mat4 cabinet(double factor=0.5,double angleDegrees=45);
struct ProjectionResult {
    Vec4 world,eye,clip;
    Vec3 ndc,screen;
    bool valid=false,inside=false;
};
ProjectionResult project(Vec3 p,const Mat4& model,const Mat4& view,const Mat4& projection,double width,double height);
} // namespace dice
