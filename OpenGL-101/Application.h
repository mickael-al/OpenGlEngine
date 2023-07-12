#pragma once

#include <cstdint>

#include "../common/GLShader.h"

// A deplacer dans un entete specifique

struct vec3
{
    float x, y, z;
};

struct Vertex
{
    vec3 position;
    vec3 color;
};

struct Application
{
    int32_t m_width;
    int32_t m_height;

    GLShader m_basicShader;

    // todo: creer une classe Mesh
    uint32_t m_VBO = 0;
    uint32_t m_IBO = 0;
    uint32_t m_VAO = 0;
    uint32_t m_indexCount = 0;
    uint32_t m_indexType = 0;
    uint32_t m_vertexCount = 0;

    inline void setSize(int w, int h) { m_width = w; m_height = h; }

    bool initialize();
    void deinitialize();
    void update();
    void render();
};
