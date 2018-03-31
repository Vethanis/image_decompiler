#include "glscreen.h"
#include "myglheaders.h"
#include "debugmacro.h"
#include "shader.h"

namespace GLScreen 
{
    const char* shader_text = "\n\
    #version 450 core\n\
    layout(location = 0) in vec2 uv;\n\
    out vec2 fragUv;\n\
    void main(){\n\
        gl_Position = vec4(uv, 1.0, 1.0);\n\
        fragUv = uv * 0.5 + 0.5;\n\
    }\n\
    ";

    unsigned vertHandle = 0;
    unsigned vao=0, vbo=0;

    void init()
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        const GLfloat coords[] = 
        {
            -1.0f, -1.0f,
            3.0f, -1.0f,
            -1.0f,  3.0f
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(coords[0]) * 6, coords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        DebugGL();
    }

    int vertexShader()
    {
        if(!vertHandle)
            vertHandle = createShader(shader_text, GL_VERTEX_SHADER);

        return vertHandle;
    }

    void draw()
    {
        if(!vao)
            init();

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        DebugGL();
    }

}; // namespace GLScreen
