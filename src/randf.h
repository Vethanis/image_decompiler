#pragma once

#include <cmath>

extern unsigned g_randSeed;

inline unsigned rand(unsigned& s = g_randSeed)
{
    unsigned f = s;
    f = (f ^ 61) ^ (f >> 16);
    f *= 9;
    f = f ^ (f >> 4);
    f *= 0x27d4eb2d;
    f = f ^ (f >> 15);
    s = f;
    return f;
}

inline float randf(unsigned& s = g_randSeed)
{
    constexpr float inv = 1.0f / float(0xffffffff);
    return float(rand(s)) * inv;
}

inline float randf2(unsigned& s = g_randSeed)
{
    return randf(s) * 2.0f - 1.0f;
}
