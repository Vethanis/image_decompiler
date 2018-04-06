#include "myglheaders.h"
#include "stdio.h"
#include "stdlib.h"
#include "debugmacro.h"
#include "window.h"
#include "input.h"
#include "randf.h"
#include <ctime>
#include "framecounter.h"
#include "lodepng.h"
#include "linmath.h"
#include "array.h"
#include "glprogram.h"
#include "texture.h"
#include "glscreen.h"
#include "framebuffer.h"
#include "vertexbuffer.h"
#include "mesh.h"

struct Renderer
{
    enum eConstants : int
    {
        NumChoices = 32,
    };

    Window m_window;
    GLProgram m_shader;
    GLProgram m_circleShader;
    GLProgram m_diffShader;
    Texture m_texture;
    Mesh m_mesh;

    Framebuffer m_framebuffers[NumChoices];
    Framebuffer m_diffbuffers[NumChoices];
    Vector<Vertex> m_vertices[NumChoices];

    const char* m_imageName=nullptr;
    double m_currentDifference = 0.0;

    unsigned m_width=0, m_height=0;
    unsigned m_frontFrame = 0;
    unsigned m_frameIdx = 0;
    unsigned m_secondsBetweenScreenshots = 60;
    unsigned m_framesPerPrimitive = 250;

    int m_maxPrimitives=0;
    int m_topMip=0;
    int m_imageId = 0;

    time_t m_lastScreenshot = 0;

    bool m_paused = false;
    bool m_showSource = false;
    bool m_viewDiff = false;

    Renderer(const Image& img, unsigned maj, unsigned min, const char* name, int max_prims, const char* imageName) : m_window(img.width, img.height, maj, min, name)
    {
        m_width = img.width;
        m_height = img.height;
        m_maxPrimitives = max_prims;
        m_imageName = imageName;
        m_topMip = (int)glm::floor(glm::log2(glm::max(float(m_width), float(m_height))));
        Input::SetWindow(m_window.getWindow());
        Input::Poll();
        const char* shaderfiles[] = {
            "screenVert.glsl",
            "screenFrag.glsl"
        };
        const char* circleFiles[] = {
            "circleVert.glsl",
            "circleFrag.glsl"
        };
        const char* diffFiles[] = {
            "screenVert.glsl",
            "diffFrag.glsl"
        };
        m_shader.setup(shaderfiles, 2);
        m_circleShader.setup(circleFiles, 2);
        m_diffShader.setup(diffFiles, 2);

        for(Framebuffer& frame : m_diffbuffers)
        {
            frame.init(m_width, m_height, 1);
        }
        for(Framebuffer& frame : m_framebuffers)
        {
            frame.init(m_width, m_height, 1);
        }
        m_mesh.init();

        glEnable(GL_BLEND); DebugGL();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); DebugGL();
        glViewport(0, 0, m_width, m_height); DebugGL();

        Reset();
        m_texture.Init(img);
        AddPrimitive(m_vertices[CurrentChoice()]);

