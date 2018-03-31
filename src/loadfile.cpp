
#define _CRT_SECURE_NO_WARNINGS

#include "loadfile.h"
#include "stdio.h"
#include <cassert>

void load_file(const char* path, Vector<char>& contents)
{
    contents.clear();
    FILE* f = fopen(path, "rb");
    if(!f)
        return;
    
    fseek(f, 0, SEEK_END);
    const int sz = ftell(f);
    contents.resize(sz + 1);
    rewind(f);
    assert(fread(contents.begin(), sz, 1, f) == 1);
    fclose(f);
    contents[sz] = 0;
}

