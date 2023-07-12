#pragma once

#include <cstdint>

#include "../common/GLShader.h"
#include "../common/Mesh.h"
#include <iostream>
#include <iomanip>

struct Application
{
    int32_t m_width;
    int32_t m_height;

    GLShader m_opaqueShader;

    std::vector<Mesh*> m_objects;

    uint32_t m_indexCount = 0;
    uint32_t m_indexType = 0;
    uint32_t m_vertexCount = 0;
    uint32_t m_UBO = 0;

    uint32_t m_lightUBO = 0;
    uint32_t m_projTextureID = 0;

    float m_elapsedTime = 0.f;

    inline void setSize(int w, int h) { m_width = w; m_height = h; }
    inline void setElapsedTime(float t) { m_elapsedTime = t; }

    void printMat4(const glm::mat4& matrix);
    bool initialize();
    void deinitialize();
    void update();
    void render();
};