        printf("Max primitives: %i\n", m_maxPrimitives);
    }
    ~Renderer()
    {
        for(auto& frame : m_framebuffers)
        {
            frame.deinit();
        }
        for(auto& frame : m_diffbuffers)
        {
            frame.deinit();
        }
        m_mesh.deinit();
        m_circleShader.deinit();
        m_shader.deinit();
        m_texture.deinit();
    }
    int CurrentChoice() { return m_frontFrame; }
    void SaveImage()
    {
        if((m_frameIdx & 1023) == 0)
        {
            time_t curTime = time(nullptr);
            time_t duration = curTime - m_lastScreenshot;
            if(duration > time_t(m_secondsBetweenScreenshots))
            {
                char filename[64] = { 0 };
                for(int i = 0; i < 63 && m_imageName[i]; ++i)
                {
                    if(m_imageName[i] == '.')
                        break;

                    filename[i] = m_imageName[i];
                }
                char buffer[128] = { 0 };
                snprintf(buffer, sizeof(buffer), "screenshots/%s_%04d_%04d.png", filename, m_imageId++, PrimitiveCount());
                m_framebuffers[CurrentChoice()].saveToFile(buffer);
                m_lastScreenshot = curTime;
            }
        }
    }
    void CommitChange(int idx, double diff)
    {
        m_frontFrame = idx;
        m_currentDifference = diff;
        if((m_frameIdx & 511) == 0)
        {
            printf("Difference: %f\n", m_currentDifference);
        }
    }
    int PrimitiveCount()
    {
        return m_vertices[CurrentChoice()].count() / 3;
    }
    void MakeRandomChange(Vector<Vertex>& vertices, int idx)
    {
        if(randu() & 1)
        {
            idx -= idx % 3;
            vec4 color = vertices[idx].color;
            float* comps = &color.x;
            float& chosen = comps[randu() % 3];
            chosen = glm::mix(chosen, randf(), randf());
            for(int i = 0; i < 3; ++i)
            {
                vertices[idx + i].color = color;
            }
        }
        else
        {
            vec4& pos = vertices[idx].position;
            pos.x = glm::mix(pos.x, 1.25f * randf2(), randf());
            pos.y = glm::mix(pos.y, 1.25f * randf2(), randf());
        }
    }
    void AddPrimitive(Vector<Vertex>& vertices)
    {
        vec2 center(randf2(), randf2());
        vec3 color(randf(), randf(), randf());
        const float len = 0.01f;
        for(int i = 0; i < 3; ++i)
        {
            Vertex& vertex = vertices.grow();
            vec2 pt = center + len * normalize(vec2(randf2(), randf2()));
            vertex.position.x = pt.x;
            vertex.position.y = pt.y;
            vertex.color = vec4(color, 1.0f);
        }
        printf("Primitive count: %i\n", PrimitiveCount());
    }
    void DrawIntoBuffer(const Vector<Vertex>& vertices, Framebuffer& destBuffer)
    {
        m_mesh.upload(vertices);
        destBuffer.bind();
        glViewport(0, 0, m_width, m_height); DebugGL();
        Framebuffer::clear();
        m_mesh.draw();
    }
    void DrawDifference(int idx)
    {
        m_diffbuffers[idx].bind();
        glViewport(0, 0, m_width, m_height); DebugGL();
        Framebuffer::clear();
        m_diffShader.bindTexture(1, m_texture.handle, "A");
        m_diffShader.bindTexture(2, m_framebuffers[idx].m_attachments[0], "B");
        GLScreen::draw();
    }
    double CalculateDifference(int idx)
    {
        vec4 dest[4];
        m_diffbuffers[idx].download(dest, 0, m_topMip);
        double diff = 0.0;
        for(vec4& v : dest)
        {
            diff += v.x + v.y + v.z;
        }
        return diff;
    }
    void Simulate()
    {
        if(m_paused)
        {
            return;
        }

        {
            m_circleShader.bind();
            int primIdx = randu() % m_vertices[0].count();
            for(int i = 0; i < NumChoices; ++i)
            {
                if(i != CurrentChoice())
                {
                    m_vertices[i] = m_vertices[CurrentChoice()];
                    MakeRandomChange(m_vertices[i], primIdx);
                }
                DrawIntoBuffer(m_vertices[i], m_framebuffers[i]);
            }
        }

        m_diffShader.bind();
        for(int i = 0; i < NumChoices; ++i)
        {
            DrawDifference(i);
        }
        Framebuffer::Barrier();
        double diffs[NumChoices];
        for(int i = 0; i < NumChoices; ++i)
        {
            diffs[i] = CalculateDifference(i);
        }

        int bestDiffIdx = CurrentChoice();
        double bestDiffVal = diffs[bestDiffIdx];
        for(int i = 0; i < NumChoices; ++i)
        {
            if(diffs[i] < bestDiffVal)
            {
                bestDiffIdx = i;
            }
        }

        CommitChange(bestDiffIdx, bestDiffVal);

        SaveImage();

        if((m_frameIdx % m_framesPerPrimitive) == 0)
        {
            if(PrimitiveCount() < m_maxPrimitives)
            {
                AddPrimitive(m_vertices[CurrentChoice()]);
            }
        }
        ++m_frameIdx;
    }
    void Reset()
    {
        for(auto& verts : m_vertices)
        {
            verts.clear();
        }
    }
    void TogglePauseSimulation()
    {
        m_paused = !m_paused;
    }
    void ToggleViewDiffBuffer()
    {
        m_viewDiff = !m_viewDiff;
    }
    void HandleInput()
    {
        for(int key : Input::GetDownKeys())
        {
            switch(key)
            {
                case GLFW_KEY_BACKSPACE:
                {
                    Reset();
                    break;
                }
                case GLFW_KEY_ENTER:
                {
                    AddPrimitive(m_vertices[CurrentChoice()]);
                    break;
                }
                case GLFW_KEY_SPACE:
                {
                    TogglePauseSimulation();
                    break;
                }
                case GLFW_KEY_F12:
                {
                    SaveImage();
                    break;
                }
                case GLFW_KEY_LEFT_ALT:
                case GLFW_KEY_RIGHT_ALT:
                {
                    m_showSource = !m_showSource;
                    break;
                }
                case GLFW_KEY_LEFT_SHIFT:
                case GLFW_KEY_RIGHT_SHIFT:
                {
                    ToggleViewDiffBuffer();
                    break;
                }
                default:
                break;
            }
        }
    }
    bool Swap()
    {
        Input::Poll();
        HandleInput();
        Simulate();
        m_shader.bind();
        m_shader.setUniformInt("flipped", 0);
        if(m_showSource)
        {
            m_shader.bindTexture(3, m_texture.handle, "current_frame");
        }
        else if(m_viewDiff)
        {
            const Framebuffer& src = m_diffbuffers[CurrentChoice()];
            m_shader.bindTexture(3, src.m_attachments[0], "current_frame");
            m_shader.setUniformInt("flipped", 1);
        }
        else
        {
            const Framebuffer& src = m_framebuffers[CurrentChoice()];
            m_shader.bindTexture(3, src.m_attachments[0], "current_frame");
        }
        m_shader.setUniformInt("seed", randu());
        Framebuffer::bindDefault();
        glViewport(0, 0, m_width, m_height); DebugGL();
        Framebuffer::clear();
        GLScreen::draw();
        m_window.swap();
        return m_window.open();
    }
};

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        puts("Missing source image argument");
        return 1;
    }
    if(argc < 3)
    {
        puts("Missing vertex count argument");
        return 1;
    }

    seedRandom();

    Image img;
    img.load(argv[1]);
    Renderer renderer(img, 4, 5, "Image Decompiler", atoi(argv[2]), argv[1]);
    img.free();

    while(renderer.Swap()){}

    return 0;
}
