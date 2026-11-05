#include "lab.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <iostream>

namespace dice {
Mat4 Mat4::operator*(const Mat4& b) const {
    Mat4 r;r.m.fill(0);
    for(int c=0;c<4;c++)for(int row=0;row<4;row++)for(int k=0;k<4;k++)r.m[c*4+row]+=m[k*4+row]*b.m[c*4+k];
    return r;
}
Vec3 Mat4::point(Vec3 p) const {
    return {m[0]*p.x+m[4]*p.y+m[8]*p.z+m[12],m[1]*p.x+m[5]*p.y+m[9]*p.z+m[13],m[2]*p.x+m[6]*p.y+m[10]*p.z+m[14]};
}
Vec3 Mat4::normal(Vec3 n) const {
    // Columns of inverse-transpose(A) are cross(b,c), cross(c,a), cross(a,b) / det(A).
    Vec3 a{m[0],m[1],m[2]},b{m[4],m[5],m[6]},c{m[8],m[9],m[10]};
    double det=a.dot(b.cross(c));
    if(std::abs(det)<1e-12)return {};
    return ((b.cross(c)*n.x+c.cross(a)*n.y+a.cross(b)*n.z)*(1/det)).unit();
}
Mat4 Mat4::translation(Vec3 v) {Mat4 a;a.m[12]=v.x;a.m[13]=v.y;a.m[14]=v.z;return a;}
Mat4 Mat4::scale(Vec3 v) {Mat4 a;a.m[0]=v.x;a.m[5]=v.y;a.m[10]=v.z;return a;}
Mat4 Mat4::rotation(Vec3 axis,double angle) {Mat4 a;a.m=Quat::axisAngle(axis,angle).matrix();return a;}
Mat4 Mat4::shear(int axis,double v) {
    Mat4 a;
    if(axis==0||axis==3)a.m[4]=v;
    if(axis==1||axis==3)a.m[9]=v;
    if(axis==2||axis==3)a.m[2]=v;
    return a;
}
bool Mat4::inverse(Mat4& result) const {
    double a[4][8]{};
    for(int r=0;r<4;r++)for(int c=0;c<4;c++){a[r][c]=m[c*4+r];a[r][c+4]=r==c?1:0;}
    for(int c=0;c<4;c++){
        int p=c;for(int r=c+1;r<4;r++)if(std::abs(a[r][c])>std::abs(a[p][c]))p=r;
        if(std::abs(a[p][c])<1e-12)return false;
        for(int j=0;j<8;j++)std::swap(a[p][j],a[c][j]);
        double d=a[c][c];for(int j=0;j<8;j++)a[c][j]/=d;
        for(int r=0;r<4;r++)if(r!=c){double t=a[r][c];for(int j=0;j<8;j++)a[r][j]-=t*a[c][j];}
    }
    for(int r=0;r<4;r++)for(int c=0;c<4;c++)result.m[c*4+r]=a[r][c+4];
    return true;
}
Mat4 TransformStep::matrix(double t) const {
    Vec3 a=axis==0?Vec3{1,0,0}:axis==1?Vec3{0,1,0}:axis==2?Vec3{0,0,1}:Vec3{1,1,1};
    if(axis==3&&kind==Transform::Rotate)a=arbitraryAxis;
    Mat4 op;
    switch(kind) {
        case Transform::Translate:return Mat4::translation(a*(amount*t));
        case Transform::Rotate:op=Mat4::rotation(a,amount*t*pi/180);break;
        case Transform::Scale:op=Mat4::scale(Vec3{1,1,1}+a*((amount-1)*t));break;
        // Reflections are discrete: avoid a singular inverse-normal matrix at scale=0.
        case Transform::Mirror:op=Mat4::scale(Vec3{1,1,1}+a*(t<1?0:-2));break;
        case Transform::Shear:op=Mat4::shear(axis,amount*t);break;
    }
    return Mat4::translation(pivot)*op*Mat4::translation(pivot*(-1));
}
std::string TransformStep::label() const {
    static const char* names[]={"Dich","Quay","Ti le","Doi xung","Xien"};
    static const char* axes[]={"X","Y","Z","XYZ"};
    std::ostringstream s;s<<names[static_cast<int>(kind)]<<' '<<axes[axis];
    if(kind!=Transform::Mirror)s<<' '<<std::fixed<<std::setprecision(kind==Transform::Rotate?0:2)<<amount;
    if(pivot.length()>0)s<<" @P";
    return s.str();
}
Mat4 Lab::at(std::size_t count) const {
    Mat4 m=initial;
    for(std::size_t i=0;i<std::min(count,steps.size());i++){
        const auto& step=steps[i];
        m=step.matrix()*m;
    }
    return m;
}
Mat4 Lab::model() const {
    Mat4 m=at(displayedCount());
    if(const auto* step=activeStep())m=step->matrix(phase)*m;
    return m;
}
const TransformStep* Lab::activeStep() const {
    return replaying&&replayCursor<cursor?&steps[replayCursor]:nullptr;
}
void Lab::stopPlayback(){playing=false;replaying=false;phase=0;}
void Lab::apply(int direction) {
    if(playing||cursor>=24)return; // Bound scale growth and keep the lesson readable.
    stopPlayback();
    steps.resize(cursor);
    double amount=direction*translationStep;
    if(tool==Transform::Rotate)amount=direction*rotationStep;
    if(tool==Transform::Scale)amount=direction>0?scaleStep:1/scaleStep;
    if(tool==Transform::Shear)amount=direction*shearStep;
    steps.push_back({tool,axis,amount,pivotEnabled&&tool!=Transform::Translate?pivot:Vec3{},arbitraryAxis});cursor=steps.size();
}
void Lab::undo(){stopPlayback();
    if(cursor>0)--cursor;}
void Lab::redo(){stopPlayback();
    if(cursor<steps.size())++cursor;}
void Lab::reset(){steps.clear();cursor=0;initial={};stopPlayback();}
void Lab::rewind(){stopPlayback();if(cursor>0){replaying=true;replayCursor=0;}}
void Lab::preview(TransformStep step,Mat4 start){reset();initial=start;steps.push_back(step);cursor=1;rewind();}
void Lab::play(){
    if(cursor==0)return;
    if(!replaying){replayCursor=0;phase=0;replaying=true;}
    playing=!playing;
}
void Lab::nextPlaybackStep(){
    if(playing||cursor==0)return;
    if(!replaying){replaying=true;replayCursor=0;}
    ++replayCursor;
    phase=0;
    if(replayCursor>=cursor)stopPlayback();
}
void Lab::advance(double fraction){
    if(playing||cursor==0||fraction<=0)return;
    if(!replaying)rewind();
    phase+=fraction;
    while(phase>=1-1e-10&&replayCursor<cursor){phase=std::max(0.0,phase-1);++replayCursor;}
    if(replayCursor>=cursor)stopPlayback();
}
double Lab::progress() const {
    if(cursor==0)return 0;
    return replaying?std::clamp((replayCursor+phase)/cursor,0.0,1.0):1.0;
}
void Lab::tick(double dt) {
    if(!playing||dt<=0)return;
    phase+=dt/1.6;
    while(phase>=1&&replayCursor<cursor){phase-=1;++replayCursor;}
    if(replayCursor>=cursor)stopPlayback();
}
int testLab() {
    int n=0;auto check=[&](bool ok){++n;
    if(!ok)throw std::runtime_error("Transform lab invariant");};
    auto near=[](Vec3 a,Vec3 b){return(a-b).length()<1e-8;};
    check(near(Mat4::translation({2,3,4}).point({1,2,3}),{3,5,7}));
    check(near(Mat4::scale({2,3,-1}).point({1,2,3}),{2,6,-3}));
    check(near(Mat4::rotation({0,0,1},pi/2).point({1,0,0}),{0,1,0}));
    check(near(Mat4::shear(0,0.5).point({1,2,3}),{2,2,3}));
    Mat4 affine=Mat4::shear(0,0.5)*Mat4::scale({2,3,-1});
    Vec3 tangent=affine.point({0,1,0})-affine.point({});
    check(std::abs(affine.normal({1,0,0}).dot(tangent))<1e-9);
    check(std::abs(affine.normal({1,0,0}).length()-1)<1e-9);
    TransformStep pivot{Transform::Rotate,2,90,{1,0,0}};
    check(near(pivot.matrix().point({2,0,0}),{1,1,0}));
    check(near(pivot.matrix().point({1,0,0}),{1,0,0}));
    TransformStep mirror{Transform::Mirror,0,0,{}};
    check(near(mirror.matrix().point({1,2,3}),{-1,2,3}));
    check(near((mirror.matrix()*mirror.matrix()).point({1,2,3}),{1,2,3}));
    Lab lab;lab.apply(1);lab.tool=Transform::Rotate;lab.axis=2;lab.apply(1);
    check(near(lab.model().point({}),{0,2,0}));
    lab.undo();check(lab.cursor==1&&near(lab.model().point({}),{2,0,0}));lab.redo();
    check(lab.cursor==2&&near(lab.model().point({}),{0,2,0}));
    lab.rewind();check(lab.replaying&&!lab.playing&&near(lab.model().point({}),{}));
    lab.play();lab.tick(.8);check(lab.displayedCount()==0&&lab.phase==.5&&lab.playing);
    Vec3 partial=lab.model().point({});lab.play();lab.tick(1);
    check(!lab.playing&&near(partial,lab.model().point({})));
    lab.advance(.5);check(lab.displayedCount()==1&&near(lab.model().point({}),{2,0,0}));
    lab.nextPlaybackStep();check(!lab.replaying&&near(lab.model().point({}),{0,2,0}));
    const double amounts[]={2,90,1.5,-1,.75};
    for(int kind=0;kind<5;kind++)for(int axis=0;axis<3;axis++){
        TransformStep step{static_cast<Transform>(kind),axis,amounts[kind],{}};
        Mat4 start=kind==3?Mat4::translation(axis==0?Vec3{1.8,0,0}:axis==1?Vec3{0,1.8,0}:Vec3{0,0,1.8}):Mat4{};
        Vec3 p{.4,.3,.2};
        lab.preview(step,start);
        check(lab.steps.size()==1&&lab.replaying&&!lab.playing&&lab.progress()==0);
        check(near(lab.model().point(p),start.point(p)));
        lab.play();lab.tick(.8);
        check(near(lab.model().point(p),(step.matrix(.5)*start).point(p)));
        check(std::abs(lab.progress()-.5)<1e-9);
        lab.play();Vec3 frozen=lab.model().point(p);lab.tick(3);check(near(lab.model().point(p),frozen));
        lab.play();lab.tick(.8);check(!lab.playing&&!lab.replaying&&lab.progress()==1);
        check(near(lab.model().point(p),(step.matrix()*start).point(p)));
        lab.rewind();for(int quarter=1;quarter<=4;quarter++){
            lab.advance();check(std::abs(lab.progress()-quarter*.25)<1e-9);
            check(near(lab.model().point(p),(step.matrix(quarter*.25)*start).point(p)));
        }
    }
    lab.preview({Transform::Translate,0,2,{}});lab.play();lab.tick(.5);
    lab.preview({Transform::Rotate,2,90,{}});
    check(lab.steps.size()==1&&lab.steps[0].kind==Transform::Rotate&&near(lab.model().point({}),{}));
    lab.play();lab.tick(1.6);check(near(lab.model().point({1,0,0}),{0,1,0}));
    lab.reset();check(lab.steps.empty()&&near(lab.model().point({1,2,3}),{1,2,3}));
    std::cout<<"PASS: "<<n<<" transform checks\n";return 0;
}
} // namespace dice
