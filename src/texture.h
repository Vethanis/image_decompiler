#pragma once

#include "myglheaders.h"
#include "linmath.h"
#include "debugmacro.h"

struct Image
{
    unsigned char* image;
    int width, height;
    bool mip;
    Image() : image(nullptr), width(0), height(0), mip(true){}
    ~Image()
    {
        free();
    }
    void load(const char* name);
    void free();
};

struct Texture
{
    unsigned handle = 0;

    struct parameter
    {
        const void* ptr;
        int FullType; 
        int Channels; 
        int ComponentType; 
        int width;
        int height;
        bool mip;
    };

    void init(const parameter& p){
        assert(handle == 0);
        glGenTextures(1, &handle);  DebugGL();
        glBindTexture(GL_TEXTURE_2D, handle);  DebugGL();
        if(p.mip){
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    DebugGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);    DebugGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);    DebugGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    DebugGL();
        }
        else{
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    DebugGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);    DebugGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    DebugGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);    DebugGL();
        }
        glTexImage2D(GL_TEXTURE_2D, 0, p.FullType, p.width, p.height, 0, p.Channels, p.ComponentType, p.ptr);    DebugGL();        
        if(p.mip)
            glGenerateMipmap(GL_TEXTURE_2D);    DebugGL();
    }
    void deinit(){
        assert(handle != 0);
        glDeleteTextures(1, &handle);    DebugGL();
    }
    void upload(const parameter& p){
        assert(handle != 0);
        glBindTexture(GL_TEXTURE_2D, handle);  DebugGL();
        glTexImage2D(GL_TEXTURE_2D, 0, p.FullType, p.width, p.height, 0, p.Channels, p.ComponentType, p.ptr);  DebugGL();
        if(p.mip)
            glGenerateMipmap(GL_TEXTURE_2D);    DebugGL();
    }
    void uploadPortion(int level, int x, int y, int w, int h, int format, int type, const void* p){
        glTextureSubImage2D(handle, level, x, y, w, h, format, type, p);
    }
    void bind(int channel){
        glActiveTexture(GL_TEXTURE0 + channel);  DebugGL();
        glBindTexture(GL_TEXTURE_2D, handle);  DebugGL();
    }
    void setCSBinding(int FullType, int binding){
        glBindImageTexture(binding, handle, 0, GL_FALSE, 0, GL_READ_WRITE, FullType);  DebugGL();
    }

#define TEX_TYPE_MACRO(name, a, b, c) \
    void init##name(int w, int h, bool mip, const void* ptr){ \
        parameter p = { ptr, a, b, c, w, h, mip }; \
        init(p); \
    }\
    void upload##name(int w, int h, bool mip, const void* ptr){ \
        parameter p = { ptr, a, b, c, w, h, mip }; \
        upload(p); \
    }

    TEX_TYPE_MACRO(1f, GL_R32F, GL_RED, GL_FLOAT);
    TEX_TYPE_MACRO(2f, GL_RG32F, GL_RG, GL_FLOAT);
    TEX_TYPE_MACRO(4f, GL_RGBA32F, GL_RGBA, GL_FLOAT);

    TEX_TYPE_MACRO(1i, GL_R32I, GL_RED, GL_INT);
    TEX_TYPE_MACRO(2i, GL_RG32I, GL_RG, GL_INT);
    TEX_TYPE_MACRO(4i, GL_RGBA32I, GL_RGBA, GL_INT);

    TEX_TYPE_MACRO(1uc, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
    TEX_TYPE_MACRO(2uc, GL_RG8, GL_RG, GL_UNSIGNED_BYTE);
    TEX_TYPE_MACRO(4uc, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);

    TEX_TYPE_MACRO(1us, GL_R16, GL_RED, GL_UNSIGNED_SHORT);
    TEX_TYPE_MACRO(2us, GL_RG16, GL_RG, GL_UNSIGNED_SHORT);
    TEX_TYPE_MACRO(4us, GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT);
    
    void Upload(const Image& image)
    {
        upload4uc(image.width, image.height, image.mip, image.image);
    }
    void Init(const Image& image)
    {
        init4uc(image.width, image.height, image.mip, image.image);
    }
};


