#pragma once

void seedRandom();

unsigned randu();

float randf();

inline float randf2()
{
    return randf() * 2.0f - 1.0f;
}
