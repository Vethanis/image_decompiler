#include "myglheaders.h"

#include "glprogram.h"

#include "shader.h"
#include "loadfile.h"
#include "debugmacro.h"
#include "glm/gtc/type_ptr.hpp"
#include "stdio.h"

void GLProgram::init(){
    m_id = glCreateProgram();  DebugGL();;
}

void GLProgram::deinit(){
    glDeleteProgram(m_id); DebugGL();;
}

int GLProgram::addShader(const char* path, int type){
    Vector<char> src;
    load_file(path, src);
    assert(src.count());
    unsigned handle = createShader(src.begin(), type);
    glAttachShader(m_id, handle);  DebugGL();;
    return handle;
}

void GLProgram::addShader(unsigned handle){
    glAttachShader(m_id, handle);  DebugGL();;
}

void GLProgram::freeShader(unsigned handle){
    glDetachShader(m_id, handle);  DebugGL();;
    glDeleteShader(handle);  DebugGL();;
}

void GLProgram::link(){
    glLinkProgram(m_id);  DebugGL();;

    int result = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, &result);
    if(!result){
        int loglen = 0;
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &loglen);  DebugGL();;
        char* log = new char[loglen + 1];
        glGetProgramInfoLog(m_id, loglen, NULL, log);  DebugGL();;
        log[loglen] = 0;
        puts(log);
        delete[] log;
    }
}

void GLProgram::bind(){
    glUseProgram(m_id);  DebugGL();;
}

int GLProgram::getUniformLocation(HashString hash){
    int* pLoc = locations[hash.m_hash];
    if(!pLoc){
        const char* loc_str = hash.str();
        assert(loc_str);
        locations.insert(hash.m_hash, glGetUniformLocation(m_id, loc_str));
        pLoc = locations[hash.m_hash];
        if(*pLoc == -1){
            printf("[GLProgram] Invalid uniform detected: %s\n", loc_str);
            //assert(false);
        }
    }
    return *pLoc;
}

void GLProgram::setUniform(int location, const glm::vec2& v){
    glUniform2fv(location, 1, glm::value_ptr(v));  DebugGL();;
}
void GLProgram::setUniform(int location, const glm::vec3& v){
    glUniform3fv(location, 1, glm::value_ptr(v));  DebugGL();;
}
void GLProgram::setUniform(int location, const glm::vec4& v){
    glUniform4fv(location, 1, glm::value_ptr(v));  DebugGL();;
}
void GLProgram::setUniform(int location, const glm::mat3& v){
    glUniformMatrix3fv(location, 1, false, glm::value_ptr(v));  DebugGL();;
}
void GLProgram::setUniform(int location, const glm::mat4& v){
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(v));  DebugGL();;
}
void GLProgram::setUniformInt(int location, int v){
    glUniform1i(location, v);  DebugGL();;
}
void GLProgram::setUniformFloat(int location, float v){
    glUniform1f(location, v);  DebugGL();;
}

void GLProgram::computeCall(int x, int y, int z){
    glDispatchCompute(x, y, z);
}

void GLProgram::setup(const char** filenames, int count){
    static const int sequences[] = {
        GL_COMPUTE_SHADER, 
        GL_VERTEX_SHADER, 
        GL_FRAGMENT_SHADER, 
        GL_VERTEX_SHADER, 
        GL_GEOMETRY_SHADER,
        GL_FRAGMENT_SHADER
    };
    assert(count > 0 && count < 4);
    int names[] = {0, 0, 0};
    init();
    int begin = 0;
    if(count == 2){
        begin = 1;
    }
    else if(count == 3){
        begin = 3;
    }
    for(int i = 0; i < count; ++i){
        names[i] = addShader(filenames[i], sequences[begin + i]);
    }
    link();
    for(int i = 0; i < count; ++i){
        freeShader(names[i]);
    }
}


void GLProgram::bindTexture(int channel, int texture, const char* name)
{
    glActiveTexture(GL_TEXTURE0 + channel);  DebugGL();
    glBindTexture(GL_TEXTURE_2D, texture);  DebugGL();
    setUniformInt(name, channel);
}

void GLProgram::bindCubemap(int channel, int texture, const char* name)
{
    glActiveTexture(GL_TEXTURE0 + channel);  DebugGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);  DebugGL();
    setUniformInt(name, channel);
}