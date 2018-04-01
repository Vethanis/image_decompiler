#pragma once

#include <cmath>
#include <random>

inline float randf()
{
    return float(rand()) / float(RAND_MAX);
}

inline float randf2()
{
    return randf() * 2.0f - 1.0f;
}
