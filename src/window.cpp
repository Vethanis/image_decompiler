#include "window.h"

#include "myglheaders.h"
#include "framecounter.h"
#include <cassert>
#include <cstdio>

void error_callback(int error, const char* description)
{
    puts(description);
    assert(false);
}

int last_id = 0;

void APIENTRY error_callback_gl(GLenum source, 
                            GLenum type, 
                            GLuint id, 
                            GLenum severity, 
                            GLsizei length, 
                            const GLchar *message, 
                            void *userParam)
{
    //if(id == 131169 || id == 131185 || id == 131218 || id == 131204) 
        //return; 

    if( id == last_id)
    {
        return;
    }
    last_id = id;

    puts("---------------");
    printf("Debug message (%u): %s\n", id, message);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             puts("Source: API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   puts("Source: Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: puts("Source: Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     puts("Source: Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     puts("Source: Application"); break;
        case GL_DEBUG_SOURCE_OTHER:           puts("Source: Other"); break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               puts("Type: Error"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: puts("Type: Deprecated Behaviour"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  puts("Type: Undefined Behaviour"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         puts("Type: Portability"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         puts("Type: Performance"); break;
        case GL_DEBUG_TYPE_MARKER:              puts("Type: Marker"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          puts("Type: Push Group"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           puts("Type: Pop Group"); break;
        case GL_DEBUG_TYPE_OTHER:               puts("Type: Other"); break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         puts("Severity: high"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       puts("Severity: medium"); break;
        case GL_DEBUG_SEVERITY_LOW:          puts("Severity: low"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: puts("Severity: notification"); break;
    }
}

Window::Window(int width, int height, int major_ver, int minor_ver, const char* title){
    glfwSetErrorCallback(error_callback);
    assert(glfwInit());
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_ver);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_ver);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if DEBUG_OPENGL_CALLS
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    assert(window);
    glfwMakeContextCurrent(window);
    glewExperimental=true;
    assert(glewInit() == GLEW_OK);
    glGetError();    // invalid enumerant shows up here, just part of glew being itself.
    glfwSwapInterval(0);
    glViewport(0, 0, width, height);

#if DEBUG_OPENGL_CALLS
    {
        GLint flags; 
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags); DebugGL();
        assert(flags & GL_CONTEXT_FLAG_DEBUG_BIT);
        glEnable(GL_DEBUG_OUTPUT); DebugGL();
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);  DebugGL();
        glDebugMessageCallback((GLDEBUGPROC)error_callback_gl, nullptr); DebugGL();
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE); DebugGL();
    }
#endif
}

Window::~Window(){
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::open(){
    return !glfwWindowShouldClose(window);
}
void Window::swap(){
    glfwSwapBuffers(window);
    frameCompleted();
}
