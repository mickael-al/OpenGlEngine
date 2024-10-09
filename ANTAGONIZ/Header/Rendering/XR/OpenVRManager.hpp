#ifndef OPENVR_MANAGER_H
#define OPENVR_MANAGER_H

#include "Initializer.hpp"
#include "openxr/include/openvr.h"
#include <iostream>
#include <vector>
#include "CustomRenderer.hpp"
#include <glm/glm.hpp>

namespace Ge
{
    class Camera;
    class OpenVRManager final : public InitializerAPI
    {
    private:
        friend class RenderingEngine;
        bool initialize(GraphicsDataMisc* gdm);
        void release();
    public:
        void update();
        void swapProjection(bool left, Camera* cam);
        void submit(bool left, unsigned int id);
        void resize(int width, int height);
        void getDevicePose();
        unsigned int getLeftFrameBuffer() const { return leftFrameBuffer; }
        unsigned int getLeftTexture() const { return leftColorTexture; }
        unsigned int getRightFrameBuffer() const { return rightFrameBuffer; }
        unsigned int getRightTexture() const { return rightColorTexture; }
    private:
        vr::IVRSystem* vrSystem = nullptr;
        vr::IVRCompositor* vrCompositor = nullptr;
        vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
        unsigned int leftFrameBuffer;
        unsigned int leftColorTexture;
        unsigned int rightFrameBuffer;
        unsigned int rightColorTexture;
        glm::mat4 hmdPoseMat4;
        glm::mat4 eyeToHeadMat4;
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
    };
}
#endif // OPENVR_MANAGER_H
