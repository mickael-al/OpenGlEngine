#include "OpenVRManager.hpp"
#include "Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glcore.hpp"
#include "GraphicsDataMisc.hpp"
#include "Debug.hpp"

glm::mat4 ConvertSteamVRMatrixToGLM(const vr::HmdMatrix34_t& matPose) {
    return glm::mat4(
        matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0f,  // Colonne 0
        matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0f,  // Colonne 1
        matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0f,  // Colonne 2
        matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f   // Colonne 3 (translation)
    );
}

// Convertir une matrice SteamVR HmdMatrix44_t en glm::mat4
glm::mat4 ConvertSteamVRProjectionToGLM(const vr::HmdMatrix44_t& matProj) {
    return glm::mat4(
        matProj.m[0][0], matProj.m[1][0], matProj.m[2][0], matProj.m[3][0],
        matProj.m[0][1], matProj.m[1][1], matProj.m[2][1], matProj.m[3][1],
        matProj.m[0][2], matProj.m[1][2], matProj.m[2][2], matProj.m[3][2],
        matProj.m[0][3], matProj.m[1][3], matProj.m[2][3], matProj.m[3][3]
    );
}

namespace Ge
{
    void OpenVRManager::update()
    {
        vrCompositor->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
    }

    void OpenVRManager::swapProjection(bool left, Camera* cam)
    {
        vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
        vrSystem->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0.0f, trackedDevicePose, vr::k_unMaxTrackedDeviceCount);        
        if (trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid) 
        {
            vr::HmdMatrix34_t hmdPose = trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;
            hmdPoseMat4 = ConvertSteamVRMatrixToGLM(hmdPose);
        }        
        if (left)
        {
            vr::HmdMatrix34_t eyeToHeadMatrix = vrSystem->GetEyeToHeadTransform(vr::Eye_Left);
            eyeToHeadMat4 = ConvertSteamVRMatrixToGLM(eyeToHeadMatrix);
            viewMatrix = glm::inverse(hmdPoseMat4 * eyeToHeadMat4);
            vr::HmdMatrix44_t projectionMatrix = vrSystem->GetProjectionMatrix(vr::Eye_Left, cam->getNear(), cam->getFar());
            projMatrix = ConvertSteamVRProjectionToGLM(projectionMatrix);

            cam->tempProjView(projMatrix, viewMatrix, glm::vec3(hmdPoseMat4[3]));
    
        }
        else
        {
            vr::HmdMatrix34_t eyeToHeadMatrix = vrSystem->GetEyeToHeadTransform(vr::Eye_Right);
            eyeToHeadMat4 = ConvertSteamVRMatrixToGLM(eyeToHeadMatrix);
            viewMatrix = glm::inverse(hmdPoseMat4 * eyeToHeadMat4);
            vr::HmdMatrix44_t projectionMatrix = vrSystem->GetProjectionMatrix(vr::Eye_Right, cam->getNear(), cam->getFar());
            projMatrix = ConvertSteamVRProjectionToGLM(projectionMatrix);

            cam->tempProjView(projMatrix, viewMatrix, glm::vec3(hmdPoseMat4[3]));
        }
    }

    void OpenVRManager::resize(int width, int height)
    {
        glBindTexture(GL_TEXTURE_2D, leftColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_2D, rightColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
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

    bool OpenVRManager::initialize(GraphicsDataMisc* gdm)
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

        glGenFramebuffers(1, &leftFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, leftFrameBuffer);
        glGenTextures(1, &leftColorTexture);
        glBindTexture(GL_TEXTURE_2D, leftColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gdm->str_width, gdm->str_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, leftColorTexture, 0);
        unsigned int attachmentsF[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, attachmentsF);
        unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::Error("Erreur lors de la creation du framebuffer forward.");
            return false;
        }
        glGenFramebuffers(1, &rightFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, rightFrameBuffer);
        glGenTextures(1, &rightColorTexture);
        glBindTexture(GL_TEXTURE_2D, rightColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gdm->str_width, gdm->str_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rightColorTexture, 0);
        unsigned int attachmentsF2[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, attachmentsF2);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::Error("Erreur lors de la creation du framebuffer forward.");
            return false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return true;
    }

    void OpenVRManager::release()
    {
        if (vrSystem)
        {
            vr::VR_Shutdown();
            vrSystem = nullptr;
            glDeleteFramebuffers(1, &leftFrameBuffer);
            glDeleteFramebuffers(1, &rightFrameBuffer);
            glDeleteTextures(1, &leftColorTexture);
            glDeleteTextures(1, &rightColorTexture);
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