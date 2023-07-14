
#define GLEW_STATIC 1
#include "GL/glew.h"
// ici w dans wglew est pour Windows
#include "GL/wglew.h"

#include "Application.h"
#include <cstdint>

#include "../common/Texture.h"

bool Application::initialize()
{
    GLenum error = glewInit();
    if (error != GLEW_OK)
    {
        return false;
    }

    Texture::SetupManager();

    m_opaqueShader.LoadVertexShader("opaque.vs.glsl");
    m_opaqueShader.LoadFragmentShader("opaque.fs.glsl");
    m_opaqueShader.Create();

    {
        Mesh object;
        Mesh::ParseObj(&object, "../data/lightning/lightning_obj.obj");
        m_objects.push_back(object);
    }

    uint32_t program = m_opaqueShader.GetProgram();
    for (Mesh& obj : m_objects) 
    {
        obj.Setup(program);
    }

    // UBO
    glGenBuffers(1, &m_UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
    // pas obligatoire de preallouer la zone memoire mais preferable
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_UBO);

    // binding de l'UBO et du Block dans le shader (à faire pour chaque shader !)    
    int32_t MatBlockIndex = glGetUniformBlockIndex(program, "Matrices");
    // note: le '0' (zero) ici correspond au meme '0' 
    // de glBindBufferBase() lors de l'initialisation
    glUniformBlockBinding(program, MatBlockIndex, 0);


    // data pour le projecteur 
    {
        m_projTextureID = Texture::LoadTexture("batman_logo.png");

        glGenBuffers(1, &m_lightUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, m_lightUBO);
        // pas obligatoire de preallouer la zone memoire mais preferable
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_STREAM_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_lightUBO);

        // binding de l'UBO et du Block dans le shader (à faire pour chaque shader !)    
        int32_t MatBlockIndex = glGetUniformBlockIndex(program, "LightMatrices");
        // note: le '1' ici correspond au meme '1' 
        // de glBindBufferBase() lors de l'initialisation
        glUniformBlockBinding(program, MatBlockIndex, 1);
    }

    return true;
}

void Application::deinitialize()
{
    glDeleteBuffers(1, &m_UBO);

    glDeleteBuffers(1, &m_lightUBO);

    m_opaqueShader.Destroy();

    Texture::PurgeTextures();
}

void Application::update()
{
}

void Application::render()
{    
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.973f, 0.514f, 0.475f, 1.f);

    // comme on va afficher de la 3D on efface le depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // backface culling et activation du z-buffer
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    uint32_t program = m_opaqueShader.GetProgram();
    glUseProgram(program);
    
    glm::vec3 up{ 0.0f, 1.f, 0.f };    
    glm::mat4 worldMatrix = glm::rotate(glm::mat4(1.0f), m_elapsedTime, up);
    
    // Camera et projection
    glm::vec3 camera_position{ 0.f, 0.f, 150.f };

    glm::mat4 matrices[2];
    // 0 = camera (view matrix)- notez le '-' devant camera_position car on deplace en fait l'objet
    // todo: utilisez glm::lookat a la place
    matrices[0] = glm::translate(glm::mat4(1.f), -camera_position);
    // 1 = projection
    matrices[1] = glm::perspectiveFov(glm::radians(45.f), (float)m_width, (float)m_height, 0.1f, 1000.f);

    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, matrices, GL_STATIC_DRAW);

    // pour le projecteur / shadow mapping ---
    glm::vec3 light_position = { 10.f, 20.f, 50.f };

    glm::mat4 lightMatrices[2];
    // 0 = camera (view matrix)- notez le '-' devant camera_position car on deplace en fait l'objet
    // todo: utilisez glm::lookat a la place
    lightMatrices[0] = glm::translate(glm::mat4(1.f), -light_position);
    // 1 = projection
    lightMatrices[1] = glm::perspectiveFov(glm::radians(45.f), (float)m_width, (float)m_height, 0.1f, 1000.f);

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, lightMatrices, GL_STATIC_DRAW);
    //
    // 
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_projTextureID);
    int32_t PROJTEXTURE = glGetUniformLocation(program, "u_ProjSampler");
    glUniform1i(PROJTEXTURE, 1);
    //
    // --------------------------------------------
    
    uint32_t WM = glGetUniformLocation(program, "u_WorldMatrix");
    glUniformMatrix4fv(WM, 1, false, glm::value_ptr(worldMatrix));

    for (auto& obj : m_objects)
    {
        for (auto& submesh : obj.meshes)
        {
            const Material& mat = submesh.materialId > -1 ? obj.materials[submesh.materialId] : Material::defaultMaterial;

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mat.diffuseTexture);

            // bind implicitement les VBO et IBO rattaches, ainsi que les definitions d'attributs
            glBindVertexArray(submesh.VAO);
            // dessine les triangles
            glDrawElements(GL_TRIANGLES, submesh.indicesCount, GL_UNSIGNED_INT, 0);
        }
    }
}

