#include "myglheaders.h"
#include "UBO.h"
#include "debugmacro.h"

static unsigned binding_tail = 0;

void UBO::init(const void* ptr, unsigned size, const char* name, const unsigned* programs, int num_progs){
    glGenBuffers(1, &id);DebugGL();
    glBindBuffer(GL_UNIFORM_BUFFER, id);DebugGL();
    glBufferData(GL_UNIFORM_BUFFER, size, ptr, GL_STATIC_DRAW);DebugGL();

    for(int i = 0; i < num_progs; i++){
        unsigned idx = glGetUniformBlockIndex(programs[i], name);DebugGL();
        glUniformBlockBinding(programs[i], idx, binding_tail);DebugGL();
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, binding_tail, id);DebugGL();
    
    binding_tail++;
}
void UBO::deinit(){
    glDeleteBuffers(1, &id);DebugGL();
}
void UBO::upload(const void* ptr, unsigned size){
    glBindBuffer(GL_UNIFORM_BUFFER, id);DebugGL();
    glBufferData(GL_UNIFORM_BUFFER, size, ptr, GL_STATIC_DRAW);DebugGL();
}
