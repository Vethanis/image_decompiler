#pragma once

#include "ints.h"
#include "vertexbuffer.h"

struct Mesh 
{
    u32 vao, vbo, num_indices;
    void draw();
    void upload(const VertexBuffer& vb);
    void init();
    void deinit();
};
