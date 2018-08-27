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
#include <string.h>

#ifdef DB_EXT
#define DB_EXT_ONLY(...) __VA_ARGS__
#else 
#define DB_EXT_ONLY(...) 
#endif // DB_EXT

DB_EXT_ONLY(#include "db.h")

#include <glm/gtx/matrix_transform_2d.hpp>

#define NELEM(x) (sizeof(x) / sizeof(x[0]))

struct sSettings
{
    const char* m_imageName = "example.png";
    const char* m_brushName = "brush.png";
    const char* m_windowName = "Image Decompiler";
    unsigned m_glMajor = 4;
    unsigned m_glMinor = 5;
    unsigned m_width = 0, m_height = 0;
    unsigned m_secondsPerScreenshot = 60 * 3;
    unsigned m_framesPerPrimitive = 250;
    int m_maxPrimitives = 1000;
    float m_primAlpha = 0.75f;
};

struct Renderer
{
    enum eConstants : int
    {
        NumChoices = 8,
        VerticesPerPrimitive = 6,
    };

    Window m_window;
    GLProgram m_shader;
    GLProgram m_primShader;
    GLProgram m_diffShader;
    Texture m_texture;
    Texture m_brushTex;
    Mesh m_mesh;

    DB_EXT_ONLY(
        DB::Context m_db;
        DB::ID m_runId;
    );

    static const vec4 m_square[VerticesPerPrimitive];

    Framebuffer m_framebuffers[NumChoices];
    Framebuffer m_diffbuffers[NumChoices];
    VertexBuffer m_vertices[NumChoices];

    struct PrimTransform
    {
        vec2 translation = vec2(0.0f);
        vec2 scale = vec2(0.01f);
        float rotation = 0.0f;

        void Randomize()
        {
            switch(randu() % 3)
            {
                case 0: 
                {
                    vec2 nTranslation = translation;
                    *(&nTranslation.x + randu() % 2) = 1.1f * randf2();
                    translation = glm::mix(translation, nTranslation, randf());
                }
                break;
                case 1: 
                {
                    vec2 nScale = scale;
                    *(&nScale.x + randu() % 2) = randf();
                    scale = glm::mix(scale, nScale, randf());
                }
                break;
                case 2: 
                {
                    float nTranslation = randf() * 3.141592f * 2.0f;
                    rotation = glm::mix(rotation, nTranslation, randf());
                };
                break;
            }
        }
        void Transform(Vertex* pVerts) const
        {
            const mat3 transform = glm::translate(mat3(), translation) * glm::rotate(mat3(), rotation) * glm::scale(mat3(), scale);
            for(int i = 0; i < VerticesPerPrimitive; ++i)
            {
                const vec4& sqr = m_square[i];
                const vec3 pos = transform * vec3(sqr.x, sqr.y, 1.0f);
                pVerts[i].position = vec4(pos.x, pos.y, sqr.z, sqr.w);
            }
        }
    };
    typedef Vector<PrimTransform> TransformBuffer;
    TransformBuffer m_transforms[NumChoices];

    sSettings m_settings;

    double m_currentDifference = 0.0;

    unsigned m_frontFrame = 0;
    unsigned m_frameIdx = 0;
    int m_topMip=0;
    int m_imageId = 0;

    time_t m_lastScreenshot = 0;

    bool m_paused = false;
    bool m_showSource = false;
    bool m_viewDiff = false;

    void init(const sSettings& settings)
    {

        m_settings = settings;
        m_window.init(m_settings.m_width, m_settings.m_height, m_settings.m_glMajor, m_settings.m_glMinor, m_settings.m_windowName);
        m_topMip = (int)glm::floor(glm::log2(glm::max(float(m_settings.m_width), float(m_settings.m_height))));
        Input::SetWindow(m_window.getWindow());
        Input::Poll();
        const char* screenShaders[] = {
            "screenVert.glsl",
            "screenFrag.glsl"
        };
        const char* primShaders[] = {
            "circleVert.glsl",
            "circleFrag.glsl"
        };
        const char* diffShaders[] = {
            "screenVert.glsl",
            "diffFrag.glsl"
        };
        m_shader.setup(screenShaders, 2);
        m_primShader.setup(primShaders, 2);
        m_diffShader.setup(diffShaders, 2);

        for(Framebuffer& frame : m_diffbuffers)
        {
            frame.init(m_settings.m_width, m_settings.m_height, 1);
        }
        for(Framebuffer& frame : m_framebuffers)
        {
            frame.init(m_settings.m_width, m_settings.m_height, 1);
        }
        m_mesh.init();

        glEnable(GL_BLEND); DebugGL();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); DebugGL();
        glViewport(0, 0, m_settings.m_width, m_settings.m_height); DebugGL();

        Reset();

        for(VertexBuffer& vb : m_vertices)
        {
            vb.reserve(m_settings.m_maxPrimitives * VerticesPerPrimitive);
        }
        for(TransformBuffer& tb : m_transforms)
        {
            tb.reserve(m_settings.m_maxPrimitives);
        }
        
        Image img;
        img.load(m_settings.m_imageName);
        m_texture.Init(img);

        Image img2;
        img2.load(m_settings.m_brushName);
        if(!img2.image)
        {
            printf("Could not load brush %s\n", m_settings.m_brushName);
            exit(1);
        }
        m_brushTex.Init(img2);

        DB_EXT_ONLY(
            DB::Image db_src;
            db_src.width = img.width;
            db_src.height = img.height;
            db_src.image = img.image;
            db_src.Finalize();
            
            DB::Image db_brush;
            db_brush.width = img2.width;
            db_brush.height = img2.height;
            db_brush.image = img2.image;
            db_brush.Finalize();

            DB::Run db_run;
            db_run.source_image_id = db_src.id;
            db_run.brush_image_id = db_brush.id;
            sprintf(db_run.filename, "%s", m_settings.m_imageName);
            sprintf(db_run.brushname, "%s", m_settings.m_brushName);
            sprintf(db_run.username, "%s", getenv("USERNAME"));
            db_run.time_begun = time(nullptr);
            db_run.frames_per_primitive = m_settings.m_framesPerPrimitive;
            db_run.seconds_between_screenshots = m_settings.m_secondsPerScreenshot;
            db_run.max_primitives = m_settings.m_maxPrimitives;
            db_run.primitive_alpha = m_settings.m_primAlpha;
            db_run.Finalize();
            m_runId = db_run.id;

            DB::Open(m_db, "ImageDecompilerDB");
            DB::Write(m_db, db_run);
            DB::Write(m_db, db_src);
            DB::Write(m_db, db_brush);
        );
    }
   void deinit()
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
        m_primShader.deinit();
        m_shader.deinit();
        m_texture.deinit();
        m_brushTex.deinit();
        m_window.deinit();

        DB_EXT_ONLY(
            DB::Close(m_db);
        );
    }
    int CurrentChoice() { return m_frontFrame; }
    void SaveImage()
    {
        if((m_frameIdx & 1023) == 0)
        {
            time_t curTime = time(nullptr);
            time_t duration = curTime - m_lastScreenshot;
            if(duration > time_t(m_settings.m_secondsPerScreenshot))
            {
                char filename[64] = { 0 };
                for(int i = 0; i < 63 && m_settings.m_imageName[i]; ++i)
                {
                    if(m_settings.m_imageName[i] == '.')
                        break;

                    filename[i] = m_settings.m_imageName[i];
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
        return m_transforms[CurrentChoice()].count();
    }
    void MakeRandomChange(VertexBuffer& vertices, TransformBuffer& transforms)
    {
        if(!PrimitiveCount())
        {
            return;
        }

        Vertex* primBegin = nullptr;
        const int num_prims = PrimitiveCount();
        int idx = VerticesPerPrimitive * (num_prims - 1);
        if(num_prims == m_settings.m_maxPrimitives)
        {
            idx = VerticesPerPrimitive * (randu() % num_prims);
        }
        else if(randf() < 0.25f)
        {
            float pos = randf();
            pos = pos * pos;
            pos = 1.0f - pos;
            idx = int(float(VerticesPerPrimitive) * pos * float(num_prims));
            idx = glm::clamp(idx, 0, num_prims - 1);
        }

        idx -= idx % VerticesPerPrimitive;
        primBegin = &vertices[idx];
        
        if(randu() & 1)
        {   
            vec4 color = primBegin->color;
            *(&color.x + randu() % 3) = randf();
            color = glm::mix(primBegin->color, color, randf());
            for(int i = 0; i < VerticesPerPrimitive; ++i)
            {
                primBegin[i].color = color;
            }
        }
        else
        {
            idx /= VerticesPerPrimitive;
            transforms[idx].Randomize();
            transforms[idx].Transform(primBegin);
        }
    }
    void AddPrimitive(Vector<Vertex>& vertices, Vector<PrimTransform>& transforms)
    {
        const vec4 color(randf(), randf(), randf(), 1.0f);

        PrimTransform& xform = transforms.append();
        xform.Randomize();

        const int idx = vertices.count();
        for(int i = 0; i < VerticesPerPrimitive; ++i)
        {
            vertices.append();
        }
        xform.Transform(&vertices[idx]);
        printf("Primitive count: %i\n", PrimitiveCount());
    }
    void DrawIntoBuffer(const Vector<Vertex>& vertices, Framebuffer& destBuffer)
    {
        m_mesh.upload(vertices);
        destBuffer.bind();
        Framebuffer::clear();
        m_mesh.draw();
    }
    void DrawDifference(int idx)
    {
        m_diffbuffers[idx].bind();
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
            m_primShader.bind();
            m_primShader.bindTexture(9, m_brushTex.handle, "brush");
            m_primShader.setUniformFloat("primAlpha", m_settings.m_primAlpha);
            for(int i = 0; i < NumChoices; ++i)
            {
                if(i != CurrentChoice())
                {
                    m_vertices[i] = m_vertices[CurrentChoice()];
                    m_transforms[i] = m_transforms[CurrentChoice()];
                    MakeRandomChange(m_vertices[i], m_transforms[i]);
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

        if((m_frameIdx % m_settings.m_framesPerPrimitive) == 0)
        {
            if(PrimitiveCount() < m_settings.m_maxPrimitives)
            {
                AddPrimitive(m_vertices[CurrentChoice()], m_transforms[CurrentChoice()]);
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
                    AddPrimitive(m_vertices[CurrentChoice()], m_transforms[CurrentChoice()]);
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
        Framebuffer::clear();
        GLScreen::draw();
        m_window.swap();
        return m_window.open();
    }
};

const vec4 Renderer::m_square[] = 
{
    vec4(1.0f,  1.0f,   1.0f, 1.0f), // tr
    vec4(-1.0f, 1.0f,   0.0f, 1.0f), // tl
    vec4(-1.0f, -1.0f,  0.0f, 0.0f), // bl
    vec4(-1.0f, -1.0f,  0.0f, 0.0f), // bl
    vec4(1.0f,  -1.0f,  1.0f, 0.0f), // br
    vec4(1.0f,  1.0f,   1.0f, 1.0f), // tr
};

typedef void (*settings_fn)(sSettings&, const char*);
struct SettingsHandler
{
    const char* pattern;
    settings_fn handler;
    int len;
    SettingsHandler(const char* pat, settings_fn fn) :  pattern(pat), handler(fn), len((int)strlen(pat)) {};
    bool Handle(sSettings& rSettings, const char* arg) const
    {
        if(strncmp(arg, pattern, len) == 0)
        {
            handler(rSettings, arg + len);
            return true;
        }
        return false;
    }
};

int main(int argc, char* argv[])
{
    Renderer renderer;
    seedRandom();

    {
        sSettings settings;
        const SettingsHandler handlers[] = 
        {
            SettingsHandler("-image=", [](sSettings& set, const char* val)
            {
                set.m_imageName = val;
            }),
            SettingsHandler("-brush=", [](sSettings& set, const char* val)
            {
                set.m_brushName = val;
            }),
            SettingsHandler("-maxPrimitives=", [](sSettings& set, const char* val)
            {
                set.m_maxPrimitives = atoi(val);
            }),
            SettingsHandler("-framesPerPrimitive=", [](sSettings& set, const char* val)
            {
                set.m_framesPerPrimitive = atoi(val);
            }),
            SettingsHandler("-secondsPerScreenshot=", [](sSettings& set, const char* val)
            {
                set.m_secondsPerScreenshot = atoi(val);
            }),
            SettingsHandler("-primAlpha=", [](sSettings& set, const char* val)
            {
                set.m_primAlpha = (float)atof(val);
                set.m_primAlpha = glm::clamp(set.m_primAlpha, 0.001f, 1.0f);
            })
        };

        for(int i = 1; i < argc; ++i)
        {
            for(const SettingsHandler& handler : handlers)
            {
                if(handler.Handle(settings, argv[i]))
                {
                    break;
                }
            }
        }

        Image img;
        img.load(settings.m_imageName);
        settings.m_width = img.width;
        settings.m_height = img.height;

        if(!img.image)
        {
            printf("Could not load image %s\n", settings.m_imageName);
            return 1;
        }

        renderer.init(settings);
    }

    while(renderer.Swap()){}

    renderer.deinit();

    return 0;
}
