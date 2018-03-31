#pragma once

#include "glm/glm.hpp"
#include "store.h"
#include "hashstring.h"

struct GLProgram{
    unsigned m_id;
    Store<int, 32> locations;
    void init();
    void deinit();
    int addShader(const char* path, int type);
    void addShader(unsigned handle);
    void freeShader(unsigned handle);
    void link();
    void bind();
    int getUniformLocation(HashString name);
    void setUniform(int loc, const glm::vec2& v);
    void setUniform(int loc, const glm::vec3& v);
    void setUniform(int loc, const glm::vec4& v);
    void setUniform(int loc, const glm::mat3& v);
    void setUniform(int loc, const glm::mat4& v);
    void setUniformInt(int loc, int v);
    void setUniformFloat(int loc, float v);
    void bindTexture(int channel, int texture, const char* name);
    void bindCubemap(int channel, int texture, const char* name);
    void computeCall(int x=0, int y=0, int z=0);
    void setup(const char** filenames, int count);
    template<typename T>
    void setUniform(const char* name, const T& t){
        int loc = getUniformLocation(HashString(name));
        setUniform(loc, t);
    }
    void setUniformInt(const char* name, int v){
        int loc = getUniformLocation(HashString(name));
        setUniformInt(loc, v);
    }
    void setUniformFloat(const char* name, float v){
        int loc = getUniformLocation(HashString(name));
        setUniformFloat(loc, v);
    }
};
