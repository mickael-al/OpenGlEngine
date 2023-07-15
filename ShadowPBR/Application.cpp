
#define GLEW_STATIC 1
#include "GL/glew.h"
#include "GL/wglew.h"
#include "Application.h"
#include <cstdint>
#include "../common/Texture.h"
#include "../libs/stb/stb_image.h"

bool Application::initialize()
{
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if (error != GLEW_OK)
    {
        return false;
    }

    Texture::SetupManager();

    m_PbrShader.LoadVertexShader("opaque.vs.glsl");
    m_PbrShader.LoadFragmentShader("opaque.fs.glsl");
    m_PbrShader.Create();

    m_ShadowShader.LoadVertexShader("shadow.vs.glsl");
    m_ShadowShader.LoadFragmentShader("shadow.fs.glsl");
    m_ShadowShader.Create();

    m_SkyboxShader.LoadVertexShader("skybox.vs.glsl");
    m_SkyboxShader.LoadFragmentShader("skybox.fs.glsl");
    m_SkyboxShader.Create();

    Mesh* m = new Mesh();
    Mesh::ParseObj(m, "../data/plane.obj");
    m_objects.push_back(m);
    m->setPosition(glm::vec3(0.0f, -2.5f, 0.0f));
    
    m = new Mesh();
    Mesh::ParseObj(m, "../data/lightning/lightning_obj.obj");
    m_objects.push_back(m);
    m->setScale(glm::vec3(0.025f));

    m = new Mesh();
    Mesh::ParseObj(m, "../data/suzanne.obj");
    m_objects.push_back(m);
    m->setPosition(glm::vec3(2.0f, 0.0f, 0.0f));

    m_skybox = new Mesh();
    Mesh::ParseObj(m_skybox, "../data/skybox.obj");

    Light * lm = new Light();
    lm->setPosition(glm::vec3(-8.0f,0.0f,-8.0f));
    lm->getLigthsMatrix()->color = glm::vec3(0.0f,1.0f,0.0f);
    lm->getLigthsMatrix()->range = 2.0f;
    lm->getLigthsMatrix()->status = 1;
    m_lights.push_back(lm);

    lm = new Light();
    lm->setPosition(glm::vec3(-7.5, 0, 7.5));
    lm->getLigthsMatrix()->color = glm::vec3(0.0f, 0.0f, 1.0f);
    lm->getLigthsMatrix()->range = 2.0f;
    lm->getLigthsMatrix()->status = 1;
    m_lights.push_back(lm);

    lm = new Light();
    lm->setPosition(glm::vec3(0.0f, 1.0f, 6.0f));
    lm->getLigthsMatrix()->color = glm::vec3(1.0f, 0.0f, 0.0f);
    lm->getLigthsMatrix()->status = 2;
    lm->getLigthsMatrix()->range = 30.0f;
    lm->getLigthsMatrix()->spotAngle = 50;
    lm->setEulerAngles(glm::vec3(-20.0f,00.0f,0.0f));    
    m_lights.push_back(lm);

    for (int i = 0; i < m_lights.size(); i++)
    {
        m_lightMatrix.push_back(*(m_lights[i]->getLigthsMatrix()));
    }

    m_im = new InputManager(m_window);
    cam = new FlyCamera(m_im,m_width, m_height);
    cam->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));

    uint32_t program = m_PbrShader.GetProgram();
    uint32_t program2 = m_ShadowShader.GetProgram();
    uint32_t program3 = m_SkyboxShader.GetProgram();
    for (int i = 0; i < m_objects.size();i++) 
    {        
        m_objects[i]->Setup(program);
    }

    // UBO Camera
    {
        glGenBuffers(1, &m_UBOCamera);
        glBindBuffer(GL_UNIFORM_BUFFER, m_UBOCamera);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixCamera), nullptr, GL_STREAM_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_UBOCamera);
        int32_t MatBlockIndex = glGetUniformBlockIndex(program, "MatrixCamera");
        glUniformBlockBinding(program, MatBlockIndex, 0);


        MatBlockIndex = glGetUniformBlockIndex(program2, "MatrixCamera");
        glUniformBlockBinding(program2, MatBlockIndex, 0);

        MatBlockIndex = glGetUniformBlockIndex(program3, "MatrixCamera");
        glUniformBlockBinding(program3, MatBlockIndex, 0);        
    }

    //Lights
    {
        glGenBuffers(1, &m_lightUBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightUBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightMatrices) * m_lights.size(), m_lights.data(), GL_STREAM_DRAW);
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
        ubd.maxLight = m_lights.size();
        ubd.maxShadow = 1;
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

    //shadow
    {
        m_framebufferName = 0;
        glGenFramebuffers(1, &m_framebufferName);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferName);

        glGenTextures(1, &m_depthTexture);
        glBindTexture(GL_TEXTURE_2D, m_depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1280, 720, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture, 0);

        glEnable(GL_DEPTH_TEST);

        // Always check that our framebuffer is ok
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            return false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        glGenBuffers(1, &m_UBOShadow);
        glBindBuffer(GL_UNIFORM_BUFFER, m_UBOShadow);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixCamera), nullptr, GL_STREAM_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_UBOShadow);
        int32_t MatBlockIndex = glGetUniformBlockIndex(program, "MatrixShadow");
        glUniformBlockBinding(program, MatBlockIndex, 4);
    }

    //CubeMap
    {
        std::vector<std::string> textures_faces;
        /*textures_faces.push_back("../data/envmaps/test_px.png");
        textures_faces.push_back("../data/envmaps/test_nx.png");
        textures_faces.push_back("../data/envmaps/test_py.png");
        textures_faces.push_back("../data/envmaps/test_ny.png");
        textures_faces.push_back("../data/envmaps/test_pz.png");
        textures_faces.push_back("../data/envmaps/test_nz.png");*/
        textures_faces.push_back("../data/envmaps/pisa_posx.jpg");
        textures_faces.push_back("../data/envmaps/pisa_negx.jpg");
        textures_faces.push_back("../data/envmaps/pisa_posy.jpg");
        textures_faces.push_back("../data/envmaps/pisa_negy.jpg");
        textures_faces.push_back("../data/envmaps/pisa_posz.jpg");
        textures_faces.push_back("../data/envmaps/pisa_negz.jpg");
        glGenTextures(1, &m_cubeMapTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture);
        int width, height, nrChannels;
        unsigned char* data;        
        for (unsigned int i = 0; i < textures_faces.size(); i++)
        {
            data = stbi_load(textures_faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data == nullptr)
            {
                std::cerr << "Error: load CubeMap texture" << std::endl;
                return false;
            }
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, nrChannels == 3 ? GL_RGB : GL_RGBA, width, height, 0, nrChannels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data
            );
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        m_skybox->SetupOnlyVertex(program3);
    }
    return true;
}

