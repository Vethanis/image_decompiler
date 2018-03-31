#include "myglheaders.h"
#include "SSBO.h"
#include "debugmacro.h"
#include "string.h"

void SSBO::init(unsigned binding){
    glGenBuffers(1, &id);
    DebugGL();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id);
    DebugGL();
}
void SSBO::deinit(){
    glDeleteBuffers(1, &id);
    DebugGL();
}
void SSBO::upload(const void* ptr, unsigned bytes){
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    DebugGL();
    glBufferData(GL_SHADER_STORAGE_BUFFER, bytes, ptr, GL_DYNAMIC_COPY);
    DebugGL();
}
void SSBO::download(void* dest, unsigned bytes){
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    DebugGL();
    void* src = glMapNamedBuffer(id, GL_DYNAMIC_COPY);
    memcpy(dest, src, bytes);
    glUnmapNamedBuffer(id);
    DebugGL();
}