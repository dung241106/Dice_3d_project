#pragma once
#include "dice.hpp"

namespace dice
{
struct Turn
{
    int axis = 0;
    int sign = 1;
};

int frontFace(Quat q);
int poseKey(Quat q); // A cube has 24 orientations; top+front uniquely identify one.
std::vector<Turn> shortestTurns(Quat from, Quat to);

class FacePuzzle
{
public:
    explicit FacePuzzle(std::uint32_t seed);
    std::array<Quat, 3> rotations{};
    std::array<Quat, 3> targets{};
    int selected = 0;
    int moves = 0;
    int hints = 0;
    int par = 0;
    bool solved = false;
    std::string message = "Ghep mat tren va mat truoc";
    void newPuzzle();
    bool turn(int axis, int sign);
    bool hint();
    bool tick(double dt); // True once, when a puzzle is solved after the last animation.

    bool busy() const
    {
        return animating_;
    }

    Quat visual(int index) const;
    int matched() const;
    int stars() const;

private:
    std::mt19937 rng_;
    Quat start_{};
    int animatedDie_ = 0;
    double time_ = 0;
    bool animating_ = false;
};

} // namespace dice
