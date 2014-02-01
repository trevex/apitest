#include "pch.h"

#include "cubes_gl_dynamic_buffer.h"
#include "mathlib.h"

Cubes_GL_DynamicBuffer::Cubes_GL_DynamicBuffer()
    : m_ib()
    , m_vb()
    , m_ub()
    , m_vs()
    , m_fs()
    , m_prog()
{}

Cubes_GL_DynamicBuffer::~Cubes_GL_DynamicBuffer()
{
    glDeleteBuffers(1, &m_ib);
    glDeleteBuffers(1, &m_vb);
    glDeleteBuffers(1, &m_ub);
    glDeleteShader(m_vs);
    glDeleteShader(m_fs);
    glDeleteProgram(m_prog);
}

bool Cubes_GL_DynamicBuffer::Init()
{
    // Shaders
    if (!CreateShader(GL_VERTEX_SHADER, "cubes_gl_dynamic_buffer_vs.glsl", &m_vs))
        return false;

    if (!CreateShader(GL_FRAGMENT_SHADER, "cubes_gl_dynamic_buffer_fs.glsl", &m_fs))
        return false;

    if (!CompileProgram(&m_prog, m_vs, m_fs, 0))
        return false;

    // Buffers
    Vertex vertices[] =
    {
        { -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f },
        {  0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f },
        {  0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f },
        { -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f },
        { -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f },
        {  0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f },
        {  0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f },
        { -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f },
    };

    uint16_t indices[] =
    {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        3, 2, 5, 3, 5, 4,
        2, 1, 6, 2, 6, 5,
        1, 7, 6, 1, 0, 7,
        0, 3, 4, 0, 4, 7
    };

    glGenBuffers(1, &m_vb);
    glBindBuffer(GL_ARRAY_BUFFER, m_vb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &m_ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glGenBuffers(1, &m_ub);

    return glGetError() == GL_NO_ERROR;
}

bool Cubes_GL_DynamicBuffer::Begin(GfxBaseApi* _activeAPI)
{
    int width = _activeAPI->GetWidth();
    int height = _activeAPI->GetHeight();

    float c[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    float d = 1.0f;
    glClearBufferfv(GL_COLOR, 0, c);
    glClearBufferfv(GL_DEPTH, 0, &d);

    // Program
    Vec3 dir = { -0.5f, -1, 1 };
    Vec3 at = { 0, 0, 0 };
    Vec3 up = { 0, 0, 1 };
    dir = normalize(dir);
    Vec3 eye = at - 250 * dir;
    Matrix view = matrix_look_at(eye, at, up);
    Matrix proj = matrix_perspective_rh_gl(radians(45.0f), (float)width / (float)height, 0.1f, 10000.0f);
    Matrix view_proj = proj * view;

    glUseProgram(m_prog);
    glUniformMatrix4fv(0, 1, GL_TRUE, &view_proj.x.x);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ub);

    // Input Layout
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
    glBindBuffer(GL_ARRAY_BUFFER, m_vb);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Rasterizer State
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, width, height);

    // Blend State
    glDisable(GL_BLEND);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Depth Stencil State
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    return true;
}

void Cubes_GL_DynamicBuffer::Draw(Matrix* transforms, int count)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_ub);
    
    for (int i = 0; i < count; ++i)
    {
        Constants cb;
        cb.world = transforms[i];
        glBufferData(GL_UNIFORM_BUFFER, sizeof(Constants), &cb, GL_DYNAMIC_DRAW);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
    }
}
