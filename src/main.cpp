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
#include <thread>

void FpsStats()
{
    static double average_dt = 0.0;
    average_dt += frameSeconds();

    if((frameCounter() & 63) == 0)
    {
        average_dt /= 64.0;
        const double ms = average_dt * 1000.0;
        printf("ms: %.6f, FPS: %.3f\n", ms, 1.0 / average_dt);
        average_dt = 0.0;
    }
}

struct Renderer
{
    enum eConstants : int
    {
        NumChoices = 16,
    };

    Window m_window;
    GLProgram m_shader;
    GLProgram m_circleShader;
    GLProgram m_diffShader;
    Texture m_texture;
    Mesh m_mesh;
    Framebuffer m_diffFramebuffer;
    vec4* m_sourceImage;

    Framebuffer m_framebuffers[NumChoices];
    Vector<Vertex> m_vertices[NumChoices];

    unsigned m_width, m_height;
    unsigned m_frontFrame = 0;
    unsigned m_framesPerAdd = 100;
    unsigned m_frameIdx = 0;
    int m_maxPrimitives;
    int m_topMip;

    bool m_paused = false;
    bool m_viewFront = false;
    bool m_showSource = false;

    Renderer(const Image& img, unsigned maj, unsigned min, const char* name, int max_prims) : m_window(img.width, img.height, maj, min, name)
    {
        m_width = img.width;
        m_height = img.height;
        m_maxPrimitives = max_prims;
        printf("Max primitives: %i\n", m_maxPrimitives);
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

        m_sourceImage = new vec4[m_width * m_height];
        m_diffFramebuffer.init(m_width, m_height, 1);
        for(Framebuffer& frame : m_framebuffers)
        {
            frame.init(m_width, m_height, 1);
        }
        m_mesh.init();

        glClearColor(0.5f, 0.5f, 0.5f, 0.0f); DebugGL();
        glEnable(GL_BLEND); DebugGL();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); DebugGL();

