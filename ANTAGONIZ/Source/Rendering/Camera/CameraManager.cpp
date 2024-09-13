#include "glcore.hpp"
#include "CameraManager.hpp"
#include "Debug.hpp"
#include "UniformBufferCamera.hpp"
#include "Camera.hpp"
#include "FlyCamera.hpp"
#include "GraphicsDataMisc.hpp"
#include <algorithm>

namespace Ge
{
    bool CameraManager::initialize(GraphicsDataMisc * gdm)
    {        
        m_gdm = gdm;
		glGenBuffers(1, &m_ssbo);
		m_gdm->str_ssbo.str_camera = m_ssbo;
		updateStorage();
        m_flyCamera = new FlyCamera(gdm);
		m_currentCamera = (Camera *)m_flyCamera;
		m_gdm->current_camera = m_currentCamera;
		m_currentCamera->setName("FlyCamera");
		m_cameras.push_back(m_currentCamera);
		updateStorage(); 
		Debug::INITSUCCESS("CameraManager");		
        return true;
    }

    Camera *CameraManager::createCamera(std::string name,int priority)
    {
        Camera * cam = new Camera(m_gdm, priority);
		m_cameras.push_back(cam);
		cam->setName(name);
		updatePriorityCamera();
		return cam;
    }

	void CameraManager::updateFlyCam()
	{
		if (m_currentCamera == (Camera *)m_flyCamera)
		{
			m_flyCamera->updateCamera();
		}
	}
    
    void CameraManager::releaseCamera(Camera *camera)
    {
		m_cameras.erase(std::remove(m_cameras.begin(), m_cameras.end(), camera), m_cameras.end());
        delete (camera);
        updatePriorityCamera();
    }

    void CameraManager::updatePriorityCamera()
    {
        int minimum = INT_MAX;
        for (int i = 0 ; i < m_cameras.size();i++)
		{
		   	if (m_cameras[i]->getPriority() < minimum)
			{
				minimum = m_cameras[i]->getPriority();
				m_currentCamera = m_cameras[i];
				m_gdm->current_camera = m_currentCamera;
			}
		}    	
		updateStorage();
    }

    Camera *CameraManager::getCurrentCamera()
    {
        return m_currentCamera;
    }

	void CameraManager::updateStorage()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(UniformBufferCamera), nullptr, GL_DYNAMIC_DRAW);
		if (m_currentCamera != nullptr)
		{
			m_currentCamera->mapMemory();
		}
	}

    void CameraManager::release()
    {
		for (int i = 0; i < m_cameras.size(); i++)
		{
			delete(m_cameras[i]);
		}
		m_cameras.clear();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glDeleteBuffers(1, &m_ssbo);
        Debug::RELEASESUCCESS("CameraManager");
    }
}