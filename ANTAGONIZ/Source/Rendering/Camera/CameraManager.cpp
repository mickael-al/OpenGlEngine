#include "CameraManager.hpp"

namespace Ge
{
    bool CameraManager::initialize()
    {        
		/*Instance = this;
        m_flyCamera = new FlyCamera(vM, im);      
		((Camera *)m_flyCamera)->setName("FlyCamera");
		m_cameras.push_back((Camera *)m_flyCamera);
        CameraManager::updatePriorityCamera(); 
		Debug::INITSUCCESS("CameraManager");		*/
        return true;
    }

    Camera *CameraManager::createCamera(std::string name)
    {
        /*Camera* cam = new Camera(vulkanM);
		m_cameras.push_back(cam);
		cam->setName(name);
		CameraManager::updatePriorityCamera();
		return cam;*/
    }

	void CameraManager::updateFlyCam()
	{
		/*if (currentCamera == (Camera*)m_flyCamera)
		{
			m_flyCamera->updateCamera();
		}*/
	}
    
    void CameraManager::releaseCamera(Camera *camera)
    {
		/*m_cameras.erase(std::remove(m_cameras.begin(), m_cameras.end(), camera), m_cameras.end());
        delete (camera);
        CameraManager::updatePriorityCamera();*/
    }

    void CameraManager::updatePriorityCamera()
    {
       /* int minimum = INT_MAX;
        for (int i = 0 ; i < m_cameras.size();i++)
		{
		   	if (m_cameras[i]->getPriority() < minimum)
			{
				minimum = m_cameras[i]->getPriority();
				currentCamera = m_cameras[i];
			}
		}    
		*/
    }

    /*Camera* CameraManager::getCurrentCamera()
    {
        return currentCamera;
    }*/

    void CameraManager::release()
    {
		/*if (m_flyCamera != nullptr)
		{
			m_cameras.erase(std::remove(m_cameras.begin(), m_cameras.end(), (Camera *)m_flyCamera), m_cameras.end());
			delete (m_flyCamera);
		}
		for (int i = 0; i < m_cameras.size(); i++)
		{
			delete(m_cameras[i]);
		}
		m_cameras.clear();
        Debug::RELEASESUCCESS("CameraManager");*/
    }
}