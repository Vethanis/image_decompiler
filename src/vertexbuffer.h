#pragma once

#include "linmath.h"
#include "array.h"

struct Vertex
{
    vec4 position; // zw -> uv
    vec4 color;
};

typedef Vector<Vertex> VertexBuffer;

struct Geometry
{
    VertexBuffer m_vb;
};