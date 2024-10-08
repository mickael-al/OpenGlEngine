#ifndef OPENVR_MANAGER_H
#define OPENVR_MANAGER_H

#include "openxr/include/openvr.h"
#include <iostream>
#include <vector>

namespace Ge
{
    class Camera;
    class OpenVRManager {
    public:
        OpenVRManager();
        ~OpenVRManager();

        bool initialize();
        void update();
        void swapProjection(bool left, Camera* cam);
        void submit(bool left, unsigned int id);
        void shutdown();
        void getDevicePose();

    private:
        vr::IVRSystem* vrSystem = nullptr;
        vr::IVRCompositor* vrCompositor = nullptr;
        vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
    };
}
#endif // OPENVR_MANAGER_H
