#pragma once

#include <cstdio>

#define PLM() printf("%s :: %d\n", __FILE__, __LINE__)

#define DEBUG_OPENGL_CALLS 0

#if DEBUG_OPENGL_CALLS

#define GL_DEBUG_BREAK() __debugbreak()

#define DebugGL(); {    \
    GLenum err = GL_NO_ERROR; \
    while((err = glGetError()) != GL_NO_ERROR){ \
        switch(err){    \
            case 0x500:    \
                puts("[OpenGL] Invalid enumeration.");  \
                PLM();  \
                GL_DEBUG_BREAK(); \
                break;  \
            case 0x501: \
                puts("[OpenGL] Invalid value.");    \
                PLM();  \
                GL_DEBUG_BREAK(); \
                break;  \
            case 0x502: \
                puts("[OpenGL] Invalid operation.");    \
                PLM();  \
                GL_DEBUG_BREAK(); \
                break;  \
            case 0x503: \
                puts("[OpenGL] Stack overflow.");   \
                PLM();  \
                GL_DEBUG_BREAK(); \
                break;  \
            case 0x504: \
                puts("[OpenGL] Stack underflow.");  \
                PLM();  \
                GL_DEBUG_BREAK(); \
                break;  \
            case 0x505: \
                puts("[OpenGL] Out of memory.");    \
                PLM();  \
                GL_DEBUG_BREAK(); \
                break;  \
            case 0x506: \
                puts("[OpenGL] Invalid framebuffer operation.");    \
                PLM();  \
                GL_DEBUG_BREAK(); \
                break;  \
            case 0x507: \
                puts("[OpenGL] Context lost."); \
                PLM();  \
                GL_DEBUG_BREAK(); \
                break;  \
            case 0x508: \
                puts("[OpenGL] Table too large.");  \
                PLM();  \
                GL_DEBUG_BREAK(); \
                break;  \
        }   \
    }   \
}

#else

#define DebugGL() 

#endif // DEBUG_OPENGL_CALLS

#define GLM_PRINT_FUNCTIONS 0

#if GLM_PRINT_FUNCTIONS

#include "glm/glm.hpp"

inline void print(const glm::vec2& v){
    printf("(%.3f, %.3f)\n", v.x, v.y);
}

inline void print(const glm::vec3& v){
    printf("(%.3f, %.3f, %.3f)\n", v.x, v.y, v.z);
}

inline void print(const glm::vec4& v){
    printf("(%.3f, %.3f, %.3f, %.3f)\n", v.x, v.y, v.z, v.w);
}

inline void print(const glm::mat3& m){
    print(m[0]);
    print(m[1]);
    print(m[2]);
}

inline void print(const glm::mat4& m){
    print(m[0]);
    print(m[1]);
    print(m[2]);
    print(m[3]);
}

#endif // GLM_PRINT_FUNCTIONS
