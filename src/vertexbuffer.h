#pragma once

#include "linmath.h"
#include "array.h"

struct Vertex
{
    vec4 position; // z -> radius
    vec4 color;

    static unsigned NumFloats()
    {
        return (sizeof(Vertex) / sizeof(float));
    }
};

typedef Vector<Vertex> VertexBuffer;

struct Geometry
{
    VertexBuffer m_vb;
};