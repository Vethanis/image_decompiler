#pragma once

struct GLFWwindow;

class Input
{
    static GLFWwindow* m_glwindow;

    static float m_scrollOffset, m_relScroll, m_cursorX, m_cursorY, m_relCursorX, m_relCursorY;

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static const int* downBegin();
    static const int* downEnd();
    static const int* upBegin();
    static const int* upEnd();
    static const int* activeBegin();
    static const int* activeEnd();
public:
    static void SetWindow(GLFWwindow* pWindow);
    static void Poll();
    static float scrollOffset();
    static float relScroll();
    static float cursorX();
    static float cursorY();
    static float relCursorX();
    static float relCursorY();

    struct DownKeys
    {
        const int* begin()
        {
            return Input::downBegin();
        }
        const int* end()
        {
            return Input::downEnd();
        }
    };
    static DownKeys GetDownKeys(){ return DownKeys(); }

    struct UpKeys
    {
        const int* begin()
        {
            return Input::upBegin();
        }
        const int* end()
        {
            return Input::upEnd();
        }
    };
    static UpKeys GetUpKeys(){ return UpKeys(); }

    struct ActiveKeys
    {
        const int* begin()
        {
            return Input::activeBegin();
        }
        const int* end()
        {
            return Input::activeEnd();
        }
    };
    static ActiveKeys GetActiveKeys(){ return ActiveKeys(); }
};