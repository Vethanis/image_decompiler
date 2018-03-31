#include "framebuffer.h"
#include "myglheaders.h"
#include "lodepng.h"
#include "randf.h"
#include "linmath.h"

const GLenum attach_ids[Framebuffer::max_attachments] = { 
    GL_COLOR_ATTACHMENT0 + 0,
    GL_COLOR_ATTACHMENT0 + 1, 
    GL_COLOR_ATTACHMENT0 + 2,
    GL_COLOR_ATTACHMENT0 + 3,
    GL_COLOR_ATTACHMENT0 + 4,
    GL_COLOR_ATTACHMENT0 + 5,
    GL_COLOR_ATTACHMENT0 + 6,
    GL_COLOR_ATTACHMENT0 + 7
};

void Framebuffer::init(int width, int height, int num_attachments)
{
    m_width = width;
    m_height = height;

    assert(num_attachments <= max_attachments && num_attachments > 0);

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    m_attachments.resize(num_attachments);
    glGenTextures(num_attachments, m_attachments.begin());

    for(int i = 0; i < m_attachments.count(); ++i)
    {
        glBindTexture(GL_TEXTURE_2D, m_attachments[i]); DebugGL();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL); DebugGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); DebugGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); DebugGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    DebugGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    DebugGL();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_attachments[i], 0); DebugGL();
    }
    
    glDrawBuffers(num_attachments, attach_ids); DebugGL();

    glGenRenderbuffers(1, &m_rbo); DebugGL();
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo); DebugGL();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height); DebugGL();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo); DebugGL();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        puts("Framebuffer not complete!");
        assert(false);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); DebugGL();
}

void Framebuffer::deinit()
{
    glDeleteRenderbuffers(1, &m_rbo); DebugGL();
    glDeleteTextures(m_attachments.count(), m_attachments.begin()); DebugGL();
    glDeleteFramebuffers(1, &m_fbo); DebugGL();
}

void Framebuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo); DebugGL();
}

void Framebuffer::download(vec4* texels, int attachment)
{
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT); DebugGL();
    glBindTexture(GL_TEXTURE_2D, m_attachments[attachment]); DebugGL();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, (float*)texels); DebugGL();
}

void Framebuffer::download(vec4* texels, int attachment, int mipLevel)
{
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT); DebugGL();
    glBindTexture(GL_TEXTURE_2D, m_attachments[attachment]); DebugGL();
    glGenerateMipmap(GL_TEXTURE_2D); DebugGL();
    glGetTexImage(GL_TEXTURE_2D, mipLevel, GL_RGBA, GL_FLOAT, (float*)texels); DebugGL();
}

void Framebuffer::saveToFile(const char* filename, int attachment)
{
    const int num_elems = m_width * m_height * 4;
    vec4* texels = new vec4[m_width * m_height];
    unsigned char* image = new unsigned char[num_elems];

    download(texels, attachment);

    for(int i = 0; i < m_width * m_height; ++i)
    {
        vec4 val = texels[i];
        val += vec4(randf(), randf(), randf(), 1.0f) * 0.001f;
        val *= 255.0f;
        val = glm::clamp(val, 0.0f, 255.0f);

        image[i * 4 + 0] = (unsigned char)val.x;
        image[i * 4 + 1] = (unsigned char)val.y;
        image[i * 4 + 2] = (unsigned char)val.z;
        image[i * 4 + 3] = 255;
    }

    unsigned error = lodepng_encode32_file(filename, image, m_width, m_height);
    if(error)
    {
        puts(lodepng_error_text(error));
    }

    delete[] image;
    delete[] texels;
}

void Framebuffer::bindDefault()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0); DebugGL();
}

void Framebuffer::clearDepth()
{
    glClear(GL_DEPTH_BUFFER_BIT); DebugGL();
}

void Framebuffer::clearColor()
{
    glClear(GL_COLOR_BUFFER_BIT) DebugGL();
}

void Framebuffer::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); DebugGL();
}