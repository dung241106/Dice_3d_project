#include "sound.hpp"
#include "dice.hpp"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

namespace dice
{
std::vector<unsigned char> synthWave(Sound cue)
{
    constexpr int rate = 22050;
    double duration = cue == Sound::Win ? 0.6 : cue == Sound::Shake ? 0.22 : 0.095;
    int count = static_cast<int>(rate * duration);
    std::vector<unsigned char> wave;
    auto u16 = [&](unsigned n)
    {
        wave.push_back(static_cast<unsigned char>(n));
        wave.push_back(static_cast<unsigned char>(n >> 8));
    };
    auto u32 = [&](unsigned n)
    {
        u16(n & 65535);
        u16(n >> 16);
    };
    auto tag = [&](const char* s)
    {
        for (int i = 0; i < 4; i++)
        {
            wave.push_back(static_cast<unsigned char>(s[i]));
        }
    };
    tag("RIFF");
    u32(36 + 2 * count);
    tag("WAVE");
    tag("fmt ");
    u32(16);
    u16(1);
    u16(1);
    u32(rate);
    u32(2 * rate);
    u16(2);
    u16(16);
    tag("data");
    u32(2 * count);
    std::mt19937 rng(1234 + static_cast<unsigned>(cue));
    std::uniform_real_distribution<double> noise(-1, 1);
    for (int i = 0; i < count; i++)
    {
        double t = static_cast<double>(i) / rate, value = 0;
        if (cue == Sound::Win)
        {
            int note = std::min(2, static_cast<int>(t / 0.2));
            double local = t - note * 0.2;
            double frequencies[] = {523.25, 659.25, 783.99};
            value = 0.23 * std::sin(2 * pi * frequencies[note] * t) *
                    std::sin(pi * std::min(1.0, local / 0.2));
        }
        else if (cue == Sound::Shake)
        {
            double local = std::fmod(t, 0.045);
            value = 0.31 * noise(rng) * std::exp(-local * 110) * (1 - t / duration);
        }
        else
        {
            double frequency = cue == Sound::Table ? 175 : 340;
            value =
                (0.28 * noise(rng) + 0.24 * std::sin(2 * pi * frequency * t)) * std::exp(-t * 65);
        }
        auto sample = static_cast<std::int16_t>(std::clamp(value, -1.0, 1.0) * 32767);
        u16(static_cast<std::uint16_t>(sample));
    }
    return wave;
}

SoundBank::SoundBank()
{
    for (int cue = 0; cue < 4; cue++)
    {
        auto source = synthWave(static_cast<Sound>(cue));
        for (int level = 0; level < 4; level++)
        {
            auto& wave = waves_[cue][level];
            wave = source;
            for (std::size_t i = 44; i + 1 < wave.size(); i += 2)
            {
                int sample = source[i] | (source[i + 1] << 8);
                if (sample >= 32768)
                {
                    sample -= 65536;
                }
                auto scaled = static_cast<std::uint16_t>(sample * (level + 1) / 4);
                wave[i] = static_cast<unsigned char>(scaled);
                wave[i + 1] = static_cast<unsigned char>(scaled >> 8);
            }
        }
    }
}

SoundBank::~SoundBank()
{
    stop();
}

bool SoundBank::play(Sound cue, double volume)
{
    if (!enabled)
    {
        return false;
    }
#ifdef _WIN32
    const int level = static_cast<int>(std::clamp(volume, 0.0, 0.999) * 4);
    return PlaySoundA(reinterpret_cast<LPCSTR>(waves_[static_cast<int>(cue)][level].data()),
                      nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT) != FALSE;
#else
    (void)cue;
    (void)volume;
    return false;
#endif
}

void SoundBank::stop()
{
#ifdef _WIN32
    PlaySoundA(nullptr, nullptr, 0);
#endif
}
} // namespace dice
