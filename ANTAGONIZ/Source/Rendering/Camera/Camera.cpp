#include "Camera.hpp"


namespace Ge
{
	Camera::Camera() : GObject()
	{
		m_far = 500.0f;
		m_fov = 70.0f;
		m_near = 0.01f;
		m_priority = 0;
		m_ortho = false;
		m_orthoSize = 10.0f;
	}	

	Camera::~Camera()
	{		
		
	}

	float Camera::aspectRatio() const
	{
		return 0.0f;
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

		projectionMatrix = glm::scale(projectionMatrix, glm::vec3(1.0f, -1.0f, 1.0f));

		return projectionMatrix;
	}

	void Camera::setFieldOfView(float fov)
	{
		m_fov = fov;
	}

	void Camera::setNear(float n)
	{
		m_near = n;
	}

	void Camera::setFar(float f)
	{
		m_far = f;
	}

	void Camera::setPriority(int p)
	{
		m_priority = p;
	}

	void Camera::setOrtho(bool state)
	{
		m_ortho = state;
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

		}
		if (ImGui::DragFloat("Near", &m_near, 0.5f, 0.0001f, 10.0f))
		{

		}
		if (ImGui::DragFloat("Far", &m_far, 0.5f, 1.0f, 10000.0f))
		{

		}
		if (ImGui::DragInt("Priority", &m_priority, 1.0f))
		{

		}
		if (ImGui::Checkbox("Ortho", &m_ortho))
		{

		}
		if (ImGui::DragFloat("OrthoSize", &m_orthoSize, 1.0f))
		{
	
		}
	}
}