#include "dice.hpp"

namespace dice {
Quat Quat::operator*(Quat b) const {
    return {w*b.w-x*b.x-y*b.y-z*b.z,w*b.x+x*b.w+y*b.z-z*b.y,
            w*b.y-x*b.z+y*b.w+z*b.x,w*b.z+x*b.y-y*b.x+z*b.w};
}
Quat Quat::unit() const {
    double n=std::sqrt(w*w+x*x+y*y+z*z);
    return n>1e-12 ? Quat{w/n,x/n,y/n,z/n} : Quat{};
}
Quat Quat::axisAngle(Vec3 axis,double a) {
    axis=axis.unit(); double s=std::sin(a/2);
    return {std::cos(a/2),axis.x*s,axis.y*s,axis.z*s};
}
Vec3 Quat::rotate(Vec3 v) const {
    Quat q=unit(),p=q*Quat{0,v.x,v.y,v.z}*Quat{q.w,-q.x,-q.y,-q.z};
    return {p.x,p.y,p.z};
}
std::array<double,16> Quat::matrix() const {
    auto q=unit(); double a=q.w,b=q.x,c=q.y,d=q.z;
    return {1-2*(c*c+d*d),2*(b*c+a*d),2*(b*d-a*c),0,
            2*(b*c-a*d),1-2*(b*b+d*d),2*(c*d+a*b),0,
            2*(b*d+a*c),2*(c*d-a*b),1-2*(b*b+c*c),0,0,0,0,1};
}
Quat Quat::fromTo(Vec3 a,Vec3 b) {
    a=a.unit(); b=b.unit(); double d=std::clamp(a.dot(b),-1.0,1.0);
    if(d>0.999999) return {};
    if(d< -0.999999) return axisAngle(a.cross(std::abs(a.x)<0.8 ? Vec3{1,0,0}:Vec3{0,1,0}),pi);
    Vec3 c=a.cross(b); return Quat{1+d,c.x,c.y,c.z}.unit();
}
Quat Quat::slerp(Quat a,Quat b,double t) {
    a=a.unit(); b=b.unit(); double d=a.w*b.w+a.x*b.x+a.y*b.y+a.z*b.z;
    if(d<0) { b={-b.w,-b.x,-b.y,-b.z}; d=-d; }
    double s0=1-t,s1=t;
    if(d<0.9995) {
        double angle=std::acos(std::clamp(d,-1.0,1.0)),s=std::sin(angle);
        s0=std::sin((1-t)*angle)/s; s1=std::sin(t*angle)/s;
    }
    return Quat{a.w*s0+b.w*s1,a.x*s0+b.x*s1,a.y*s0+b.y*s1,a.z*s0+b.z*s1}.unit();
}
const std::array<Face,6>& faces() {
    // u cross v = outward normal. Opposite faces always sum to 7.
    static const std::array<Face,6> f{{
        {1,{0,1,0},{1,0,0},{0,0,-1}}, {6,{0,-1,0},{1,0,0},{0,0,1}},
        {2,{0,0,1},{1,0,0},{0,1,0}}, {5,{0,0,-1},{-1,0,0},{0,1,0}},
        {3,{1,0,0},{0,0,-1},{0,1,0}}, {4,{-1,0,0},{0,0,1},{0,1,0}}
    }};
    return f;
}
std::vector<Vec3> pipCenters(int n) {
    constexpr double d=0.29;
    std::vector<Vec3> p;
    if(n==1||n==3||n==5) p.push_back({0,0,0});
    if(n>=2) { p.push_back({-d,-d,0}); p.push_back({d,d,0}); }
    if(n>=4) { p.push_back({-d,d,0}); p.push_back({d,-d,0}); }
    if(n==6) { p.push_back({-d,0,0}); p.push_back({d,0,0}); }
    return p;
}
Mesh roundedCube(int n) {
    n=std::max(n,2); Mesh mesh;
    for(const auto& f:faces()) {
        unsigned base=static_cast<unsigned>(mesh.vertices.size());
        for(int j=0;j<=n;j++) for(int i=0;i<=n;i++) {
            Vec3 p=f.normal*half+f.u*((2.0*i/n-1)*half)+f.v*((2.0*j/n-1)*half);
            double k=half-bevel;
            Vec3 c{std::clamp(p.x,-k,k),std::clamp(p.y,-k,k),std::clamp(p.z,-k,k)};
            Vec3 normal=(p-c).unit();
            mesh.vertices.push_back({c+normal*bevel,normal});
        }
        for(int j=0;j<n;j++) for(int i=0;i<n;i++) {
            unsigned a=base+static_cast<unsigned>(j*(n+1)+i),b=a+1,c=a+static_cast<unsigned>(n+1),d=c+1;
            for(unsigned v:{a,b,d,a,d,c}) mesh.indices.push_back(v);
        }
    }
    return mesh;
}
int topFace(Quat q) {
    double best=-2; int value=1;
    for(auto f:faces()) { double y=q.rotate(f.normal).y; if(y>best) {best=y;value=f.value;} }
    return value;
}
Quat faceUp(int value,double yaw) {
    for(auto f:faces()) if(f.value==value)
        return Quat::axisAngle({0,1,0},yaw)*Quat::fromTo(f.normal,{0,1,0});
    return {};
}
double supportHeight(Quat q) {
    // Rounded cube = inner box Minkowski-summed with a sphere of radius bevel.
    Vec3 up=Quat{q.w,-q.x,-q.y,-q.z}.unit().rotate({0,1,0});
    return (half-bevel)*(std::abs(up.x)+std::abs(up.y)+std::abs(up.z))+bevel;
}
Mesh basicCube() {
    Mesh mesh;
    for (auto f:faces()) {
        unsigned base=static_cast<unsigned>(mesh.vertices.size());
        for (auto p:std::array<Vec3,4>{{{-half,-half,0},{half,-half,0},{half,half,0},{-half,half,0}}})
            mesh.vertices.push_back({f.normal*half+f.u*p.x+f.v*p.y,f.normal});
        for (unsigned i:{0u,1u,2u,0u,2u,3u}) mesh.indices.push_back(base+i);
    }
    return mesh;
}
} // namespace dice
