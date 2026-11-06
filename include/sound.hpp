#pragma once
#include <array>
#include <vector>
#include <cstdint>

namespace dice
{
enum class Sound
{
    Shake,
    Table,
    Wall,
    Win
};
std::vector<unsigned char> synthWave(Sound cue);

class SoundBank
{
public:
    SoundBank();
    ~SoundBank();
    bool enabled = true;
    bool play(Sound cue, double volume = 1);
    void stop();

private:
    std::array<std::array<std::vector<unsigned char>, 4>, 4> waves_;
};
} // namespace dice
