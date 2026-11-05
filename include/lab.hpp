#pragma once
#include "dice.hpp"

namespace dice {
struct Mat4 {
    std::array<double,16> m{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    Mat4 operator*(const Mat4& b) const;
    Vec3 point(Vec3 p) const;
    Vec3 normal(Vec3 n) const;
    static Mat4 translation(Vec3 t);
    static Mat4 scale(Vec3 s);
    static Mat4 rotation(Vec3 axis,double angle);
    static Mat4 shear(int axis,double value);
    bool inverse(Mat4& result) const; // Returns false for singular matrices.
};
enum class Transform { Translate,Rotate,Scale,Mirror,Shear };
struct TransformStep {
    Transform kind=Transform::Translate;
    int axis=0;double amount=0;
    Vec3 pivot;
    Vec3 arbitraryAxis{1,1,0};
    Mat4 matrix(double t=1) const;
    std::string label() const;
};
class Lab {
public:
    Transform tool=Transform::Translate;
    int axis=0;
    bool pivotEnabled=false,playing=false;
    Vec3 pivot{1,0,0},arbitraryAxis{1,1,0};
    double translationStep=2,rotationStep=90,scaleStep=1.5,shearStep=0.75;
    Mat4 initial;
    std::vector<TransformStep> steps;
    std::size_t cursor=0;
    double phase=0;
    std::size_t replayCursor=0;
    bool replaying=false;
    std::size_t displayedCount() const {return replaying?replayCursor:cursor;}
    const TransformStep* activeStep() const;
    void stopPlayback();
    Mat4 model() const;
    Mat4 at(std::size_t count) const;
    void apply(int direction);
    void undo();
    void redo();
    void reset();
    void rewind();
    void preview(TransformStep step,Mat4 start=Mat4{});
    void play();
    void nextPlaybackStep();
    void advance(double fraction=0.25);
    double progress() const;
    void tick(double dt);
};
int testLab();
} // namespace dice
