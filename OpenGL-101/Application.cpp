
#define GLEW_STATIC 1
#include "GL/glew.h"
// ici w dans wglew est pour Windows
#include "GL/wglew.h"

#include <array>
#include "Application.h"

bool Application::initialize()
{
    GLenum error = glewInit();
    if (error != GLEW_OK)
        return false;

    m_basicShader.LoadVertexShader("basic.vs.glsl");
    m_basicShader.LoadFragmentShader("basic.fs.glsl");
    m_basicShader.Create();

    const std::array<Vertex, 4> quadVertices{
        Vertex{vec3{-0.8f, -0.8f, 0.f}, vec3{1.f, 0.f, 0.f}},
        Vertex{vec3{+0.8f, -0.8f, 0.f}, vec3{0.f, 1.f, 0.f}},
        Vertex{vec3{+0.8f, +0.8f, 0.f}, vec3{0.f, 0.f, 1.f}},
        Vertex{vec3{-0.8f, +0.8f, 0.f}, vec3{1.f, 1.f, 1.f}}
    };
    m_vertexCount = quadVertices.size();

    const std::array<uint16_t, 6> quadIndices{ 0, 1, 2, 0, 2, 3 };
    m_indexCount = quadIndices.size();
    m_indexType = GL_UNSIGNED_SHORT;

    glBindVertexArray(0);
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertexCount, quadVertices.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &m_IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * m_indexCount, quadIndices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);

    // les 2 appels a glVertexAttribPointer vont referencer ce VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    uint32_t program = m_basicShader.GetProgram();
    // todo: verifiez bien que la valeur de retour est != -1
    const int32_t POSITION = glGetAttribLocation(program, "a_Position");
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    const int32_t COLOR = glGetAttribLocation(program, "a_Color");
    glEnableVertexAttribArray(COLOR);
    glVertexAttribPointer(COLOR, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glBindVertexArray(0);

    return true;
}

void Application::deinitialize()
{
    m_basicShader.Destroy();
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_IBO);
    glDeleteVertexArrays(1, &m_VAO);
}

void Application::update()
{
}

void Application::render()
{    
    glViewport(0, 0, m_width, m_height);
    glClearColor(1.f, 1.f, 0.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    uint32_t program = m_basicShader.GetProgram();
    glUseProgram(program);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, m_indexType, 0);
}