        Reset();
        if(img.image)
        {
            constexpr unsigned num_components = 4;
            const unsigned numTexels = m_width * m_height;
            for(unsigned i = 0; i < numTexels; ++i)
            {
                float* dst = &m_sourceImage[i].x;
                const unsigned char* pSrc = &img.image[i * num_components];
                for(unsigned j = 0; j < num_components; ++j)
                {
                    dst[j] = pSrc[j] / 255.0f + randf() * (1.0f / 512.0f);
                    dst[j] = glm::clamp(dst[j], 0.0f, 1.0f);
                }
                dst[num_components - 1] = 1.0f;
            }
        }
        m_texture.Init(img);
        AddPrimitive(m_vertices[CurrentChoice()]);
    }
    ~Renderer()
    {
        m_diffFramebuffer.deinit();
        for(auto& frame : m_framebuffers)
        {
            frame.deinit();
        }
        m_mesh.deinit();
        m_circleShader.deinit();
        m_shader.deinit();
        m_diffFramebuffer.deinit();
        m_texture.deinit();

        delete[] m_sourceImage;
    }
    int CurrentChoice() { return m_frontFrame; }
    void SaveImage()
    {
        char buffer[64] = {0};
        snprintf(buffer, 64, "screenshots/choice_%d.png", m_frontFrame);
        m_framebuffers[CurrentChoice()].saveToFile(buffer);
    }
    void CommitChange(int idx)
    {
        m_frontFrame = idx;
    }
    int PrimitiveCount()
    {
        return m_vertices[m_frontFrame].count() / 3;
    }
    void MakeRandomChange(Vector<Vertex>& vertices)
    {
        const float alpha = 0.25f * randf();
        float fpos = randf();
        fpos = fpos * fpos;
        fpos = 1.0f - fpos;
        fpos = glm::clamp(fpos * float(PrimitiveCount()), 0.0f, float(PrimitiveCount() - 1));
        const int primIdx = 3 * int(fpos);
        const bool mutateColor = rand(g_randSeed) & 1;
        if(mutateColor)
        {
            vec4 color = glm::mix(vertices[primIdx].color, vec4(randf(), randf(), randf(), 1.0f), alpha);
            for(int i = 0; i < 3; ++i)
            {
                vertices[primIdx + i].color = color;
            }
        }
        else
        {
            Vertex& vert = vertices[rand(g_randSeed) % vertices.count()];
            vert.position.x = glm::mix(vert.position.x, randf(), alpha);
            vert.position.y = glm::mix(vert.position.y, randf(), alpha);
        }
    }
    void AddPrimitive(Vector<Vertex>& vertices)
    {
        vec2 center(randf2(), randf2());
        vec3 color(randf(), randf(), randf());
        const float len = 0.2f;
        for(int i = 0; i < 3; ++i)
        {
            Vertex& vertex = vertices.grow();
            vec2 pt = center + len * normalize(vec2(randf2(), randf2()));
            vertex.position.x = pt.x;
            vertex.position.y = pt.y;
            vertex.color = vec4(color, 1.0f);
        }
        printf("Primitive count: %i\n", PrimitiveCount());

        if(PrimitiveCount() % 10 == 0)
        {
            SaveImage();
        }
    }
    void DrawIntoBuffer(const Vector<Vertex>& vertices, Framebuffer& destBuffer)
    {
        m_mesh.upload(vertices);
        destBuffer.bind();
        glViewport(0, 0, m_width, m_height); DebugGL();
        Framebuffer::clear();
        m_mesh.draw();
    }
    double CalculateDifference(Framebuffer& buffer)
    {
        Framebuffer::clear();
        m_diffShader.bindTexture(1, m_texture.handle, "A");
        m_diffShader.bindTexture(2, buffer.m_attachments[0], "B");
        GLScreen::draw();
        vec4 dest[4*4];
        m_diffFramebuffer.download(dest, 0, m_topMip);
        double diff = 0.0;
        for(vec4& v : dest)
        {
            diff += v.x;
        }
        return diff;
    }
    void Simulate()
    {
        if(m_paused)
        {
            return;
        }
        m_circleShader.bind();
        double diffs[NumChoices];

        for(int i = 0; i < NumChoices; ++i)
        {
            if(i != CurrentChoice())
            {
                m_vertices[i] = m_vertices[CurrentChoice()];
                MakeRandomChange(m_vertices[i]);
            }
            DrawIntoBuffer(m_vertices[i], m_framebuffers[i]);
        }

        m_diffShader.bind();
        m_diffFramebuffer.bind();
        glViewport(0, 0, m_width, m_height); DebugGL();
        for(int i = 0; i < NumChoices; ++i)
        {
            diffs[i] = CalculateDifference(m_framebuffers[i]);
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

        CommitChange(bestDiffIdx);

        ++m_frameIdx;
        if(m_frameIdx >= m_framesPerAdd && m_vertices[CurrentChoice()].count() < m_maxPrimitives * 3)
        {
            AddPrimitive(m_vertices[CurrentChoice()]);
            m_frameIdx = 0;
        }
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
    void ToggleViewFrontBuffer()
    {
        m_viewFront = !m_viewFront;
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
                case GLFW_KEY_LEFT_SHIFT:
                case GLFW_KEY_RIGHT_SHIFT:
                {
                    ToggleViewFrontBuffer();
                    break;
                }
                case GLFW_KEY_LEFT_ALT:
                case GLFW_KEY_RIGHT_ALT:
                {
                    m_showSource = !m_showSource;
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
        if(m_showSource)
        {
            m_shader.bindTexture(3, m_texture.handle, "current_frame");
        }
        else
        {
            const Framebuffer& src = m_framebuffers[CurrentChoice()];
            m_shader.bindTexture(3, src.m_attachments[0], "current_frame");
        }
        m_shader.setUniformInt("seed", rand(g_randSeed));
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
    g_randSeed = (unsigned)time(NULL);
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

    Image img;
    img.load(argv[1]);
    Renderer renderer(img, 4, 5, "Image Decompiler", atoi(argv[2]));
    img.free();

    while(renderer.Swap()){}

    return 0;
}
