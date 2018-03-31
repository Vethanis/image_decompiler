#include "shader.h"

#include "myglheaders.h"
#include "stdio.h"

unsigned createShader(const char* src, int type){
    unsigned handle = glCreateShader(type);  DebugGL();;
    glShaderSource(handle, 1, &src, NULL);  DebugGL();;
    glCompileShader(handle);  DebugGL();;

    int result = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &result);  DebugGL();;

    if(!result){
        int loglen = 0;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &loglen);  DebugGL();;
        char* log = new char[loglen + 1];
        glGetShaderInfoLog(handle, loglen, NULL, log);  DebugGL();;
        log[loglen] = 0;
        puts(log);
        puts(src);
        delete[] log;
    }

    return handle;
}

void deleteShader(unsigned id){
    glDeleteShader(id);  DebugGL();;
}
