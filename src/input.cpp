#include "input.h"

#include "myglheaders.h"
#include "array.h"

GLFWwindow* Input::m_glwindow = nullptr;
float Input::m_scrollOffset = 0.0f;
float Input::m_relScroll = 0.0f;
float Input::m_cursorX = 0.0f;
float Input::m_cursorY = 0.0f;
float Input::m_relCursorX = 0.0f;
float Input::m_relCursorY = 0.0f;

Array<int, 64> g_downKeys;
Array<int, 64> g_upKeys;
Array<int, 64> g_activeKeys;

void Input::SetWindow(GLFWwindow* window)
{
    m_glwindow = window;
    //glfwSetInputMode(m_glwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(m_glwindow, key_callback);
    glfwSetCursorPosCallback(m_glwindow, cursor_position_callback);
    glfwSetMouseButtonCallback(m_glwindow, mouse_button_callback);
    glfwSetScrollCallback(m_glwindow, scroll_callback);
}

void Input::Poll()
{
    g_downKeys.clear();
    g_upKeys.clear();
    m_relCursorX = 0.0f;
    m_relCursorY = 0.0f;
    glfwPollEvents();
}

float Input::scrollOffset(){
    return Input::m_scrollOffset;
}

float Input::relScroll(){
    return Input::m_relScroll;
}

float Input::cursorX(){
    return Input::m_cursorX;
}

float Input::cursorY(){
    return Input::m_cursorY;
}

float Input::relCursorX(){
    return Input::m_relCursorX;
}

float Input::relCursorY(){
    return Input::m_relCursorY;
}

void Input::mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    if(action == GLFW_PRESS){
        g_downKeys.grow() = button;
        g_activeKeys.uniquePush(button);
    }
    else if(action == GLFW_RELEASE){
        g_upKeys.grow() = button;
        g_activeKeys.findRemove(button);
    }
}

void Input::cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
    m_relCursorX = m_cursorX - (float)xpos;
    m_relCursorY = m_cursorY - (float)ypos;
    m_cursorX = (float) xpos;
    m_cursorY = (float) ypos;
}

void Input::scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    m_relScroll = (float)yoffset - m_scrollOffset;
    m_scrollOffset = (float)yoffset;
}

void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(action == GLFW_PRESS){
        g_downKeys.grow() = key;
        g_activeKeys.uniquePush(key);
    }
    else if(action == GLFW_RELEASE){
        g_upKeys.grow() = key;
        g_activeKeys.findRemove(key);
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

const int* Input::downBegin(){
    return g_downKeys.begin();
}
const int* Input::downEnd(){
    return g_downKeys.end();
}
const int* Input::upBegin(){
    return g_upKeys.begin();
}
const int* Input::upEnd(){
    return g_upKeys.end();
}
const int* Input::activeBegin(){
    return g_activeKeys.begin();
}
const int* Input::activeEnd(){
    return g_activeKeys.end();
}