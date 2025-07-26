#include "glcore.hpp"
#include "Camera.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/quaternion.hpp" 
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/common.hpp"
#include "GraphicsDataMisc.hpp"
#include "imgui-cmake/Header/imgui.h"
#include "PointeurClass.hpp"
#include "Engine.hpp"
#include "LightManager.hpp"

namespace Ge
{
	Camera::Camera(GraphicsDataMisc * gdm, int priority) : GObject()
	{
		m_far = 500.0f;
		m_fov = 70.0f;
		m_near = 0.1f;
		m_priority = priority;
		m_gdm = gdm;
		m_ssbo = m_gdm->str_ssbo.str_camera;
		m_ortho = false;
		m_orthoSize = 10.0f;
		const ptrClass * pc = Engine::getPtrClassAddr();
		m_lm = pc->lightManager;
		mapMemory();
	}	

	Camera::~Camera()
	{		
		
	}

	float Camera::aspectRatio() const
	{
		return (float)m_gdm->str_width / (float)m_gdm->str_height;
	}

	glm::mat4 Camera::getViewMatrix() const
	{
		return glm::inverse(getModelMatrix());
	}

	glm::mat4 Camera::getProjectionMatrix() const
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

	void Camera::mapMemory()
	{	
		if (m_gdm->current_camera == this)
		{
			m_uniformBufferCamera.camPos = m_transform.position;
			m_uniformBufferCamera.view = getViewMatrix();
			m_uniformBufferCamera.proj = getProjectionMatrix();
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(UniformBufferCamera), &m_uniformBufferCamera);
			m_lm->updateShadowCascadeMatrix();
		}
	}

	void Camera::setFieldOfView(float fov)
	{
		m_fov = fov;
		mapMemory();
	}

	void Camera::setNear(float n)
	{
		m_near = n;
		mapMemory();
	}

	void Camera::setFar(float f)
	{
		m_far = f;
		mapMemory();
	}

	void Camera::setPriority(int p)
	{
		m_priority = p;
		mapMemory();
	}

	void Camera::setOrtho(bool state)
	{
		m_ortho = state;
		mapMemory();
	}

	void Camera::setOrthoSize(float size)
	{
		m_orthoSize = size;
		mapMemory();
	}

	float Camera::getOrthoSize() const
	{
		return m_orthoSize;
	}

	bool Camera::getOrtho() const
	{
		return m_ortho;
	}

	float Camera::getFieldOfView()
	{
		return m_fov;
	}

	float Camera::getNear()
	{
		return m_near;
	}

	float Camera::getFar()
	{
		return m_far;
	}

	int Camera::getPriority()
	{
		return m_priority;
	}

	void Camera::onGUI()
	{
		GObject::onGUI();
		ImGui::TextColored(ImVec4(0.2f, 1, 0.2f, 1), "Camera\n");
		if (ImGui::DragFloat("Fov", &m_fov,2.0f,10.0f,180.0f))
		{
			mapMemory();
		}
		if (ImGui::DragFloat("Near", &m_near, 0.5f, 0.0001f, 10.0f))
		{
			mapMemory();
		}
		if (ImGui::DragFloat("Far", &m_far, 0.5f, 1.0f, 10000.0f))
		{
			mapMemory();
		}
		if (ImGui::DragInt("Priority", &m_priority, 1.0f))
		{
			mapMemory();
		}
		if (ImGui::Checkbox("Ortho", &m_ortho))
		{
			mapMemory();
		}
		if (ImGui::DragFloat("OrthoSize", &m_orthoSize, 1.0f))
		{
			mapMemory();
		}
	}
}