#pragma once

#include <cstdint>

#include "../common/GLShader.h"

#include "RenderTarget.h"

// todo: utilisez la glm

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp>

// optionnel: A deplacer dans un entete specifique
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoords;
};

struct Application
{
    int32_t m_width;
    int32_t m_height;

    RenderTarget m_renderTarget;

    GLShader m_dragonShader;

    // todo: creer une classe Mesh
    uint32_t m_VBO = 0;
    uint32_t m_IBO = 0;
    uint32_t m_VAO = 0;
    uint32_t m_indexCount = 0;
    uint32_t m_indexType = 0;
    uint32_t m_vertexCount = 0;
    uint32_t m_UBO = 0;

    float m_elapsedTime = 0.f;

    inline void setSize(int w, int h) { m_width = w; m_height = h; }
    inline void setElapsedTime(float t) { m_elapsedTime = t; }

    bool initialize();
    void deinitialize();
    void update();
    void render();
};
