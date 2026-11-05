#pragma once
#include "dice.hpp"

namespace dice {
double supportExtent(Quat q,Vec3 direction);
Vec3 supportPoint(Quat q,Vec3 direction);
struct ThrowRecord {
    unsigned id=0;
    int count=3;
    double power=0,duration=0;
    std::array<int,3> values{};
    int tableHits=0,wallHits=0;
    int total() const {int sum=0;for(int i=0;i<count;i++)sum+=values[i];return sum;}
};
struct ThrowBody {
    Die die;
    Vec3 velocity,angular;
    bool sleeping=true,settling=false;
    double quiet=0,settleTime=0;
    Quat settleStart,settleTarget;
};
class ThrowSimulation {
public:
    explicit ThrowSimulation(std::uint32_t seed);
    std::array<ThrowBody,3> bodies;
    std::vector<ThrowRecord> history;
    int count=3,tableHits=0,wallHits=0;
    bool rolling=false,paused=false;
    double elapsed=0,lastPower=0,tableImpact=0,wallImpact=0,pairImpact=0;
    int pairHits=0;
    static constexpr double wallX=5.0,wallZ=3.0;
    bool setCount(int value);
    bool launch(double power,Vec3 direction={});
    bool step(double dt,bool single=false);
    bool load(const std::filesystem::path& file);
    bool save(const std::filesystem::path& file) const;
    long long cumulative() const;
private:
    std::mt19937 rng_;
    void integrate(double dt);
    void contact(ThrowBody& b,Vec3 normal,double penetration,bool table);
    void constrain(ThrowBody& b);
    void settle(ThrowBody& b);
};
int testThrows();
} // namespace dice
