
#define GLEW_STATIC 1
#include "GL/glew.h"
#include "GL/wglew.h"
#include "Application.h"
#include <cstdint>
#include "../common/Texture.h"

bool Application::initialize()
{
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if (error != GLEW_OK)
    {
        return false;
    }

    Texture::SetupManager();

    m_opaqueShader.LoadVertexShader("opaque.vs.glsl");
    m_opaqueShader.LoadFragmentShader("opaque.fs.glsl");
    m_opaqueShader.Create();

    Mesh* m = new Mesh();
    Mesh::ParseObj(m, "../data/lightning/lightning_obj.obj");
    m_objects.push_back(m);
    m->setScale(glm::vec3(0.025f));

    m = new Mesh();
    Mesh::ParseObj(m, "../data/plane.obj");
    m_objects.push_back(m);    
    m->setPosition(glm::vec3(0.0f, -2.5f, 0.0f));

    m = new Mesh();
    Mesh::ParseObj(m, "../data/suzanne.obj");
    m_objects.push_back(m);
    m->setPosition(glm::vec3(2.0f, 0.0f, 0.0f));

    LightMatrices lm;
    lm.position = glm::vec3(0.0f);
    lm.color = glm::vec3(1.0f);
    lm.direction = glm::vec3(0.0f);
    lm.range = 10.0f;
    lm.spotAngle = 0.0f;
    lm.status = 0;
    m_lightMatrix.push_back(lm);

    uint32_t program = m_opaqueShader.GetProgram();
    for (int i = 0; i < m_objects.size();i++) 
    {
        m_objects[i]->Setup(program);        
    }


    // UBO Camera
    glGenBuffers(1, &m_UBOCamera);
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBOCamera);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixCamera), nullptr, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_UBOCamera);
    int32_t MatBlockIndex = glGetUniformBlockIndex(program, "MatrixCamera");
    glUniformBlockBinding(program, MatBlockIndex, 0);

    //Lights
    {
        glGenBuffers(1, &m_lightUBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightUBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightMatrices) * m_lightMatrix.size(), m_lightMatrix.data(), GL_STREAM_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_lightUBO);
        int32_t MatBlockIndex = glGetUniformBlockIndex(program, "LightUBO");
        glUniformBlockBinding(program, MatBlockIndex, 1);
    }

    //Divers
    {
        glGenBuffers(1, &m_UBD);
        glBindBuffer(GL_UNIFORM_BUFFER, m_UBD);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferDiver), nullptr, GL_STREAM_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_UBD);
        int32_t MatBlockIndex = glGetUniformBlockIndex(program, "UniformBufferDiver");
        glUniformBlockBinding(program, MatBlockIndex, 2);
        ubd.gamma = 1.0f;
        ubd.maxLight = m_lightMatrix.size();
        ubd.maxShadow = 0;
        ubd.u_time = 0.0f;
    }

    //Materials
    {
        glGenBuffers(1, &m_UBM);
        glBindBuffer(GL_UNIFORM_BUFFER, m_UBM);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(SubMat), nullptr, GL_STREAM_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_UBM);
        int32_t MatBlockIndex = glGetUniformBlockIndex(program, "Materials");
        glUniformBlockBinding(program, MatBlockIndex, 3);
    }
    return true;
}

void Application::deinitialize()
{
    for (Mesh* obj : m_objects)
    {
        delete obj;
    }
    m_objects.clear();
    glDeleteBuffers(1, &m_UBOCamera);

    glDeleteBuffers(1, &m_lightUBO);

    m_opaqueShader.Destroy();

    Texture::PurgeTextures();
}

void Application::update()
{
}

void Application::printMat4(const glm::mat4& matrix)
{
    const float* data = glm::value_ptr(matrix);

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            std::cout << std::setw(10) << std::setprecision(4) << data[j * 4 + i] << " ";
        }
        std::cout << std::endl;
    }
}

void Application::render()
{    
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    uint32_t program = m_opaqueShader.GetProgram();
    glUseProgram(program);
    
    MatrixCamera mc;
    glm::vec3 up{ 0.0f, 1.f, 0.f };        
    mc.position = glm::vec3( 0.f, 0.f, -10.f);
    mc.u_ViewMatrix = glm::translate(glm::mat4(1.f), mc.position);
    mc.u_ProjectionMatrix = glm::perspectiveFov(glm::radians(45.f), (float)m_width, (float)m_height, 0.1f, 1000.f);

    glBindBuffer(GL_UNIFORM_BUFFER, m_UBOCamera);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixCamera), &mc, GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightUBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightMatrices) * m_lightMatrix.size(), m_lightMatrix.data(), GL_STATIC_DRAW);
    ubd.u_time = m_elapsedTime;
        
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBD);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferDiver), &ubd, GL_STATIC_DRAW);

    for (Mesh * obj : m_objects)
    {        
        uint32_t WM = glGetUniformLocation(program, "u_WorldMatrix");
        glUniformMatrix4fv(WM, 1, false, glm::value_ptr(obj->getModelMatrix()));

        for (auto& submesh : obj->meshes)
        {
            const Material& mat = submesh.materialId > -1 ? obj->materials[submesh.materialId] : Material::defaultMaterial;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mat.diffuseTexture);
            int32_t albedoText = glGetUniformLocation(program, "albedoMap");
            glUniform1i(albedoText, 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mat.normalMap);
            int32_t normalText = glGetUniformLocation(program, "normalMap");
            glUniform1i(normalText, 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, mat.metallicMap);
            int32_t metallicText = glGetUniformLocation(program, "metallicMap");
            glUniform1i(metallicText, 2);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, mat.roughnessMap);
            int32_t roughnessText = glGetUniformLocation(program, "roughnessMap");
            glUniform1i(roughnessText, 3);

            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, mat.aoMap);
            int32_t aoText = glGetUniformLocation(program, "aoMap");
            glUniform1i(aoText, 4);

            glBindBuffer(GL_UNIFORM_BUFFER, m_UBM);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(SubMat), &mat.submat, GL_STATIC_DRAW);
            glBindVertexArray(submesh.VAO);
            glDrawElements(GL_TRIANGLES, submesh.indicesCount, GL_UNSIGNED_INT, 0);
        }
    }

}

