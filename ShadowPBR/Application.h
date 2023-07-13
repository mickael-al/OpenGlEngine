#pragma once

#include <cstdint>

#include "../common/GLShader.h"
#include "../common/Mesh.h"
#include <iostream>
#include <iomanip>

struct LightMatrices
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 direction;
    float range;
    float spotAngle;
    uint status;//DirLight = 0 ; PointLight = 1 ; SpotLight = 2
};


struct MatrixCamera
{
    alignas(16) glm::vec3 position;
    alignas(16) glm::mat4 u_ViewMatrix;
    alignas(16) glm::mat4 u_ProjectionMatrix;
};

struct UniformBufferDiver
{
    uint maxLight;
    uint maxShadow;
    float u_time;
    float gamma;
};

struct Application
{
    int32_t m_width;
    int32_t m_height;

    GLShader m_opaqueShader;

    std::vector<Mesh*> m_objects;
    std::vector<LightMatrices> m_lightMatrix;
    UniformBufferDiver ubd;

    uint32_t m_indexCount = 0;
    uint32_t m_indexType = 0;
    uint32_t m_vertexCount = 0;

    uint32_t m_UBOCamera = 0;
    uint32_t m_lightUBO = 0;
    uint32_t m_UBD = 0;
    uint32_t m_UBM = 0;

    float m_elapsedTime = 0.f;

    inline void setSize(int w, int h) { m_width = w; m_height = h; }
    inline void setElapsedTime(float t) { m_elapsedTime = t; }

    void printMat4(const glm::mat4& matrix);
    bool initialize();
    void deinitialize();
    void update();
    void render();
};
