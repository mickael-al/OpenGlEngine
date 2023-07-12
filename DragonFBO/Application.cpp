
#define GLEW_STATIC 1
#include "GL/glew.h"
// ici w dans wglew est pour Windows
#include "GL/wglew.h"

#include "Application.h"
#include "DragonData.h"
#include <cstdint>

bool Application::initialize()
{
    GLenum error = glewInit();
    if (error != GLEW_OK)
        return false;

    m_dragonShader.LoadVertexShader("dragon3D.vs.glsl");
    m_dragonShader.LoadFragmentShader("dragon3D.fs.glsl");
    m_dragonShader.Create();

    m_vertexCount = sizeof(DragonVertices)/sizeof(Vertex);

    m_indexCount =sizeof(DragonIndices)/sizeof(uint16_t);
    m_indexType = GL_UNSIGNED_SHORT;

    glBindVertexArray(0);
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertexCount, DragonVertices, GL_STATIC_DRAW);
    glGenBuffers(1, &m_IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * m_indexCount, DragonIndices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);

    // les appels a glVertexAttribPointer vont referencer ce VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    uint32_t program = m_dragonShader.GetProgram();
    // todo: verifiez bien que la valeur de retour est != -1
    const int32_t POSITION = 0;// glGetAttribLocation(program, "a_Position");
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    const int32_t NORMAL = 1;// glGetAttribLocation(program, "a_Normal");
    glEnableVertexAttribArray(NORMAL);
    glVertexAttribPointer(NORMAL, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);

    // UBO
    glGenBuffers(1, &m_UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
    // pas obligatoire de preallouer la zone memoire mais preferable
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_STREAM_DRAW);
    // binding point = ce qui relie un Objet OpenGL avec 
    //                 son equivalent dans un shader

    int32_t MatBlockIndex = glGetUniformBlockIndex(program, "Matrices");
    // note: le '0' (zero) ici correspond au meme '0' 
    // de glBindBufferBase() lors de l'initialisation
    glUniformBlockBinding(program, MatBlockIndex, 0 /*binding point*/);


    glBindBufferBase(GL_UNIFORM_BUFFER, 0/*binding point*/, m_UBO);

    constexpr bool hasDepth = true;
    m_renderTarget.Create(m_width, m_height, hasDepth);


    return true;
}

void Application::deinitialize()
{
    m_renderTarget.Destroy();

    glDeleteBuffers(1, &m_UBO);
    m_dragonShader.Destroy();
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

    // On redirige les fonctions de rendu vers notre FBO
    m_renderTarget.Bind();
    
    {
        // comme on va afficher de la 3D on efface le depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // backface culling et activation du z-buffer
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        uint32_t program = m_dragonShader.GetProgram();
        glUseProgram(program);

        glm::mat4 worldMatrix = glm::rotate(glm::mat4(1.0f), m_elapsedTime, glm::vec3(0.0f, 1.f, 0.f));

        glm::mat4 matrices[2];
        // 0 = camera (view matrix)
        matrices[0] = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -10.f));
        // 1 = projection
        matrices[1] =
            glm::perspectiveFov(glm::radians(45.f), (float)m_width, (float)m_height, 0.1f, 1000.f);

        glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, matrices, GL_STATIC_DRAW);

        uint32_t WM = glGetUniformLocation(program, "u_WorldMatrix");
        glUniformMatrix4fv(WM, 1, false, glm::value_ptr(worldMatrix));

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, m_indexCount, m_indexType, 0);
    }

    // on revient vers le back buffer
    m_renderTarget.Unbind();

    // todo: utiliser un shader pour faire la copie
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_renderTarget.m_FBO);

    glBlitFramebuffer(0, 0, m_width, m_height, // coord sources
                      0, 0, m_width, m_height, // coord destinations
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

