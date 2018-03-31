#pragma once

#include "array.h"

void load_file(const char* path, Vector<char>& contents);

inline const char* nextline(const char* p)
{
    for(; p[0] && p[0] != '\n'; p++){};
    if(p[0] == '\n'){
        p++;
    }
    return p;
}
