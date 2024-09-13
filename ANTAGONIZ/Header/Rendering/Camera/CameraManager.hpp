#ifndef __ENGINE_CAMERA_MANAGER__
#define __ENGINE_CAMERA_MANAGER__

#include "Initializer.hpp"
#include "Manager.hpp"
#include <vector>
#include <string>

namespace Ge
{
	class Camera;
	class FlyCamera;
}

namespace Ge
{
    class CameraManager final : public InitializerAPI, public Manager
    {
	protected:
		friend class RenderingEngine;
		friend class Window;
		bool initialize(GraphicsDataMisc * gdm);
		void release();
		void updateStorage();
		void updatePriorityCamera();
	public:
        Camera *createCamera(std::string name = "Camera",int priority = 0);		
        void releaseCamera(Camera *camera);      
        Camera *getCurrentCamera();
		void updateFlyCam();	
    private:				
		std::vector<Camera *> m_cameras;
        Camera * m_currentCamera;
		GraphicsDataMisc * m_gdm;
        FlyCamera * m_flyCamera;
    };
}

#endif //!__ENGINE_CAMERA_MANAGER__