#ifndef __ENGINE_CAMERA_MANAGER__
#define __ENGINE_CAMERA_MANAGER__

#include <string>
#include <vector>
#include "Initializer.hpp"

namespace Ge
{
    class Camera;
    class FlyCamera;
    class CameraManager final : public Initializer
    {
    public:
        bool initialize();
        void release();
        Camera *createCamera(std::string name = "Camera");
        void releaseCamera(Camera *camera);      
        //Camera *getCurrentCamera();
		void updateFlyCam();
    private:		
        std::vector<Camera *> m_camera;
        FlyCamera * m_flyCamera;
    };
}

#endif //__ENGINE_CAMERA_MANAGER__