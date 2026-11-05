#define NOMINMAX
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/freeglut.h>
#include "dice.hpp"
#include "lab.hpp"
#include "puzzle.hpp"
#include "throw_sim.hpp"
#include "sound.hpp"
#include "view.hpp"
#include "ui.hpp"
#include "storage.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

using namespace dice;
using ui::Color;
namespace {
constexpr int virtualW=1440,virtualH=900;
constexpr double sceneX=28,sceneY=174,sceneW=990;
double sceneH=548;
enum class Page {Simulation,Model,Transform,Projection,Puzzle};
enum Action {
    TabSim=10,TabModel,TabTransform,TabProjection,TabPuzzle,Help,
    Count1=20,Count2,Count3,Roll,Pause,Single,ResetCamera,Slow,SoundToggle,
    Wire=40,Lighting,Normals,Shadows,Axes,Material,
    Face1=60,Face2,Face3,Face4,Face5,Face6,ExplodeLess,ExplodeMore,MeshToggle,AutoRotate,
    ToolT=80,ToolR,ToolS,ToolMirror,ToolShear,AxisX,AxisY,AxisZ,AxisK,
    ApplyMinus,ApplyPlus,Undo,Redo,ClearLab,PlayLab,NextLab,DirectionToggle,PivotToggle,Composite,
    PresetTR=110,PresetRT,PresetPivot,PresetInverse,MatrixToggle,
    Perspective=130,Ortho,Oblique,Front,Top,Side,VertexNext,FovLess,FovMore,
    Die1=150,Die2,Die3,TurnXp,TurnXm,TurnYp,TurnYm,TurnZp,TurnZm,Hint,NewPuzzle,ExportData
};
Page page=Page::Simulation;
Lab lab;
std::unique_ptr<ThrowSimulation> simulation;
std::unique_ptr<FacePuzzle> puzzle;
SoundBank soundBank;
Mesh rounded,basic;
GLuint roundedList=0,basicList=0,woodTexture=0;
bool wire=false,lighting=true,normals=false,shadows=true,axes=false,slow=false,help=false;
bool compositeLesson=false;
int lessonDirection=1;
bool charging=false,chargeMouse=false,dragging=false,aiming=false,autoRotate=false,roundedMode=true;
bool matrixInverse=false,fullscreen=false,writable=true,smokeMode=false,demoMode=false;
int width=1440,height=900,vw=1440,vh=900,offsetX=0,offsetY=0,lastX=0,lastY=0;
int selectedFace=1,material=0,projectionMode=0,selectedVertex=6;
double yaw=24,pitch=34,cameraDistance=13.5,fov=42,explode=0.0,modelAngle=0;
double chargeTime=0,accumulator=0,soundCooldown=0,aimStartX=0,aimStartY=0,aimEndX=0,aimEndY=0;
std::string notice="Giữ Space để lấy lực, thả để đổ";
std::filesystem::path dataDirectory;
std::chrono::steady_clock::time_point lastTime;
Mat4 sceneView,sceneProjection,sceneModel;
const std::array<Color,6> faceColors{{{0.95,0.84,0.59},{0.43,0.79,0.82},{0.59,0.68,0.95},{0.90,0.57,0.67},{0.69,0.61,0.91},{0.57,0.84,0.68}}};
const std::array<Vec3,8> cubeVertices{{{-half,-half,-half},{half,-half,-half},{half,half,-half},{-half,half,-half},
    {-half,-half,half},{half,-half,half},{half,half,half},{-half,half,half}}};

std::string fixed(double n,int digits=2){if(std::abs(n)<0.000001)n=0;std::ostringstream s;s<<std::fixed<<std::setprecision(digits)<<n;return s.str();}
std::string vec(Vec3 v,int digits=2){return "("+fixed(v.x,digits)+", "+fixed(v.y,digits)+", "+fixed(v.z,digits)+")";}
std::string vec4(Vec4 v){return "("+fixed(v.x)+", "+fixed(v.y)+", "+fixed(v.z)+", "+fixed(v.w)+")";}
void vertex(Vec3 v){glVertex3d(v.x,v.y,v.z);}
void normal(Vec3 v){glNormal3d(v.x,v.y,v.z);}
void segment(Vec3 a,Vec3 b){glBegin(GL_LINES);vertex(a);vertex(b);glEnd();}
void cube(Vec3 p,Vec3 scale,Color c){glPushMatrix();glTranslated(p.x,p.y,p.z);glScaled(scale.x,scale.y,scale.z);ui::color(c);glutSolidCube(1);glPopMatrix();}
void label3D(Vec3 p,const std::string& s,Color c=ui::white){ui::color(c);glRasterPos3d(p.x,p.y,p.z);for(char ch:s)glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,ch);}
void basis(double length=2){
    glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);glLineWidth(2);
    std::array<Vec3,3> ends{{{length,0,0},{0,length,0},{0,0,length}}};
    std::array<Color,3> colors{{{1,.36,.38},{.42,.94,.57},{.37,.63,1}}};
    for(int i=0;i<3;i++){ui::color(colors[i]);segment({},ends[i]);label3D(ends[i]*1.1,std::string(1,"XYZ"[i]),colors[i]);}
    glPopAttrib();
}
void grid(double y,double extent=5){
    glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);ui::color({.14,.23,.31});
    for(int i=-int(extent);i<=int(extent);i++){segment({double(i),y,-extent},{double(i),y,extent});segment({-extent,y,double(i)},{extent,y,double(i)});}glPopAttrib();
}
void pips(const Face& f,Vec3 offset={},bool selected=false){
    ui::color(selected?Color{.07,.12,.17}:Color{.035,.045,.06});normal(f.normal);
    for(auto p:pipCenters(f.value)){
        Vec3 center=offset+f.normal*(half+.003)+f.u*p.x+f.v*p.y;
        glBegin(GL_TRIANGLE_FAN);vertex(center);
        for(int k=0;k<=32;k++){double a=2*pi*k/32;vertex(center+(f.u*std::cos(a)+f.v*std::sin(a))*.088);}glEnd();
    }
}
void body(bool roundedBody=true,Color c={.95,.92,.82}){
    glPushAttrib(GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_POLYGON_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);
    if(!lighting||wire)glDisable(GL_LIGHTING);
    GLfloat s=material==2?.93f:material==1?.12f:.55f;
    GLfloat spec[]={s,s,s,1};glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,spec);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,material==2?100:material==1?14:55);
    bool wood=material==1&&!wire;
    if(wood){
        const GLfloat u[]={.8f,.1f,.6f,0},v[]={0,.9f,.1f,0};
        glBindTexture(GL_TEXTURE_2D,woodTexture);glEnable(GL_TEXTURE_2D);
        glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);glTexGenfv(GL_S,GL_OBJECT_PLANE,u);
        glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);glTexGenfv(GL_T,GL_OBJECT_PLANE,v);
        glEnable(GL_TEXTURE_GEN_S);glEnable(GL_TEXTURE_GEN_T);c={.68,.40,.18};
    }
    ui::color(wire?ui::accent:c);if(wire)glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glCallList(roundedBody?roundedList:basicList);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDisable(GL_TEXTURE_2D);glDisable(GL_TEXTURE_GEN_S);glDisable(GL_TEXTURE_GEN_T);
    for(auto f:faces())pips(f);
    glPopAttrib();
}
void drawNormals(const Mat4& m){
    glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);glLineWidth(2);ui::color(ui::gold);
    for(auto f:faces()){Vec3 p=m.point(f.normal*(half+.01));segment(p,p+m.normal(f.normal)*.65);}glPopAttrib();
}
Mat4 dieMatrix(const Die& d){Mat4 r;r.m=d.rotation.matrix();return Mat4::translation(d.position)*r;}
void drawDie(const Die& d,int index=-1){
    auto m=dieMatrix(d);glPushMatrix();glMultMatrixd(m.m.data());body();glPopMatrix();
    if(normals)drawNormals(m);
    if(index>=0){glPushAttrib(GL_ENABLE_BIT);glDisable(GL_LIGHTING);label3D(d.position+Vec3{-.15,1.1,0},"#"+std::to_string(index+1));glPopAttrib();}
}
void topOutline(const Die& d){
    glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);glLineWidth(3);ui::color(ui::gold);
    for(auto f:faces())if(f.value==topFace(d.rotation)){
        glBegin(GL_LINE_LOOP);
        for(auto p:std::array<Vec3,4>{{{-.48,-.48,0},{.48,-.48,0},{.48,.48,0},{-.48,.48,0}}}){
            vertex(d.position+d.rotation.rotate(f.normal*(half+.009)+f.u*p.x+f.v*p.y));
        }
        glEnd();
    }glPopAttrib();
}
void contactShadow(const Die& d){
    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);
    glColor4d(0,0,0,.26/(1+.2*d.position.y));
    double radius=.65+.09*d.position.y;glBegin(GL_TRIANGLE_FAN);vertex({d.position.x,.011,d.position.z});
    for(int i=0;i<=48;i++){double a=2*pi*i/48;vertex({d.position.x+radius*std::cos(a),.011,d.position.z+radius*std::sin(a)});}glEnd();glPopAttrib();
}
void tray(){
    cube({0,-.22,0},{10.55,.4,6.55},{.13,.19,.27});cube({0,-.045,0},{10,.09,6},{.08,.21,.24});
    cube({0,.18,-3.16},{10.55,.38,.2},{.24,.32,.40});cube({-5.16,.18,0},{.2,.38,6.5},{.24,.32,.40});cube({5.16,.18,0},{.2,.38,6.5},{.24,.32,.40});
    grid(.004,3);
}
void projectedShadows(){
    glPushAttrib(GL_ENABLE_BIT|GL_STENCIL_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT|GL_CURRENT_BIT);
    glEnable(GL_STENCIL_TEST);glStencilMask(0xFF);glStencilFunc(GL_ALWAYS,1,0xFF);glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);glDepthMask(GL_FALSE);
    glBegin(GL_QUADS);vertex({-5,.005,-3});vertex({-5,.005,3});vertex({5,.005,3});vertex({5,.005,-3});glEnd();
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);glStencilFunc(GL_EQUAL,1,0xFF);glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
    glDisable(GL_LIGHTING);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glColor4d(0,0,0,.25);
    // S = (plane dot light) I - light * plane^T.
    double light[]={-4,9,6,1},plane[]={0,1,0,-.015};Mat4 s;s.m.fill(0);double dot=0;
    for(int i=0;i<4;i++)dot+=plane[i]*light[i];
    for(int c=0;c<4;c++)for(int r=0;r<4;r++)s.m[c*4+r]=(r==c?dot:0)-light[r]*plane[c];
    for(int i=0;i<simulation->count;i++){auto m=s*dieMatrix(simulation->bodies[i].die);glPushMatrix();glMultMatrixd(m.m.data());glCallList(roundedList);glPopMatrix();}
    glPopAttrib();
    for(int i=0;i<simulation->count;i++)contactShadow(simulation->bodies[i].die);
}
void modelScene(){
    Mat4 m=Mat4::rotation({0,1,0},modelAngle);sceneModel=m;
    grid(-1.7);if(axes)basis(2.4);
    glPushMatrix();glMultMatrixd(m.m.data());
    if(explode<.001)body(roundedMode);
    else for(auto f:faces()){
        Vec3 offset=f.normal*explode;
        ui::color(faceColors[static_cast<std::size_t>(f.value-1)]);normal(f.normal);
        glPushAttrib(GL_POLYGON_BIT|GL_ENABLE_BIT);if(wire){glDisable(GL_LIGHTING);glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);}
        glBegin(GL_QUADS);for(auto p:std::array<Vec3,4>{{{-half,-half,0},{half,-half,0},{half,half,0},{-half,half,0}}})vertex(offset+f.normal*half+f.u*p.x+f.v*p.y);glEnd();
        glPopAttrib();pips(f,offset,true);
    }
    if(normals)for(auto face:faces()){
        glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);ui::color(ui::gold);
        Vec3 start=face.normal*(half+explode+.02);segment(start,start+face.normal*.65);glPopAttrib();
    }
    glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);glLineWidth(3);
    for(auto face:faces())if(face.value==selectedFace){
        Vec3 center=face.normal*(half+explode+.018);ui::color(ui::gold);glBegin(GL_LINE_LOOP);
        for(auto p:std::array<Vec3,4>{{{-.53,-.53,0},{.53,-.53,0},{.53,.53,0},{-.53,.53,0}}}){vertex(center+face.u*p.x+face.v*p.y);}
        glEnd();
        ui::color({1,.4,.4});segment(center,center+face.u*.8);label3D(center+face.u*.9,"u");
        ui::color({.4,1,.6});segment(center,center+face.v*.8);label3D(center+face.v*.9,"v");
        ui::color({.4,.7,1});segment(center,center+face.normal*.8);label3D(center+face.normal*.9,"n");
    }glPopAttrib();glPopMatrix();
}
void transformScene(){
    grid(-1.65);if(axes)basis(3);
    Mat4 m=lab.model();sceneModel=m;
    glPushMatrix();glMultMatrixd(lab.initial.m.data());
    glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);ui::color({.26,.35,.45});glutWireCube(2*half);glPopAttrib();glPopMatrix();
    glPushMatrix();glMultMatrixd(m.m.data());body(roundedMode);glPopMatrix();
    if(normals)drawNormals(m);
    glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);ui::color(ui::accent);glLineWidth(2);
    Vec3 previous=lab.initial.point({});for(std::size_t i=1;i<=lab.displayedCount();i++){Vec3 p=lab.at(i).point({});segment(previous,p);previous=p;}segment(previous,m.point({}));
    const auto* active=lab.activeStep();
    if(!active&&compositeLesson&&!lab.steps.empty())active=&lab.steps.back();
    Vec3 pivot=active?active->pivot:lab.pivotEnabled?lab.pivot:Vec3{};
    if((active?active->kind:lab.tool)==Transform::Mirror){
        glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);glColor4d(.35,.65,.9,.15);
        int planeAxis=active?active->axis:lab.axis;
        if(planeAxis<3){
            Vec3 u=planeAxis==0?Vec3{0,1,0}:Vec3{1,0,0},v=planeAxis==2?Vec3{0,1,0}:Vec3{0,0,1};
            glBegin(GL_QUADS);for(auto p:std::array<Vec3,4>{{{-2.5,-2.5,0},{2.5,-2.5,0},{2.5,2.5,0},{-2.5,2.5,0}}})vertex(pivot+u*p.x+v*p.y);glEnd();
        }
        glDepthMask(GL_TRUE);glDisable(GL_BLEND);
    }
    if(pivot.length()>0){
        ui::color(ui::gold);glPushMatrix();glTranslated(pivot.x,pivot.y,pivot.z);glutSolidSphere(.075,16,12);glPopMatrix();label3D(pivot+Vec3{.1,.15,0},"P");
    }
    int axis=active?active->axis:lab.axis;
    if(axis==3){
        if((active?active->kind:lab.tool)==Transform::Rotate){Vec3 d=(active?active->arbitraryAxis:lab.arbitraryAxis).unit();ui::color(ui::gold);segment(pivot-d*3,pivot+d*3);label3D(pivot+d*3,"k");}
        else basis(3);
    }
    else{
        const std::array<Vec3,3> directions{{{1,0,0},{0,1,0},{0,0,1}}};
        const std::array<Color,3> colors{{{1,.36,.38},{.42,.94,.57},{.37,.63,1}}};
        ui::color(colors[static_cast<std::size_t>(axis)]);segment(pivot-directions[static_cast<std::size_t>(axis)]*3,pivot+directions[static_cast<std::size_t>(axis)]*3);
        label3D(pivot+directions[static_cast<std::size_t>(axis)]*3,std::string(1,"XYZ"[axis]),colors[static_cast<std::size_t>(axis)]);
    }
    glPopAttrib();
}
void projectionScene(){
    grid(-1.8);if(axes)basis(3);
    sceneModel=Mat4::rotation({0,1,0},.25)*Mat4::rotation({1,0,0},.12);
    glPushMatrix();glMultMatrixd(sceneModel.m.data());body(false);glPopMatrix();
    if(normals)drawNormals(sceneModel);
    glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);ui::color(ui::gold);
    auto p=sceneModel.point(cubeVertices[static_cast<std::size_t>(selectedVertex)]);
    glPushMatrix();glTranslated(p.x,p.y,p.z);glutSolidSphere(.06,12,12);glPopMatrix();label3D(p+Vec3{.1,.1,0},"V"+std::to_string(selectedVertex));
    // Equal-size reference objects placed at different depths make perspective visible.
    glPushMatrix();glTranslated(-2.3,0,-3.2);body(false,{.42,.70,.81});glPopMatrix();
    glPushMatrix();glTranslated(2.3,0,2.2);body(false,{.73,.67,.92});glPopMatrix();glPopAttrib();
}
void puzzleScene(){
    tray();if(axes)basis(2);
    for(int i=0;i<3;i++){
        auto q=puzzle->visual(i);Die d{{(i-1)*2.65,supportHeight(q),0},q,topFace(q)};
        if(shadows)contactShadow(d);
        drawDie(d,i);
        if(poseKey(puzzle->rotations[i])==poseKey(puzzle->targets[i]))topOutline(d);
    }
    glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT|GL_CURRENT_BIT);glDisable(GL_LIGHTING);glLineWidth(3);ui::color(ui::accent);glBegin(GL_LINE_LOOP);
    for(int j=0;j<64;j++){double a=2*pi*j/64;vertex({(puzzle->selected-1)*2.65+.95*std::cos(a),.015,.95*std::sin(a)});}glEnd();glPopAttrib();
}
void setupView(){
    double p=pitch*pi/180,a=yaw*pi/180;Vec3 target{0,page==Page::Simulation?1.0:0,0};
    Vec3 eye=target+Vec3{cameraDistance*std::cos(p)*std::sin(a),cameraDistance*std::sin(p),cameraDistance*std::cos(p)*std::cos(a)};
    sceneView=lookAt(eye,target,{0,1,0});double extent=cameraDistance*std::tan(fov*pi/360),aspect=sceneW/sceneH;
    if(projectionMode==0)sceneProjection=perspective(fov,aspect,.1,80);
    else sceneProjection=orthographic(-extent*aspect,extent*aspect,-extent,extent,.1,80)*(projectionMode==2?cabinet():Mat4{});
}
void scene(){
    int x=offsetX+int(sceneX*vw/virtualW),y=offsetY+vh-int((sceneY+sceneH)*vh/virtualH);
    int w=int(sceneW*vw/virtualW),h=int(sceneH*vh/virtualH);
    glViewport(x,y,w,h);glEnable(GL_SCISSOR_TEST);glScissor(x,y,w,h);glClearColor(.045f,.078f,.12f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);glDisable(GL_SCISSOR_TEST);
    setupView();glMatrixMode(GL_PROJECTION);glLoadMatrixd(sceneProjection.m.data());glMatrixMode(GL_MODELVIEW);glLoadMatrixd(sceneView.m.data());
    glEnable(GL_DEPTH_TEST);glEnable(GL_NORMALIZE);glEnable(GL_COLOR_MATERIAL);glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    if(lighting&&!wire)glEnable(GL_LIGHTING);else glDisable(GL_LIGHTING);
    glEnable(GL_LIGHT0);glEnable(GL_LIGHT1);GLfloat key[]={-4,9,6,1},fill[]={5,4,-3,1},diff[]={.95f,.92f,.85f,1},fd[]={.20f,.28f,.38f,1},ambient[]={.20f,.23f,.28f,1};
    glLightfv(GL_LIGHT0,GL_POSITION,key);glLightfv(GL_LIGHT0,GL_DIFFUSE,diff);glLightfv(GL_LIGHT1,GL_POSITION,fill);glLightfv(GL_LIGHT1,GL_DIFFUSE,fd);glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);
    sceneModel={};
    switch(page){
        case Page::Simulation:tray();if(shadows)projectedShadows();if(axes)basis(2);
            for(int i=0;i<simulation->count;i++){drawDie(simulation->bodies[i].die,i);if(!simulation->rolling&&!simulation->history.empty())topOutline(simulation->bodies[i].die);}break;
        case Page::Model:modelScene();break;
        case Page::Transform:transformScene();break;
        case Page::Projection:projectionScene();break;
        case Page::Puzzle:puzzleScene();break;
    }
    glDisable(GL_LIGHTING);glDisable(GL_DEPTH_TEST);
}
void uiBegin(){
    glViewport(offsetX,offsetY,vw,vh);glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0,virtualW,virtualH,0,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();
    glDisable(GL_LIGHTING);glDisable(GL_DEPTH_TEST);glDisable(GL_TEXTURE_2D);ui::buttons.clear();
}
void section(double y,const std::string& heading){ui::text(1066,y,heading,ui::accent,18);}
void miniDie(double x,double y,int value,Color c=ui::white,double size=32){
    ui::box(x,y,size,size,c,5);ui::color(ui::background);glPointSize(4*float(vw)/virtualW);
    for(auto p:pipCenters(value)){glBegin(GL_POINTS);glVertex2d(x+size/2+p.x*size*.95,y+size/2-p.y*size*.95);glEnd();}glPointSize(1);
}
void commonOptions(double y){
    ui::button(1064,y,104,"Khung dây",Wire,wire);ui::button(1176,y,104,"Ánh sáng",Lighting,lighting);ui::button(1288,y,104,"Pháp tuyến",Normals,normals);
    ui::button(1064,y+46,104,axes?"Ẩn trục":"Hiện trục",Axes,axes);ui::button(1176,y+46,104,"Bóng đổ",Shadows,shadows,page==Page::Simulation||page==Page::Puzzle);
    const char* names[]={"Nhựa","Gỗ","Kim loại"};ui::button(1288,y+46,104,names[material],Material);
}
double chargePower(){return std::clamp(.12+.88*chargeTime/1.35,.12,1.0);}
double aimPower(){return std::clamp(std::hypot(aimEndX-aimStartX,aimEndY-aimStartY)/220,.12,1.0);}
void simulationPanel(){
    section(210,"ĐIỀU KHIỂN MÔ PHỎNG");
    for(int i=0;i<3;i++)ui::button(1064+i*112,232,104,std::to_string(i+1)+" viên",Count1+i,simulation->count==i+1,!simulation->rolling&&!charging&&!aiming);
    double power=charging?chargePower():aiming?aimPower():simulation->lastPower;
    ui::text(1066,305,"Lực đổ",ui::muted);ui::text(1337,305,std::to_string(int(power*100))+"%",ui::gold);
    ui::box(1064,322,328,10,ui::raised,4);if(power>.001)ui::box(1064,322,328*power,10,ui::accent,4);
    ui::button(1064,356,328,charging?"THẢ ĐỂ ĐỔ":"GIỮ ĐỂ LẤY LỰC",Roll,charging,!simulation->rolling,50);
    ui::button(1064,420,158,simulation->paused?"Tiếp tục  P":"Tạm dừng  P",Pause,simulation->paused,simulation->rolling);
    ui::button(1230,420,162,"Bước 1/120 s",Single,false,simulation->rolling&&simulation->paused);
    ui::button(1064,468,158,"Chậm 0,3x",Slow,slow);ui::button(1230,468,162,soundBank.enabled?"Âm thanh bật":"Âm thanh tắt",SoundToggle,soundBank.enabled);
    section(548,"TRẠNG THÁI VIÊN #1");const auto& b=simulation->bodies[0];
    ui::text(1066,580,"p = "+vec(b.die.position),ui::white,17);
    ui::text(1066,609,"v = "+vec(b.velocity),ui::white,17);
    ui::text(1066,638,"ω = "+vec(b.angular),ui::white,17);
    ui::text(1066,674,"Va bàn "+std::to_string(simulation->tableHits)+"   Va thành "+std::to_string(simulation->wallHits),ui::muted,17);
    ui::text(1066,703,"Giữa các viên "+std::to_string(simulation->pairHits)+"   t = "+fixed(simulation->elapsed)+" s",ui::muted,17);
    commonOptions(740);
}
void modelPanel(){
    section(210,"CẤU TẠO MỘT XÚC XẮC");
    ui::button(1064,232,158,roundedMode?"Khối bo cạnh":"Khối cơ bản",MeshToggle,roundedMode);
    ui::button(1230,232,162,"Tự xoay",AutoRotate,autoRotate);
    ui::text(1066,308,"Tách mặt  "+fixed(explode,1),ui::white,20);
    ui::button(1064,329,158,"− Thu lại",ExplodeLess);ui::button(1230,329,162,"+ Tách ra",ExplodeMore);
    section(407,"MẶT ĐANG CHỌN");
    for(int i=0;i<6;i++){ui::button(1064+i*56,429,48,std::to_string(i+1),Face1+i,selectedFace==i+1);}
    const Face* f=nullptr;for(const auto& face:faces())if(face.value==selectedFace)f=&face;
    ui::text(1066,510,"n = "+vec(f->normal,0),ui::white,18);
    ui::text(1066,540,"u = "+vec(f->u,0)+"   v = "+vec(f->v,0),ui::muted,16);
    ui::text(1066,573,"Chấm "+std::to_string(selectedFace)+" / Mặt đối "+std::to_string(7-selectedFace),ui::gold,20);
    section(625,"DỮ LIỆU HÌNH HỌC");
    const auto& mesh=roundedMode?rounded:basic;
    ui::text(1066,657,std::to_string(mesh.vertices.size())+" bản ghi đỉnh, "+std::to_string(mesh.indices.size()/3)+" tam giác",ui::white,18);
    ui::text(1066,687,"8 góc hình học, 12 cạnh, 6 mặt logic",ui::muted,17);
    ui::text(1066,714,"Chấm tròn = quạt 32 tam giác",ui::muted,17);
    commonOptions(740);
}
void matrixBlock(double x,double y,const Mat4& m,int size=17,double cell=76,double line=29){
    for(int r=0;r<4;r++)for(int c=0;c<4;c++)ui::text(x+c*cell,y+r*line,fixed(m.m[c*4+r]),c==3&&r<3?ui::gold:ui::white,size);
}
std::string toolName(Transform t){switch(t){case Transform::Translate:return "Tịnh tiến";case Transform::Rotate:return "Quay";case Transform::Scale:return "Tỉ lệ";case Transform::Mirror:return "Đối xứng";case Transform::Shear:return "Biến dạng xiên";}return "";}
std::string transformLabel(const TransformStep& s){
    if(s.kind==Transform::Mirror){const char* planes[]={"YZ","XZ","XY"};return std::string("Đối xứng qua ")+(s.axis==3?"O":std::string("mặt ")+planes[s.axis]);}
    std::string axis=s.axis==3?(s.kind==Transform::Rotate?"k":"XYZ"):std::string(1,"XYZ"[s.axis]);
    int decimals=s.kind==Transform::Rotate?(std::abs(s.amount-std::round(s.amount))>1e-6?1:0):2;
    std::string value=" "+fixed(s.amount,decimals)+(s.kind==Transform::Rotate?"°":"");
    return toolName(s.kind)+" "+axis+value+(s.pivot.length()>0?" qua P":"");
}
void configureLesson(){
    compositeLesson=false;matrixInverse=false;
    if(lab.tool==Transform::Translate)lab.pivotEnabled=false;
    double amount=lessonDirection*lab.translationStep;
    if(lab.tool==Transform::Rotate)amount=lessonDirection*lab.rotationStep;
    if(lab.tool==Transform::Scale)amount=lessonDirection>0?lab.scaleStep:1/lab.scaleStep;
    if(lab.tool==Transform::Mirror)amount=-1;
    if(lab.tool==Transform::Shear)amount=lessonDirection*lab.shearStep;
    TransformStep step{lab.tool,lab.axis,amount,lab.pivotEnabled&&lab.tool!=Transform::Translate?lab.pivot:Vec3{},lab.arbitraryAxis};
    Mat4 start;
    if(lab.tool==Transform::Mirror){
        Vec3 offset=lab.axis==0?Vec3{1.8,0,0}:lab.axis==1?Vec3{0,1.8,0}:lab.axis==2?Vec3{0,0,1.8}:Vec3{1.2,.8,1.2};
        start=Mat4::translation(offset);
    }
    lab.preview(step,start);notice="";
}
void transformPanel(){
    const auto* active=lab.activeStep();
    if(!active&&compositeLesson&&!lab.steps.empty())active=&lab.steps.back();
    auto tool=active?active->kind:lab.tool;int axis=active?active->axis:lab.axis;
    section(210,"PHÉP BIẾN ĐỔI 3D");
    const char* names[]={"Tịnh tiến","Quay","Tỉ lệ","Đối xứng","Xiên"};
    for(int i=0;i<5;i++)ui::button(1064+(i%3)*112,232+(i/3)*46,104,names[i],ToolT+i,!compositeLesson&&static_cast<int>(tool)==i);
    ui::button(1288,278,104,"Tổ hợp",Composite,compositeLesson);
    const char* axesNames[]={"X","Y","Z","Tất cả"};
    const char* mirrorNames[]={"mp YZ","mp XZ","mp XY","Gốc O"};
    for(int i=0;i<4;i++)ui::button(1064+i*84,336,76,tool==Transform::Mirror?mirrorNames[i]:tool==Transform::Rotate&&i==3?"Trục k":axesNames[i],AxisX+i,axis==i,!compositeLesson);
    double value=active?active->amount:tool==Transform::Translate?lab.translationStep:tool==Transform::Rotate?lab.rotationStep:tool==Transform::Scale?lab.scaleStep:lab.shearStep;
    std::string amount=tool==Transform::Translate?fixed(value)+" đơn vị":tool==Transform::Rotate?fixed(value,0)+"°":tool==Transform::Scale?"× "+fixed(value):tool==Transform::Shear?fixed(value):"Mặt phẳng đối xứng";
    if(tool==Transform::Mirror){
        const char* planes[]={"YZ","XZ","XY"};ui::text(1066,420,axis==3?"Qua gốc O":std::string("Qua mặt ")+planes[axis],ui::gold,21);
    }else{
        ui::button(1064,394,48,"−",ApplyMinus,false,!compositeLesson);
        ui::text(1122,420,amount,ui::gold,21);
        ui::button(1344,394,48,"+",ApplyPlus,false,!compositeLesson);
    }
    ui::button(1064,451,158,lessonDirection>0?"Chiều dương":"Chiều ngược",DirectionToggle,lessonDirection<0,!compositeLesson&&tool!=Transform::Mirror);
    ui::button(1230,451,162,lab.pivotEnabled?"Tâm P (1,0,0)":"Tâm O",PivotToggle,lab.pivotEnabled,!compositeLesson&&tool!=Transform::Translate);
    if(compositeLesson){
        std::string sequence;
        for(std::size_t i=0;i<lab.steps.size();i++){if(i)sequence+="  →  ";sequence+=lab.steps[i].kind==Transform::Translate?"T":"R";}
        ui::text(1066,526,sequence,ui::accent,24);
    }else if(axis==3&&tool==Transform::Rotate)ui::text(1066,526,"k = "+vec(lab.arbitraryAxis,0),ui::muted,18);
    section(561,"MA TRẬN HIỆN TẠI");
    ui::button(1260,538,132,matrixInverse?"Xem M":"Xem M⁻¹",MatrixToggle,matrixInverse,true,30);
    Mat4 m=lab.model(),inverse;bool invertible=m.inverse(inverse);if(matrixInverse&&invertible)m=inverse;
    matrixBlock(1069,602,m,17,78,30);
    ui::text(1066,751,"p' = M p",ui::gold,20);
    ui::button(1064,799,158,"Pháp tuyến",Normals,normals);ui::button(1230,799,162,"Khung dây",Wire,wire);
}
void projectionPanel(){
    section(210,"PHÉP CHIẾU VÀ HỆ QUAN SÁT");
    ui::button(1064,232,104,"Phối cảnh",Perspective,projectionMode==0);ui::button(1176,232,104,"Trực giao",Ortho,projectionMode==1);ui::button(1288,232,104,"Xiên 1/2",Oblique,projectionMode==2);
    ui::button(1064,282,104,"Mặt trước",Front);ui::button(1176,282,104,"Nhìn trên",Top);ui::button(1288,282,104,"Mặt bên",Side);
    ui::text(1066,355,"FOV = "+fixed(fov,0)+"°   R = "+fixed(cameraDistance,1),ui::white,20);
    ui::button(1064,378,158,"− Góc nhìn",FovLess);ui::button(1230,378,162,"+ Góc nhìn",FovMore);
    section(458,"THEO DÕI MỘT ĐỈNH");ui::button(1280,436,112,"Đổi V"+std::to_string(selectedVertex),VertexNext,false,true,30);
    auto p=cubeVertices[static_cast<std::size_t>(selectedVertex)];auto r=project(p,sceneModel,sceneView,sceneProjection,sceneW,sceneH);
    ui::text(1066,493,"Model "+vec(p),ui::white,16);ui::text(1066,524,"World "+vec({r.world.x,r.world.y,r.world.z}),ui::white,16);
    ui::text(1066,555,"Eye "+vec({r.eye.x,r.eye.y,r.eye.z}),ui::white,16);ui::text(1066,586,"Clip "+vec4(r.clip),ui::white,15);
    ui::text(1066,617,"NDC "+vec(r.ndc),ui::accent,17);ui::text(1066,648,"Pixel trong khung "+vec({r.screen.x,r.screen.y,0},1),ui::gold,16);
    ui::text(1066,679,r.inside?"Đỉnh nằm trong thể tích nhìn":"Đỉnh ngoài thể tích nhìn",ui::muted,17);
    ui::text(1066,731,"p_clip = P V M p_model",ui::white,20);
    ui::text(1066,762,"NDC = (x/w, y/w, z/w)",ui::muted,18);
    ui::button(1064,799,158,"Góc mặc định",ResetCamera);ui::button(1230,799,162,axes?"Ẩn trục":"Hiện trục",Axes,axes);
    ui::text(1066,854,"PDF trang 28–49 / Chiếu và quan sát",ui::muted,15);
}
void puzzlePanel(){
    section(210,"GHÉP HƯỚNG XÚC XẮC");
    for(int i=0;i<3;i++)ui::button(1064+i*112,232,104,"Viên #"+std::to_string(i+1),Die1+i,puzzle->selected==i,!puzzle->busy());
    ui::text(1066,314,"MỤC TIÊU",ui::muted,17);ui::text(1187,314,"TRÊN",ui::muted,15);ui::text(1295,314,"TRƯỚC +Z",ui::muted,15);
    for(int i=0;i<3;i++){
        double y=336+i*48;ui::text(1066,y+23,"Viên #"+std::to_string(i+1),ui::white,18);
        miniDie(1192,y,topFace(puzzle->targets[i]),faceColors[static_cast<std::size_t>(i)]);miniDie(1305,y,frontFace(puzzle->targets[i]),faceColors[static_cast<std::size_t>(i)]);
        if(poseKey(puzzle->rotations[i])==poseKey(puzzle->targets[i]))ui::text(1362,y+23,"✓",ui::accent,20);
    }
    const char* names[]={"X +90°","X −90°","Y +90°","Y −90°","Z +90°","Z −90°"};
    for(int i=0;i<6;i++)ui::button(1064+(i%2)*166,509+(i/2)*46,158,names[i],TurnXp+i,false,!puzzle->busy()&&!puzzle->solved);
    ui::button(1064,655,158,"Gợi ý BFS  H",Hint,false,!puzzle->busy()&&!puzzle->solved);ui::button(1230,655,162,"Bài mới  Space",NewPuzzle,false,!puzzle->busy());
    section(737,"ĐỒ THỊ 24 TRẠNG THÁI");
    ui::text(1066,769,"Một cạnh = quay ±90° quanh X/Y/Z",ui::white,17);
    ui::text(1066,800,"BFS tìm số bước ít nhất",ui::accent,20);
    ui::text(1066,829,"Mặt trước là +Z thế giới, theo trục xanh",ui::muted,15);
    ui::text(1066,854,"Phối hợp hình học và tìm kiếm trạng thái",ui::muted,15);
}
void footer(){
    if(page==Page::Model)return;
    ui::box(28,740,990,132,ui::panel);
    if(page==Page::Simulation){
        std::string result=simulation->rolling?(simulation->paused?"TẠM DỪNG MÔ PHỎNG":"ĐANG CHUYỂN ĐỘNG"):"SẴN SÀNG MÔ PHỎNG";
        if(!simulation->rolling&&!simulation->history.empty()){
            const auto& r=simulation->history.back();result.clear();for(int i=0;i<r.count;i++){if(i)result+=" + ";result+=std::to_string(r.values[i]);}result+=" = "+std::to_string(r.total());
        }
        ui::text(50,notice.empty()?798:781,result,ui::white,28);if(!notice.empty())ui::text(50,813,notice,ui::muted,18);
        int n=std::min(5,static_cast<int>(simulation->history.size()));
        ui::text(50,850,"LỊCH SỬ",ui::accent,15);for(int i=0;i<n;i++){
            const auto& r=simulation->history[simulation->history.size()-1-static_cast<std::size_t>(i)];
            ui::text(150+i*158,850,"#"+std::to_string(r.id)+"  Tổng "+std::to_string(r.total()),ui::muted,17);
        }
    }else if(page==Page::Transform){
        ui::button(50,757,160,lab.playing?"Tạm dừng":lab.replaying&&lab.progress()>0?"Tiếp tục":"Chạy",PlayLab,lab.playing,lab.cursor>0);
        ui::button(220,757,130,"Tiến 25%  .",NextLab,false,lab.cursor>0&&!lab.playing);
        ui::button(360,757,110,"Về đầu",ClearLab);
        if(compositeLesson){
            ui::button(487,757,158,"T rồi R",PresetTR);ui::button(655,757,158,"R rồi T",PresetRT);ui::button(823,757,172,"Phép ngược",PresetInverse);
        }
        const auto* active=lab.activeStep();
        const auto& step=active?*active:lab.steps.back();
        TransformStep shown=step;
        if(active&&step.kind!=Transform::Mirror)shown.amount=step.kind==Transform::Scale?1+(step.amount-1)*lab.phase:step.amount*lab.phase;
        std::string label=transformLabel(shown);
        if(active&&step.kind!=Transform::Mirror)label+=" / "+fixed(step.amount,step.kind==Transform::Rotate?0:2)+(step.kind==Transform::Rotate?"°":"");
        if(compositeLesson)label="Bước "+std::to_string(active?lab.replayCursor+1:lab.cursor)+" / "+std::to_string(lab.cursor)+"   "+label;
        ui::text(50,827,label,ui::white,21);
        ui::text(925,827,std::to_string(int(lab.progress()*100+.5))+"%",ui::gold,21);
        ui::box(50,846,945,7,ui::raised,3);if(lab.progress()>0)ui::box(50,846,945*lab.progress(),7,ui::accent,3);
    }else if(page==Page::Projection){
        ui::text(50,780,projectionMode==0?"Phối cảnh / vật gần lớn hơn, vật xa nhỏ hơn":projectionMode==1?"Trực giao / kích thước ảnh không phụ thuộc độ sâu":"Chiếu xiên Cabinet / hệ số chiều sâu 1/2",ui::white,25);
        ui::text(50,816,"Kéo chuột để đổi θ, φ. Cuộn để đổi R. Nút góc nhìn thay đổi FOV.",ui::muted,18);
        ui::text(50,850,"Khung vàng đánh dấu pixel chiếu của đỉnh đang chọn. Gốc pixel ở góc trên trái khung 3D.",ui::accent,17);
    }else{
        ui::text(50,780,puzzle->solved?"HOÀN THÀNH / "+std::to_string(puzzle->stars())+" SAO":puzzle->message,ui::white,26);
        ui::text(50,816,"Đúng "+std::to_string(puzzle->matched())+"/3 viên   /   Đã đi "+std::to_string(puzzle->moves)+" bước   /   Tối ưu "+std::to_string(puzzle->par)+" bước",ui::accent,21);
        auto q=puzzle->rotations[puzzle->selected];
        ui::text(50,850,"Viên đang chọn có mặt trên "+std::to_string(topFace(q))+", mặt trước (+Z) "+std::to_string(frontFace(q))+". Phím x/y/z quay dương, Shift quay âm.",ui::muted,17);
    }
}
void helpOverlay(){
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glColor4d(0,0,0,.7);glBegin(GL_QUADS);glVertex2d(0,0);glVertex2d(1440,0);glVertex2d(1440,900);glVertex2d(0,900);glEnd();glDisable(GL_BLEND);
    ui::box(225,155,990,590,ui::panel);ui::text(267,214,"HƯỚNG DẪN DEMO",ui::white,32);
    const std::vector<std::string> lines{
        "F2 Mô phỏng  /  F3 Mô hình  /  F4 Biến đổi  /  F5 Phép chiếu  /  F6 Thử thách",
        "Chuột trái kéo để xoay camera. Cuộn để zoom. C trả góc nhìn mặc định.",
        "Mô phỏng: giữ Space rồi thả. Chuột phải kéo để lấy hướng và lực đổ.",
        "P tạm dừng. Dấu chấm tiến một bước 1/120 s. 1–3 chọn số viên.",
        "Biến đổi: 1–5 chọn phép, X/Y/Z/K chọn trục. +/− điều chỉnh mức biến đổi.",
        "Space chạy hoặc tạm dừng. Dấu chấm tiến 25%. Mục Tổ hợp có các ví dụ T/R.",
        "Thử thách: 1–3 chọn viên, x/y/z quay +90°, X/Y/Z quay −90°. H gợi ý BFS.",
        "W khung dây, N pháp tuyến, L ánh sáng. F11 toàn màn hình, F12 chụp BMP.",
        "Tài liệu báo cáo, slide và kịch bản demo nằm trong thư mục docs."};
    for(std::size_t i=0;i<lines.size();i++)ui::text(267,267+i*43,lines[i],i==0?ui::accent:ui::white,19);
    ui::button(947,672,220,"Đóng hướng dẫn  F1",Help,true);
}
void overlay(){
    uiBegin();
    ui::text(28,51,"DICE  /  3D LAB",ui::white,32);
    ui::text(28,81,"MÔ PHỎNG XÚC XẮC VÀ HỌC ĐỒ HỌA MÁY TÍNH",ui::muted,16);
    ui::text(1070,49,"C++17 + OPENGL",ui::accent,20);ui::button(1290,29,122,"Hướng dẫn",Help,help);
    const char* names[]={"F2  Mô phỏng","F3  Mô hình","F4  Biến đổi 3D","F5  Phép chiếu","F6  Thử thách"};
    for(int i=0;i<5;i++)ui::button(28+i*280,108,264,names[i],TabSim+i,static_cast<int>(page)==i,true,44);
    ui::box(1042,174,370,698,ui::panel);
    switch(page){case Page::Simulation:simulationPanel();break;case Page::Model:modelPanel();break;case Page::Transform:transformPanel();break;case Page::Projection:projectionPanel();break;case Page::Puzzle:puzzlePanel();break;}
    footer();
    if(page==Page::Projection)ui::text(48,203,"MODEL / VIEW / PROJECTION",ui::muted,15);
    ui::button(841,184,156,"Góc mặc định  C",ResetCamera,false,true,30);
    if(page==Page::Model||page==Page::Transform)ui::button(660,184,170,"Xuất dữ liệu  E",ExportData,false,true,30);
    if(page==Page::Projection){auto r=project(cubeVertices[static_cast<std::size_t>(selectedVertex)],sceneModel,sceneView,sceneProjection,sceneW,sceneH);
        if(r.inside){double x=sceneX+r.screen.x,y=sceneY+r.screen.y;ui::rectangle(x-8,y-1,16,2,ui::gold);ui::rectangle(x-1,y-8,2,16,ui::gold);ui::text(x+12,y-10,"V"+std::to_string(selectedVertex),ui::gold,18);}
    }
    if(axes){
        double x=88,y=sceneY+sceneH-64;
        const std::array<Vec3,3> directions{{{1,0,0},{0,1,0},{0,0,1}}};
        const std::array<Color,3> colors{{{1,.36,.38},{.42,.94,.57},{.37,.63,1}}};
        glLineWidth(3);
        for(int i=0;i<3;i++){
            auto d=multiply(sceneView,{directions[i].x,directions[i].y,directions[i].z,0});
            ui::color(colors[i]);glBegin(GL_LINES);glVertex2d(x,y);glVertex2d(x+38*d.x,y-38*d.y);glEnd();
            ui::text(x+49*d.x-5,y-49*d.y+6,std::string(1,"XYZ"[i]),colors[i],18);
        }
        glLineWidth(1);
    }
    if(aiming){ui::color(ui::gold);glLineWidth(4);glBegin(GL_LINES);glVertex2d(aimStartX,aimStartY);glVertex2d(aimEndX,aimEndY);glEnd();
        double a=std::atan2(aimEndY-aimStartY,aimEndX-aimStartX);glBegin(GL_TRIANGLES);glVertex2d(aimEndX,aimEndY);glVertex2d(aimEndX-22*std::cos(a-.4),aimEndY-22*std::sin(a-.4));glVertex2d(aimEndX-22*std::cos(a+.4),aimEndY-22*std::sin(a+.4));glEnd();glLineWidth(1);}
    if(help){ui::buttons.clear();helpOverlay();}
}
void display(){glClearColor(float(ui::background.r),float(ui::background.g),float(ui::background.b),1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);scene();overlay();glutSwapBuffers();}
void reshape(int w,int h){
    width=std::max(1,w);height=std::max(1,h);double s=std::min(width/double(virtualW),height/double(virtualH));
    vw=std::max(1,int(virtualW*s));vh=std::max(1,int(virtualH*s));offsetX=(width-vw)/2;offsetY=(height-vh)/2;ui::initializeFonts(s);
}
void resetCamera(){yaw=24;pitch=34;cameraDistance=page==Page::Model?8:page==Page::Transform?10:13.5;fov=42;}
void switchPage(Page next){
    if(simulation->rolling||charging||aiming||puzzle->busy()){notice="Tạm hoàn thành lượt hiện tại trước khi đổi trang";return;}
    page=next;sceneH=page==Page::Model?698:548;help=false;projectionMode=0;resetCamera();notice="";
    if(page==Page::Transform&&lab.steps.empty())configureLesson();
}
void preset(int id){
    lab.reset();matrixInverse=false;lab.pivotEnabled=false;compositeLesson=true;
    TransformStep t{Transform::Translate,0,2,{}},r{Transform::Rotate,2,90,{}};
    if(id==PresetTR){lab.steps={t,r};notice="T rồi R: tâm (2,0,0) quay đến (0,2,0)";}
    if(id==PresetRT){lab.steps={r,t};notice="R rồi T: tâm kết thúc ở (2,0,0)";}
    if(id==PresetPivot){r.axis=3;r.amount=60;r.pivot={1,0,0};r.arbitraryAxis={1,1,0};lab.steps={r};lab.pivotEnabled=true;lab.axis=3;lab.arbitraryAxis={1,1,0};notice="k qua P=(1,0,0), hướng (1,1,0), góc 60°";}
    if(id==PresetInverse){auto backR=r;backR.amount=-90;auto backT=t;backT.amount=-2;lab.steps={t,r,backR,backT};notice="T, R, R⁻¹, T⁻¹ đưa hình về trạng thái ban đầu";}
    lab.cursor=lab.steps.size();lab.tool=Transform::Rotate;lab.rotationStep=id==PresetPivot?60:90;lab.axis=id==PresetPivot?3:2;lab.rewind();
    if(id==PresetPivot)compositeLesson=false;
}
void saveHistory(){if(writable&&!simulation->save(dataDirectory/"throws.csv")){notice="Không lưu được CSV. Kiểm tra quyền ghi thư mục data.";std::cerr<<notice<<'\n';}}
void finishedThrow(){
    notice.clear();
    if(!smokeMode)saveHistory();
}
void launch(double force,Vec3 direction={}){
    if(simulation->launch(force,direction)){accumulator=0;notice.clear();}
    else notice="Không khởi tạo được lượt mô phỏng";
}
void beginCharge(bool fromMouse){
    if(page!=Page::Simulation||simulation->rolling||charging||aiming||help)return;
    charging=true;chargeMouse=fromMouse;chargeTime=0;soundBank.play(Sound::Shake,.5);
}
void releaseCharge(){if(!charging)return;double force=chargePower();charging=false;soundBank.stop();launch(force);}
void action(int id){
    if(id==Help){help=!help;return;}
    if(help)return;
    if(id>=TabSim&&id<=TabPuzzle){switchPage(static_cast<Page>(id-TabSim));return;}
    if(id==ResetCamera){resetCamera();return;}
    if(id==ExportData){
        auto dir=dataDirectory/"exports";bool ok=exportOBJ(dir/"dice_body.obj",roundedMode?rounded:basic,page==Page::Transform?lab.model():Mat4{});
        if(page==Page::Transform)ok=exportTransforms(dir/"transforms.csv",lab)&&ok;
        notice=ok?"Đã xuất dữ liệu vào data/exports":"Xuất dữ liệu thất bại";std::cout<<notice<<" "<<dir<<'\n';return;
    }
    if(id==Wire){wire=!wire;return;}if(id==Normals){normals=!normals;return;}
    if(id==Lighting){lighting=!lighting;return;}if(id==Axes){axes=!axes;return;}if(id==Shadows){shadows=!shadows;return;}
    if(id==Material){material=(material+1)%3;return;}
    if(id>=Count1&&id<=Count3){if(page==Page::Simulation&&!charging&&!aiming)simulation->setCount(id-Count1+1);return;}
    if(id==Pause){if(simulation->rolling)simulation->paused=!simulation->paused;return;}
    if(id==Single){if(simulation->paused&&simulation->step(1.0/120,true))finishedThrow();return;}
    if(id==Slow){slow=!slow;return;}if(id==SoundToggle){soundBank.enabled=!soundBank.enabled;if(!soundBank.enabled)soundBank.stop();return;}
    if(id>=Face1&&id<=Face6){selectedFace=id-Face1+1;return;}
    if(id==ExplodeLess){explode=std::max(0.0,explode-.35);return;}if(id==ExplodeMore){explode=std::min(1.75,explode+.35);return;}
    if(id==MeshToggle){roundedMode=!roundedMode;return;}if(id==AutoRotate){autoRotate=!autoRotate;return;}
    if(id>=ToolT&&id<=ToolShear){lab.tool=static_cast<Transform>(id-ToolT);configureLesson();return;}
    if(id>=AxisX&&id<=AxisK){if(!compositeLesson){lab.axis=id-AxisX;configureLesson();}return;}
    if(id==DirectionToggle){if(!compositeLesson){lessonDirection=-lessonDirection;configureLesson();}return;}
    if(id==PivotToggle){if(!compositeLesson){lab.pivotEnabled=!lab.pivotEnabled;configureLesson();}return;}
    if(id==Composite){preset(PresetTR);return;}
    if(id==ApplyPlus||id==ApplyMinus){
        if(compositeLesson)return;
        int sign=id==ApplyPlus?1:-1;
        if(lab.tool==Transform::Translate)lab.translationStep=std::clamp(lab.translationStep+sign*.5,.5,4.0);
        if(lab.tool==Transform::Rotate)lab.rotationStep=std::clamp(lab.rotationStep+sign*15.0,15.0,180.0);
        if(lab.tool==Transform::Scale)lab.scaleStep=std::clamp(lab.scaleStep+sign*.25,.25,3.0);
        if(lab.tool==Transform::Shear)lab.shearStep=std::clamp(lab.shearStep+sign*.25,.25,2.0);
        configureLesson();return;
    }
    if(id==ClearLab){lab.rewind();return;}
    if(id==PlayLab){if(lab.steps.empty())configureLesson();lab.play();return;}if(id==NextLab){lab.advance();return;}
    if(id>=PresetTR&&id<=PresetInverse){preset(id);return;}if(id==MatrixToggle){matrixInverse=!matrixInverse;return;}
    if(id>=Perspective&&id<=Oblique){projectionMode=id-Perspective;if(projectionMode==2){yaw=0;pitch=0;}return;}
    if(id==Front||id==Top||id==Side){projectionMode=1;yaw=id==Side?90:0;pitch=id==Top?89.9:0;return;}
    if(id==VertexNext){selectedVertex=(selectedVertex+1)%8;return;}if(id==FovLess){fov=std::max(20.0,fov-5);return;}if(id==FovMore){fov=std::min(75.0,fov+5);return;}
    if(id>=Die1&&id<=Die3){if(!puzzle->busy())puzzle->selected=id-Die1;return;}
    if(id>=TurnXp&&id<=TurnZm){puzzle->turn((id-TurnXp)/2,(id-TurnXp)%2==0?1:-1);return;}
    if(id==Hint){puzzle->hint();return;}if(id==NewPuzzle){puzzle->newPuzzle();return;}
}
void key(unsigned char k,int,int){
    if(k==27){help=false;if(charging){charging=false;soundBank.stop();}aiming=false;return;}
    if(k=='?'){action(Help);return;}if(help)return;
    if(k=='c'){action(ResetCamera);return;}if(k=='w'){action(Wire);return;}if(k=='n'){action(Normals);return;}if(k=='l'){action(Lighting);return;}
    if(k=='e'&&(page==Page::Model||page==Page::Transform)){action(ExportData);return;}
    if(page==Page::Simulation){if(k==' ')beginCharge(false);if(k=='p')action(Pause);if(k=='.')action(Single);if(k>='1'&&k<='3')action(Count1+k-'1');}
    if(page==Page::Model){if(k=='+')action(ExplodeMore);if(k=='-')action(ExplodeLess);if(k>='1'&&k<='6')action(Face1+k-'1');}
    if(page==Page::Transform){
        if(k>='1'&&k<='5')action(ToolT+k-'1');
        if(k=='x')action(AxisX);
        if(k=='y')action(AxisY);
        if(k=='z')action(AxisZ);
        if(k=='k')action(AxisK);
        if(k=='+'||k=='=')action(ApplyPlus);
        if(k=='-')action(ApplyMinus);
        if(k=='r')action(ClearLab);
        if(k==' ')action(PlayLab);
        if(k=='.')action(NextLab);
    }
    if(page==Page::Projection){if(k=='o')action(Ortho);if(k=='v')action(VertexNext);}
    if(page==Page::Puzzle){
        if(k>='1'&&k<='3')action(Die1+k-'1');
        if(k=='h')action(Hint);
        if(k==' ')action(NewPuzzle);
        if(k=='x'||k=='X')action(k=='x'?TurnXp:TurnXm);
        if(k=='y'||k=='Y')action(k=='y'?TurnYp:TurnYm);
        if(k=='z'||k=='Z')action(k=='z'?TurnZp:TurnZm);
    }
    glutPostRedisplay();
}
void keyUp(unsigned char k,int,int){if(k==' '&&charging&&!chargeMouse)releaseCharge();}
bool inScene(double x,double y){return x>=sceneX&&x<=sceneX+sceneW&&y>=sceneY&&y<=sceneY+sceneH;}
std::pair<double,double> virtualPoint(int x,int y){return {(x-offsetX)*double(virtualW)/vw,(y-(height-offsetY-vh))*double(virtualH)/vh};}
void mouse(int button,int state,int x,int y){
    auto [mx,my]=virtualPoint(x,y);
    if(button==GLUT_LEFT_BUTTON){
        if(state==GLUT_UP){dragging=false;if(charging&&chargeMouse)releaseCharge();return;}
        int id=ui::hit(mx,my);if(id>=0){if(id==Roll)beginCharge(true);else action(id);return;}
        if(!help&&inScene(mx,my)){dragging=true;lastX=x;lastY=y;}
    }
    if(button==GLUT_RIGHT_BUTTON&&page==Page::Simulation&&!help){
        if(state==GLUT_DOWN&&inScene(mx,my)&&!simulation->rolling&&!charging){aiming=true;aimStartX=aimEndX=mx;aimStartY=aimEndY=my;}
        else if(state==GLUT_UP&&aiming){
            aimEndX=mx;aimEndY=my;double dx=aimEndX-aimStartX,dy=aimEndY-aimStartY,force=aimPower(),a=yaw*pi/180;
            aiming=false;Vec3 direction{dx*std::cos(a)+dy*std::sin(a),0,-dx*std::sin(a)+dy*std::cos(a)};launch(force,direction);
        }
    }
}
void motion(int x,int y){
    if(dragging){yaw+=(x-lastX)*.4;pitch=std::clamp(pitch+(y-lastY)*.3,-75.0,89.9);lastX=x;lastY=y;}
    if(aiming){auto p=virtualPoint(x,y);aimEndX=p.first;aimEndY=p.second;}glutPostRedisplay();
}
void wheel(int,int direction,int,int){if(!help)cameraDistance=std::clamp(cameraDistance*(direction>0?.91:1.1),3.2,28.0);}
void entry(int state){if(state==GLUT_LEFT){dragging=false;aiming=false;if(charging&&chargeMouse){charging=false;soundBank.stop();}}}
void update(double dt){
    if(help)return;
    if(charging)chargeTime+=dt;
    if(page==Page::Transform)lab.tick(dt);
    if(page==Page::Model&&autoRotate)modelAngle+=dt*.4;
    if(page==Page::Puzzle&&puzzle->tick(dt))soundBank.play(Sound::Win,.5);
    soundCooldown=std::max(0.0,soundCooldown-dt);
    if(page!=Page::Simulation)return;
    accumulator+=std::min(dt,.25)*(slow?.3:1);
    while(accumulator>=1.0/120){
        if(simulation->step(1.0/120))finishedThrow();
        double impact=std::max({simulation->tableImpact,simulation->wallImpact,simulation->pairImpact});
        if(impact>.65&&soundCooldown<=0){soundBank.play(simulation->wallImpact>=impact?Sound::Wall:Sound::Table,std::clamp(impact/6,.15,1.0));soundCooldown=.075;}
        simulation->tableImpact=simulation->wallImpact=simulation->pairImpact=0;accumulator-=1.0/120;
    }
}
void timer(int){auto now=std::chrono::steady_clock::now();double dt=std::chrono::duration<double>(now-lastTime).count();lastTime=now;update(std::min(.25,dt));glutPostRedisplay();glutTimerFunc(16,timer,0);}
bool screenshot(const std::filesystem::path& path){
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width*height*3));glReadBuffer(GL_FRONT);glPixelStorei(GL_PACK_ALIGNMENT,1);glFinish();glReadPixels(0,0,width,height,GL_BGR_EXT,GL_UNSIGNED_BYTE,pixels.data());
    std::ofstream out(path,std::ios::binary);if(!out)return false;
    auto u16=[&](unsigned v){out.put(char(v));out.put(char(v>>8));};auto u32=[&](unsigned v){u16(v&65535);u16(v>>16);};unsigned row=(unsigned(width)*3+3)&~3u;
    out<<"BM";u32(54+row*unsigned(height));u32(0);u32(54);u32(40);u32(unsigned(width));u32(unsigned(height));u16(1);u16(24);u32(0);u32(row*unsigned(height));u32(2835);u32(2835);u32(0);u32(0);
    for(int y=0;y<height;y++){out.write(reinterpret_cast<const char*>(pixels.data()+y*width*3),width*3);for(unsigned p=unsigned(width*3);p<row;p++)out.put(0);}return bool(out);
}
void special(int keyValue,int,int){
    if(keyValue==GLUT_KEY_F1){action(Help);return;}
    if(keyValue>=GLUT_KEY_F2&&keyValue<=GLUT_KEY_F6){action(TabSim+keyValue-GLUT_KEY_F2);return;}
    if(keyValue==GLUT_KEY_F11){fullscreen=!fullscreen;if(fullscreen)glutFullScreen();else glutReshapeWindow(1280,800);return;}
    if(keyValue==GLUT_KEY_F12){auto dir=dataDirectory/"captures";std::error_code ec;std::filesystem::create_directories(dir,ec);auto file=dir/(std::to_string(std::chrono::system_clock::now().time_since_epoch().count())+".bmp");display();notice=!ec&&screenshot(file)?"Đã chụp BMP trong data/captures":"Lỗi chụp ảnh";}
}
int smoke(const std::filesystem::path& dir){
    std::filesystem::create_directories(dir);int failures=0;
    auto capture=[&](const std::string& name){display();failures+=!screenshot(dir/(name+".bmp"));failures+=glGetError()!=GL_NO_ERROR;};
    capture("01_simulation");
    key(' ',0,0);update(.7);failures+=!charging;keyUp(' ',0,0);failures+=!simulation->rolling;
    for(int i=0;i<30;i++)update(1.0/120);
    capture("02_in_motion");
    action(Pause);double before=simulation->elapsed;update(.1);failures+=simulation->elapsed!=before;action(Single);failures+=std::abs(simulation->elapsed-before-1.0/120)>1e-8;capture("03_single_step");action(Pause);
    for(int i=0;i<1400&&simulation->rolling;i++)update(1.0/120);
    failures+=simulation->rolling||simulation->history.size()!=1;
    for(int i=0;i<simulation->count;i++)failures+=simulation->history.back().values[i]!=topFace(simulation->bodies[i].die.rotation);
    capture("04_result");
    action(TabModel);capture("05_model");action(ExplodeMore);action(ExplodeMore);action(Face3);capture("06_faces");action(Wire);capture("07_wireframe");action(Wire);
    action(TabTransform);
    const char* lessonCaptures[]={"21_translate","22_rotate","23_scale","24_mirror","25_shear"};
    for(int kind=0;kind<5;kind++){
        action(ToolT+kind);action(AxisX);
        failures+=compositeLesson||lab.steps.size()!=1||static_cast<int>(lab.steps[0].kind)!=kind||lab.progress()!=0;
        auto step=lab.steps[0];auto start=lab.initial;
        action(PlayLab);update(kind==3?1.6:.8);
        Vec3 p{.4,.3,.2};failures+=(lab.model().point(p)-(step.matrix(kind==3?1:.5)*start).point(p)).length()>1e-8;
        capture(lessonCaptures[kind]);
    }
    // Choosing a tool replaces the previous lesson, including while it is playing.
    display();mouse(GLUT_LEFT_BUTTON,GLUT_DOWN,1228,250);
    failures+=lab.tool!=Transform::Rotate||lab.steps.size()!=1||lab.steps[0].kind!=Transform::Rotate||lab.progress()!=0;
    display();mouse(GLUT_LEFT_BUTTON,GLUT_DOWN,285,776);failures+=std::abs(lab.progress()-.25)>1e-9;
    action(PlayLab);update(.3);display();mouse(GLUT_LEFT_BUTTON,GLUT_DOWN,1330,250);
    failures+=lab.tool!=Transform::Scale||lab.playing||lab.progress()!=0;
    action(Composite);failures+=!compositeLesson||lab.steps.size()!=2;
    action(PlayLab);update(3.2);failures+=(lab.model().point({})-Vec3{0,2,0}).length()>1e-8;capture("08_transform_TR");action(PlayLab);update(.8);action(PlayLab);
    auto frozen=lab.model().point({});update(.1);failures+=(lab.model().point({})-frozen).length()>1e-8;capture("09_playback");
    action(NextLab);action(NextLab);failures+=(lab.model().point({})-Vec3{2,0,0}).length()>1e-8;
    for(int i=0;i<4;i++)action(NextLab);
    failures+=lab.replaying;
    action(PresetRT);action(PlayLab);update(3.2);capture("10_transform_RT");action(PresetPivot);action(PlayLab);update(1.6);normals=true;capture("11_arbitrary_axis");normals=false;
    action(PresetInverse);action(PlayLab);update(6.4);failures+=(lab.model().point({1,2,3})-Vec3{1,2,3}).length()>1e-8;capture("12_inverse");
    action(TabProjection);capture("13_perspective");
    // The CPU inspector must agree with the exact matrices loaded into OpenGL.
    GLdouble v[16]{},p[16]{};glGetDoublev(GL_MODELVIEW_MATRIX,v);glGetDoublev(GL_PROJECTION_MATRIX,p);
    scene();glGetDoublev(GL_MODELVIEW_MATRIX,v);glGetDoublev(GL_PROJECTION_MATRIX,p);
    for(int i=0;i<16;i++){failures+=std::abs(v[i]-sceneView.m[i])>1e-4;failures+=std::abs(p[i]-sceneProjection.m[i])>1e-4;}
    action(Ortho);capture("14_orthographic");action(Oblique);capture("15_oblique");action(Front);capture("16_front");action(Top);capture("17_top");
    action(TabPuzzle);capture("18_puzzle");
    for(int i=0;i<3;i++){
        action(Die1+i);auto path=shortestTurns(puzzle->rotations[i],puzzle->targets[i]);
        for(auto t:path){action(TurnXp+t.axis*2+(t.sign<0));update(.25);}
    }
    failures+=!puzzle->solved;capture("19_puzzle_solved");action(Help);capture("20_help");action(Help);
    // Exercise actual hitboxes and mouse releases after the documented visual captures.
    action(TabSim);display();action(Axes);failures+=!axes;capture("26_axes");action(Axes);failures+=axes;
    display();mouse(GLUT_LEFT_BUTTON,GLUT_DOWN,1100,250);failures+=simulation->count!=1;
    mouse(GLUT_LEFT_BUTTON,GLUT_DOWN,1200,380);update(.4);failures+=!charging;
    mouse(GLUT_LEFT_BUTTON,GLUT_UP,1200,380);failures+=charging||!simulation->rolling;
    for(int i=0;i<1400&&simulation->rolling;i++)update(1.0/120);
    failures+=simulation->rolling||simulation->history.back().count!=1;
    display();mouse(GLUT_RIGHT_BUTTON,GLUT_DOWN,300,400);motion(480,380);failures+=!aiming||aimPower()<.7;
    mouse(GLUT_RIGHT_BUTTON,GLUT_UP,480,380);failures+=aiming||!simulation->rolling;
    for(int i=0;i<1400&&simulation->rolling;i++)update(1.0/120);
    failures+=simulation->rolling||simulation->history.size()!=3;
    reshape(1024,768);display();failures+=glGetError()!=GL_NO_ERROR;
    std::cout<<"OpenGL smoke: "<<(failures?"FAIL":"PASS")<<" ("<<failures<<" errors) / "<<glGetString(GL_VERSION)<<'\n';return failures?1:0;
}
void createLists(){
    rounded=roundedCube();basic=basicCube();
    auto compile=[](const Mesh& mesh){GLuint list=glGenLists(1);glNewList(list,GL_COMPILE);glBegin(GL_TRIANGLES);for(auto i:mesh.indices){normal(mesh.vertices[i].normal);vertex(mesh.vertices[i].position);}glEnd();glEndList();return list;};
    roundedList=compile(rounded);basicList=compile(basic);glGenTextures(1,&woodTexture);glBindTexture(GL_TEXTURE_2D,woodTexture);
    std::array<unsigned char,64*64> pixels{};for(int y=0;y<64;y++)for(int x=0;x<64;x++)pixels[static_cast<std::size_t>(y*64+x)]=static_cast<unsigned char>((.82+.12*std::sin(x*.45+std::sin(y*.15)*1.2))*255);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,64,64,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,pixels.data());
}
} // namespace
int main(int argc,char** argv){
    std::uint32_t seed=2026;std::filesystem::path captures;dataDirectory=std::filesystem::absolute(argv[0]).parent_path()/"data";
    for(int i=1;i<argc;i++){
        std::string arg=argv[i];if(arg=="--test")return runTests();
        if(arg=="--help"){std::cout<<"Dice3D_Edu [--seed UINT32] [--data-dir PATH] [--demo] [--smoke CAPTURE_DIR]\n";return 0;}
        if(arg=="--demo"){demoMode=true;continue;}
        if((arg=="--seed"||arg=="--data-dir"||arg=="--smoke")&&i+1<argc){
            std::string value=argv[++i];
            if(arg=="--seed"){try{std::size_t end=0;auto n=std::stoull(value,&end);if(end!=value.size()||n>UINT32_MAX)throw std::out_of_range("seed");seed=static_cast<std::uint32_t>(n);}catch(...){std::cerr<<"Invalid seed\n";return 2;}}
            if(arg=="--data-dir")dataDirectory=std::filesystem::absolute(value);
            if(arg=="--smoke"){smokeMode=true;captures=std::filesystem::absolute(value);}
        }else{std::cerr<<"Unknown or incomplete option: "<<arg<<'\n';return 2;}
    }
    simulation=std::make_unique<ThrowSimulation>(seed);puzzle=std::make_unique<FacePuzzle>(seed^0xA531u);
    if(!smokeMode&&!demoMode&&std::filesystem::exists(dataDirectory/"throws.csv")&&!simulation->load(dataDirectory/"throws.csv")){writable=false;notice="CSV lỗi được giữ nguyên. Dùng --data-dir để chọn thư mục mới.";}
    if(demoMode){writable=false;notice="Giữ Space để lấy lực, thả để đổ";}
    if(smokeMode){soundBank.enabled=false;writable=false;}
    std::cout<<"DICE / 3D LAB - Education Edition\nseed="<<seed<<"\nData: "<<dataDirectory<<'\n';
    int glutArgc=1;glutInit(&glutArgc,argv);glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL|GLUT_MULTISAMPLE);glutInitWindowSize(1440,900);int window=glutCreateWindow("Dice 3D - Education Edition");
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);glutIgnoreKeyRepeat(1);createLists();
    glutDisplayFunc(display);glutReshapeFunc(reshape);glutKeyboardFunc(key);glutKeyboardUpFunc(keyUp);glutSpecialFunc(special);glutMouseFunc(mouse);glutMotionFunc(motion);glutMouseWheelFunc(wheel);glutEntryFunc(entry);reshape(1440,900);
    if(smokeMode){glutMainLoopEvent();int result=smoke(captures);ui::releaseFonts();glDeleteLists(roundedList,1);glDeleteLists(basicList,1);glDeleteTextures(1,&woodTexture);glutDestroyWindow(window);return result;}
    lastTime=std::chrono::steady_clock::now();glutTimerFunc(16,timer,0);glutMainLoop();return 0;
}
