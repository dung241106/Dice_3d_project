#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

namespace dice {
constexpr double pi = 3.14159265358979323846;
constexpr double half = 0.62, bevel = 0.12;
struct Vec3 {
    double x=0,y=0,z=0;
    Vec3 operator+(Vec3 b) const { return {x+b.x,y+b.y,z+b.z}; }
    Vec3 operator-(Vec3 b) const { return {x-b.x,y-b.y,z-b.z}; }
    Vec3 operator*(double s) const { return {x*s,y*s,z*s}; }
    double dot(Vec3 b) const { return x*b.x+y*b.y+z*b.z; }
    Vec3 cross(Vec3 b) const { return {y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x}; }
    double length() const { return std::sqrt(dot(*this)); }
    Vec3 unit() const { double n=length(); return n>1e-12 ? *this*(1/n) : Vec3{1,0,0}; }
};
struct Quat {
    double w=1,x=0,y=0,z=0;
    Quat operator*(Quat b) const;
    Quat unit() const;
    Vec3 rotate(Vec3 v) const;
    std::array<double,16> matrix() const; // Column-major, column vectors, OpenGL convention.
    static Quat axisAngle(Vec3 axis,double angle);
    static Quat fromTo(Vec3 a,Vec3 b);
    static Quat slerp(Quat a,Quat b,double t);
};
struct Face { int value; Vec3 normal,u,v; };
const std::array<Face,6>& faces();
std::vector<Vec3> pipCenters(int value); // z=0, local face coordinates.
struct Vertex { Vec3 position,normal; };
struct Mesh { std::vector<Vertex> vertices; std::vector<unsigned> indices; };
Mesh roundedCube(int subdivisions=20);
int topFace(Quat q);
double supportHeight(Quat q); // Exact rounded-box support, not its enclosing sphere.
Quat faceUp(int value,double yaw=0);
struct Die {
    Vec3 position;
    Quat rotation;
    int value=1;
};
Mesh basicCube();
int runTests();
} // namespace dice
