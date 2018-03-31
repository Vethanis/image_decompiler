#include "texture.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

void Image::load(const char* filename)
{
    int channels = 0;
    image = stbi_load(filename, &width, &height, &channels, 4);
    assert(image);
}

void Image::free()
{
    stbi_image_free(image);
    image = nullptr;
}