void Application::deinitialize()
{
    delete cam;
    delete m_im;
    for (Mesh* obj : m_objects)
    {
        delete obj;
    }
    m_objects.clear();
    glDeleteBuffers(1, &m_UBOCamera);

    glDeleteBuffers(1, &m_lightUBO);

    m_PbrShader.Destroy();

    Texture::PurgeTextures();
}

void Application::update()
{
    m_im->updateAxis();
    m_deltaTime = m_elapsedTime - m_lastElapsedTime;
    m_lastElapsedTime = m_elapsedTime;
    cam->updateCamera(m_deltaTime);
    m_objects[1]->setEulerAngles(glm::vec3(0, m_elapsedTime*20.0f, 0));
    m_objects[2]->setEulerAngles(glm::vec3(m_elapsedTime * 40.0f, 0, 0));
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
    MatrixCamera mc;
    mc.position = cam->getPosition();
    mc.u_ViewMatrix = cam->getViewMatrix();
    mc.u_ProjectionMatrix = cam->getProjectionMatrix();

    glViewport(0, 0, m_width, m_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //skybox
    {        
        glDepthMask(GL_FALSE);
        int32_t program3 = m_SkyboxShader.GetProgram();
        glUseProgram(program3);
        
        glBindBuffer(GL_UNIFORM_BUFFER, m_UBOCamera);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixCamera), &mc, GL_STATIC_DRAW);

        glActiveTexture(GL_TEXTURE_CUBE_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture);
        int32_t skyboxText = glGetUniformLocation(program3, "skybox");
        glUniform1i(skyboxText, 0);

        glBindVertexArray(m_skybox->meshes[0].VAO);

        glDrawElements(GL_TRIANGLES, m_skybox->meshes[0].indicesCount, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);
    }
   
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);    
   

    //shadow
    {
        uint32_t program2 = m_ShadowShader.GetProgram();
        for (int i = 0; i < m_objects.size(); i++)
        {
            m_objects[i]->SetupOnlyVertex(program2);
        }
        glUseProgram(program2);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferName);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        glBindBuffer(GL_UNIFORM_BUFFER, m_UBOCamera);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixCamera), m_lights[2]->getShadowMatrix(), GL_STATIC_DRAW);
        
        for (Mesh* obj : m_objects)
        {
            uint32_t WM = glGetUniformLocation(program2, "u_WorldMatrix");
            glUniformMatrix4fv(WM, 1, false, glm::value_ptr(obj->getModelMatrix()));
            for (auto& submesh : obj->meshes)
            {
                glBindVertexArray(submesh.VAO);
                glDrawElements(GL_TRIANGLES, submesh.indicesCount, GL_UNSIGNED_INT, 0);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        uint32_t program = m_PbrShader.GetProgram();
        for (int i = 0; i < m_objects.size(); i++)
        {
            m_objects[i]->Setup(program);
        }
    }

    uint32_t program = m_PbrShader.GetProgram();
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    int32_t shadowTex = glGetUniformLocation(program, "shadowMap");
    glUniform1i(shadowTex, 5);

    glBindBuffer(GL_UNIFORM_BUFFER, m_UBOCamera);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixCamera), &mc, GL_STATIC_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, m_UBOShadow);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MatrixCamera), m_lights[2]->getShadowMatrix(), GL_STATIC_DRAW);

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
            Material& mat = submesh.materialId > -1 ? obj->materials[submesh.materialId] : Material::defaultMaterial;

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

