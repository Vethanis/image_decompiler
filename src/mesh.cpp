
#define _CRT_SECURE_NO_WARNINGS

#include "myglheaders.h"
#include "mesh.h"
#include "debugmacro.h"
#include "vertexbuffer.h"

template<typename T>
void mesh_layout(int location);

static size_t offset_loc = 0;
static size_t stride = 0;

template<typename T>
void begin_mesh_layout(){
    offset_loc = 0;
    stride = sizeof(T);
}

template<>
inline void mesh_layout<glm::vec2>(int location){
    glEnableVertexAttribArray(location); DebugGL();;
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, (int)stride, (void*)offset_loc); DebugGL();;
    offset_loc += sizeof(glm::vec2);
}
template<>
inline void mesh_layout<glm::vec3>(int location){
    glEnableVertexAttribArray(location); DebugGL();;
    glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, (int)stride, (void*)offset_loc); DebugGL();;
    offset_loc += sizeof(glm::vec3);
}
template<>
inline void mesh_layout<glm::vec4>(int location){
    glEnableVertexAttribArray(location); DebugGL();;
    glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, (int)stride, (void*)offset_loc); DebugGL();;
    offset_loc += sizeof(glm::vec4);
}
template<>
inline void mesh_layout<unsigned>(int location){
    glEnableVertexAttribArray(location); DebugGL();
    glVertexAttribPointer(location, 1, GL_UNSIGNED_INT, GL_FALSE, (int)stride, (void*)offset_loc); DebugGL();
    offset_loc += sizeof(unsigned);
}

void Mesh::init(){
    num_indices = 0;
    glGenVertexArrays(1, &vao); DebugGL();;
    glGenBuffers(1, &vbo); DebugGL();;

    glBindVertexArray(vao); DebugGL();;
    glBindBuffer(GL_ARRAY_BUFFER, vbo); DebugGL();

    begin_mesh_layout<Vertex>();
    mesh_layout<glm::vec4>(0);
    mesh_layout<glm::vec4>(1);
}

void Mesh::deinit(){
    glDeleteBuffers(1, &vbo); DebugGL();;
    glDeleteVertexArrays(1, &vao); DebugGL();;
    DebugGL();
}

void Mesh::upload(const VertexBuffer& vb){
    glBindVertexArray(vao); DebugGL();

    num_indices = vb.count();

    glBindBuffer(GL_ARRAY_BUFFER, vbo); DebugGL();
    glBufferData(GL_ARRAY_BUFFER, vb.bytes(), 
        vb.begin(), GL_STATIC_DRAW); DebugGL();
}

void Mesh::draw(){
    if(!num_indices)
        return;

    glBindVertexArray(vao); DebugGL();
    glDrawArrays(GL_TRIANGLES, 0, num_indices); DebugGL();
}
