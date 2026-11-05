#include "puzzle.hpp"
#include <queue>
#include <map>
#include <fstream>

namespace dice {
namespace {
Vec3 axisVector(int axis){return axis==0?Vec3{1,0,0}:axis==1?Vec3{0,1,0}:Vec3{0,0,1};}
Quat turned(Quat q,Turn t){return (Quat::axisAngle(axisVector(t.axis),t.sign*pi/2)*q).unit();}
}
int frontFace(Quat q){
    int value=1;double best=-2;
    for(auto f:faces()){double z=q.rotate(f.normal).z;if(z>best){best=z;value=f.value;}}
    return value;
}
int poseKey(Quat q){return topFace(q)*7+frontFace(q);}
std::vector<Turn> shortestTurns(Quat from,Quat to){
    struct Node {Quat rotation;std::vector<Turn> path;};
    std::queue<Node> queue;std::map<int,bool> seen;
    queue.push({from,{}});seen[poseKey(from)]=true;
    while(!queue.empty()){
        Node current=queue.front();queue.pop();
        if(poseKey(current.rotation)==poseKey(to))return current.path;
        for(int axis=0;axis<3;axis++)for(int sign:{1,-1}){
            Turn turn{axis,sign};Quat next=turned(current.rotation,turn);
            if(!seen[poseKey(next)]){seen[poseKey(next)]=true;auto path=current.path;path.push_back(turn);queue.push({next,std::move(path)});}
        }
    }
    return {}; // Inputs used by the puzzle are always cube orientations.
}
FacePuzzle::FacePuzzle(std::uint32_t seed):rng_(seed){newPuzzle();}
void FacePuzzle::newPuzzle(){
    if(animating_)return;
    moves=0;hints=0;par=0;solved=false;selected=0;
    std::vector<Quat> orientations;
    for(int face=1;face<=6;face++)for(int yaw=0;yaw<4;yaw++)orientations.push_back(faceUp(face,yaw*pi/2));
    std::uniform_int_distribution<int> orientation(0,23);
    for(int i=0;i<3;i++){
        rotations[i]=orientations[orientation(rng_)];
        do{targets[i]=orientations[orientation(rng_)];}while(poseKey(rotations[i])==poseKey(targets[i]));
        par+=static_cast<int>(shortestTurns(rotations[i],targets[i]).size());
    }
    message="Ghep mat tren va mat truoc";
}
bool FacePuzzle::turn(int axis,int sign){
    if(animating_||solved||selected<0||selected>2||axis<0||axis>2||(sign!=1&&sign!=-1))return false;
    start_=rotations[selected];animatedDie_=selected;
    rotations[selected]=turned(start_,{axis,sign});moves++;time_=0;animating_=true;
    message=std::string(1,"XYZ"[axis])+(sign>0?" +90":" -90");return true;
}
Quat FacePuzzle::visual(int index) const {
    if(animating_&&index==animatedDie_){double u=std::clamp(time_/0.24,0.0,1.0);return Quat::slerp(start_,rotations[index],u*u*(3-2*u));}
    return rotations[index];
}
int FacePuzzle::matched() const {int n=0;for(int i=0;i<3;i++)if(poseKey(rotations[i])==poseKey(targets[i]))n++;return n;}
int FacePuzzle::stars() const {return solved?std::min(hints?2:3,moves<=par?3:moves<=par+4?2:1):0;}
bool FacePuzzle::hint(){
    if(animating_||solved)return false;
    auto path=shortestTurns(rotations[selected],targets[selected]);
    if(path.empty()){message="Vien nay da dung";return false;}
    hints++;message=std::string("Goi y: ")+"XYZ"[path[0].axis]+(path[0].sign>0?" +90":" -90");return true;
}
bool FacePuzzle::tick(double dt){
    if(!animating_||dt<=0)return false;
    time_+=dt;if(time_<0.24)return false;
    animating_=false;
    if(matched()==3){solved=true;message="HOAN THANH - "+std::to_string(stars())+" sao";return true;}
    return false;
}
} // namespace dice
