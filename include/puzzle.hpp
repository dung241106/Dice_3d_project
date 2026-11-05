#pragma once
#include "dice.hpp"

namespace dice {
struct Turn {int axis=0,sign=1;};
int frontFace(Quat q);
int poseKey(Quat q); // A cube has 24 orientations; top+front uniquely identify one.
std::vector<Turn> shortestTurns(Quat from,Quat to);

class FacePuzzle {
public:
    explicit FacePuzzle(std::uint32_t seed);
    std::array<Quat,3> rotations{},targets{};
    int selected=0,moves=0,hints=0,par=0;
    bool solved=false;
    std::string message="Ghep mat tren va mat truoc";
    void newPuzzle();
    bool turn(int axis,int sign);
    bool hint();
    bool tick(double dt); // True once, when a puzzle is solved after the last animation.
    bool busy() const {return animating_;}
    Quat visual(int index) const;
    int matched() const;
    int stars() const;
private:
    std::mt19937 rng_;
    Quat start_{};
    int animatedDie_=0;
    double time_=0;
    bool animating_=false;
};

} // namespace dice
