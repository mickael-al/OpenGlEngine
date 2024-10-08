#include "OpenVRManager.hpp"
#include "Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

glm::mat4 ConvertHmdMatrixToGLM(const vr::HmdMatrix44_t& hmdMatrix) {
    // Créer une matrice glm::mat4 en utilisant les éléments de vr::HmdMatrix44_t
    return glm::mat4(
        hmdMatrix.m[0][0], hmdMatrix.m[1][0], hmdMatrix.m[2][0], hmdMatrix.m[3][0],
        hmdMatrix.m[0][1], hmdMatrix.m[1][1], hmdMatrix.m[2][1], hmdMatrix.m[3][1],
        hmdMatrix.m[0][2], hmdMatrix.m[1][2], hmdMatrix.m[2][2], hmdMatrix.m[3][2],
        hmdMatrix.m[0][3], hmdMatrix.m[1][3], hmdMatrix.m[2][3], hmdMatrix.m[3][3]
    );
}
namespace Ge
{

    OpenVRManager::OpenVRManager() : vrSystem(nullptr) 
    {
    }

    OpenVRManager::~OpenVRManager() 
    {
        shutdown();
    }

    void OpenVRManager::update()
    {
        vrCompositor->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
    }

    void OpenVRManager::swapProjection(bool left, Camera* cam)
    {
        if (left)
        {
            vr::HmdMatrix44_t projMatrixLeft = vrSystem->GetProjectionMatrix(vr::Eye_Left, cam->getNear(), cam->getFar());
            glm::mat4 proj = ConvertHmdMatrixToGLM(projMatrixLeft);
            cam->tempProj(proj);
        }
        else
        {
            vr::HmdMatrix44_t projMatrixRight = vrSystem->GetProjectionMatrix(vr::Eye_Right, cam->getNear(), cam->getFar());
            glm::mat4 proj = ConvertHmdMatrixToGLM(projMatrixRight);
            cam->tempProj(proj);
        }
    }

    void OpenVRManager::submit(bool left, unsigned int id)
    {
        if (left)
        {
            vr::Texture_t leftTexture = { (void*)id, vr::TextureType_OpenGL, vr::ColorSpace_Auto };
            vr::EVRCompositorError e = vrCompositor->Submit(vr::Eye_Left, &leftTexture);
            if (e != 0)
            {
                std::cout << "Error: " << e << std::endl;
            }
        }
        else
        {
            vr::Texture_t rightTexture = { (void*)id, vr::TextureType_OpenGL, vr::ColorSpace_Auto };
            vr::EVRCompositorError e = vrCompositor->Submit(vr::Eye_Right, &rightTexture);
            if (e != 0)
            {
                std::cout << "Error: " << e << std::endl;
            }
        }
    }

    bool OpenVRManager::initialize() 
    {
        vr::EVRInitError eError = vr::VRInitError_None;
        vr::IVRSystem* pSystem = vr::VR_Init(&eError, vr::VRApplication_Scene);
        if (!pSystem) 
        {
            std::cerr << "Échec de l'initialisation d'OpenVR." << std::endl;
            return false;
        }
        vrSystem = pSystem;
        vrCompositor = vr::VRCompositor();
        if (!vrCompositor) 
        {
            std::cerr << "Erreur lors de la récupération de l'interface IVRCompositor." << std::endl;
            return false;
        }
        return true;
    }

    void OpenVRManager::shutdown() 
    {
        if (vrSystem) 
        {
            vr::VR_Shutdown();
            vrSystem = nullptr;
        }
    }

    void OpenVRManager::getDevicePose() 
    {
        for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) 
        {
            if (trackedDevicePose[i].bPoseIsValid) 
            {

            }
        }
    }
}