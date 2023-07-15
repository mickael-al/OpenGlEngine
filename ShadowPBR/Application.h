#pragma once

#include <cstdint>
#include "../common/GLShader.h"
#include "../common/Mesh.h"
#include <iostream>
#include <iomanip>
#include "InputManager.hpp"

struct LightMatrices
{
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec3 direction;
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

class Camera : public GObject
{
public:
    Camera(int32_t width,int32_t height) : GObject()
    {
        m_far = 500.0f;
        m_fov = 70.0f;
        m_near = 0.1f;
        m_ortho = false;
        m_orthoSize = 10.0f;
        m_width = width;
        m_height = height;
    }
    glm::mat4 getViewMatrix() const
    {
        return glm::inverse(getModelMatrix());
    }
    float aspectRatio() const
    {
        return (float)m_width / (float)m_height;
    }
    glm::mat4 getProjectionMatrix() const
    {
        glm::mat4 projectionMatrix;
        if (m_ortho)
        {
            float halfHeight = m_orthoSize * 0.5f;
            float halfWidth = halfHeight * aspectRatio();
            projectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, m_near, m_far);
        }
        else
        {
            projectionMatrix = glm::perspective(glm::radians(m_fov), aspectRatio(), m_near, m_far);
        }

        return projectionMatrix;
    }
    void setSize(int w, int h) { m_width = w; m_height = h; }
    void mapMemory(){}
protected:
    float m_fov = 80.0f;
    float m_near = 0.1f;
    float m_far = 300.0f;
    bool m_ortho = false;
    int32_t m_width;
    int32_t m_height;
    float m_orthoSize;
};

class Light : public GObject
{
public:
    Light() : GObject()    
    {
        m_far = 500.0f;
        m_fov = 45.0f;
        m_near = 0.1f;
        m_ortho = true;
        m_orthoSize = 10.0f;
        m_width = 1280;
        m_height = 720;
    }

    float aspectRatio() const
    {
        return (float)m_width / (float)m_height;
    }
    LightMatrices * getLigthsMatrix()
    {
        m_lm.position = m_transform.position;
        m_lm.direction = getDirection();        
        return &m_lm;
    }

    glm::mat4 getViewMatrix() const
    {
        return glm::inverse(getModelMatrix());
    }

    MatrixCamera * getShadowMatrix()
    {
        m_cam.position = m_transform.position;
        m_cam.u_ViewMatrix = getViewMatrix();
        m_cam.u_ProjectionMatrix = getProjectionMatrix();
        return &m_cam;
    }
    glm::mat4 getProjectionMatrix() const
    {
        glm::mat4 projectionMatrix;
        if (m_ortho)
        {
            float halfHeight = m_orthoSize * 0.5f;
            float halfWidth = halfHeight * aspectRatio();
            projectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, m_near, m_far);
        }
        else
        {
            projectionMatrix = glm::perspective(glm::radians(m_fov), aspectRatio(), m_near, m_far);
        }

        return projectionMatrix;
    }
    void mapMemory() {}
private:
    LightMatrices m_lm;
    MatrixCamera m_cam;
    float m_fov = 60.0f;
    float m_near = 0.1f;
    float m_far = 500.0f;
    bool m_ortho = false;
    int32_t m_width;
    int32_t m_height;
    float m_orthoSize;
};

class FlyCamera : public Camera
{
public:
    FlyCamera(InputManager* im, int32_t width, int32_t height) : Camera(width, height)
    {
        m_angleY = 0.0f;
        m_angleX = 0.0f;
        m_moveSpeed = 0.0f;
        m_im = im;
    }
    void updateCamera(float dt)
    {
        if (m_im->getKey(340))
        {
            m_addMoveSpeed += dt * m_fastSpeedMult;
        }
        else
        {
            m_addMoveSpeed = 0.0f;
        }
        m_moveSpeed = dt * m_maxSpeed;
        m_moveSpeed += m_addMoveSpeed;

        if (m_im->getMouse(1))
        {
            m_angleY -= m_im->axisMouseY() * m_maxSpeedRotate;
            m_angleX -= m_im->axisMouseX() * m_maxSpeedRotate;
            setEulerAngles(glm::vec3(m_angleY, m_angleX, 0.0f));
        }

        if (m_im->getKey(87))
        {
            setPosition(getPosition() + transformDirectionAxeZ() * m_moveSpeed);
        }
        else if (m_im->getKey(83))
        {
            setPosition(getPosition() + transformDirectionAxeZ() * -m_moveSpeed);
        }

        if (m_im->getKey(68))
        {
            setPosition(getPosition() + transformDirectionAxeX() * -m_moveSpeed);
        }
        else if (m_im->getKey(65))
        {
            setPosition(getPosition() + transformDirectionAxeX() * m_moveSpeed);
        }

        if (m_im->getKey(341))
        {
            setPosition(getPosition() + glm::vec3(0, -1, 0) * m_moveSpeed);
        }
        else if (m_im->getKey(32))
        {
            setPosition(getPosition() + glm::vec3(0, 1, 0) * m_moveSpeed);
        }
    }
private:
    float m_angleX;
    float m_angleY;
    float m_maxSpeed = 10.0f;
    float m_fastSpeedMult = 1.0f;
    float m_moveSpeed = 0.0f;
    float m_maxSpeedRotate = 0.2f;
    float m_addMoveSpeed = 0.0f;
    InputManager* m_im;
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

    GLShader m_PbrShader;
    GLShader m_ShadowShader;

    std::vector<Mesh*> m_objects;
    std::vector<Light*> m_lights;
    std::vector<LightMatrices> m_lightMatrix;
    UniformBufferDiver ubd;
    InputManager* m_im;
    FlyCamera* cam = nullptr;

    uint32_t m_indexCount = 0;
    uint32_t m_indexType = 0;
    uint32_t m_vertexCount = 0;

    uint32_t m_UBOCamera = 0;
    uint32_t m_UBOShadow = 0;
    uint32_t m_UBOCubeMap = 0;
    uint32_t m_lightUBO = 0;
    uint32_t m_UBD = 0;
    uint32_t m_UBM = 0;
    uint32_t m_framebufferName = 0;
    uint32_t m_depthTexture = 0;

    GLFWwindow* m_window;
    float m_elapsedTime = 0.0f;
    float m_lastElapsedTime = 0.0f;
    float m_deltaTime = 0.0f;

    inline void setSize(int w, int h) { m_width = w; m_height = h; if (cam != nullptr) { cam->setSize(w, h); } }
    inline void setElapsedTime(float t) { m_elapsedTime = t; }
    inline void setWindow(GLFWwindow* window) { m_window = window; }

    void printMat4(const glm::mat4& matrix);
    bool initialize();
    void deinitialize();
    void update();
    void render();
};
