#include "glcore.hpp"
#include "Lights.hpp"
#include "GraphicsDataMisc.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/common.hpp"
#include "Component.hpp"
#include "imgui-cmake/Header/imgui.h"
#include "Transform.hpp"
#include "Debug.hpp"
#include <algorithm>
#include "Camera.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"

namespace Ge
{
	Lights::Lights(unsigned int index, GraphicsDataMisc *gdm) : GObject()
	{
		m_gdm = gdm;
		m_ssbo = m_gdm->str_ssbo.str_light;
		m_ssboShadow = m_gdm->str_ssbo.str_shadow;
		m_index = index;
		m_ubl.color = glm::vec3(1.0f);
		m_ubl.range = 10.0f;
		m_ubl.spotAngle = 45.0f;
		m_ubl.shadowId = -1;
		m_shadowMatrix.resize(m_ubl.status == 0 ? SHADOW_MAP_CASCADE_COUNT : (m_ubl.status == 1 ? SHADOW_MAP_CUBE_COUNT : SHADOW_MAP_SPOT_COUNT));
	}

	void Lights::mapMemory()
	{
		m_ubl.position = m_transform.position;
		m_ubl.direction = getDirection();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, m_index * sizeof(UniformBufferLight), sizeof(UniformBufferLight), &m_ubl);
		if (m_ubl.shadowId >= 0)
		{
			mapMemoryShadow();
		}
	}

	void Lights::setshadow(bool state)
	{
		if (m_shadow != state)
		{
			m_shadow = state;
			const ptrClass* pc = Engine::getPtrClassAddr();
			pc->lightManager->updateStorageShadow();
		}
	}

	void Lights::mapMemoryShadow()
	{					
		if (m_ubl.status == 0 || m_ubl.status == 2)
		{
			m_ubl.direction = getDirection();
			Camera* currentCamera = m_gdm->current_camera;
			float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

			float nearClip = currentCamera->getNear();
			float farClip = currentCamera->getFar();
			float clipRange = farClip - nearClip;

			float minZ = nearClip;
			float maxZ = nearClip + clipRange;

			float range = maxZ - minZ;
			float ratio = maxZ / minZ;

			for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
			{
				float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
				float log = minZ * std::pow(ratio, p);
				float uniform = minZ + range * p;
				float d = cascadeSplitLambda * (log - uniform) + uniform;
				cascadeSplits[i] = (d - nearClip) / clipRange;
			}

			// Calculate orthographic projection matrix for each cascade
			float lastSplitDist = 0.0;
			for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
			{
				float splitDist = cascadeSplits[i];

				frustumCorners[0] = glm::vec3(-1.0f, 1.0f, -1.0f);
				frustumCorners[1] = glm::vec3(1.0f, 1.0f, -1.0f);
				frustumCorners[2] = glm::vec3(1.0f, -1.0f, -1.0f);
				frustumCorners[3] = glm::vec3(-1.0f, -1.0f, -1.0f);
				frustumCorners[4] = glm::vec3(-1.0f, 1.0f, 1.0f);
				frustumCorners[5] = glm::vec3(1.0f, 1.0f, 1.0f);
				frustumCorners[6] = glm::vec3(1.0f, -1.0f, 1.0f);
				frustumCorners[7] = glm::vec3(-1.0f, -1.0f, 1.0f);

				glm::mat4 invCam = glm::inverse(currentCamera->getProjectionMatrix() * currentCamera->getViewMatrix());
				for (uint32_t i = 0; i < 8; i++)
				{
					glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
					frustumCorners[i] = invCorner / invCorner.w;
				}

				for (uint32_t i = 0; i < 4; i++)
				{
					glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
					frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
					frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
				}

				// Get frustum center
				glm::vec3 frustumCenter = glm::vec3(0.0f);
				for (uint32_t i = 0; i < 8; i++)
				{
					frustumCenter += frustumCorners[i];
				}
				frustumCenter /= 8.0f;

				float radius = 0.0f;
				for (uint32_t i = 0; i < 8; i++)
				{
					float distance = glm::length(frustumCorners[i] - frustumCenter);
					radius = glm::max(radius, distance);
				}
				radius = std::ceil(radius * 16.0f) / 16.0f;

				glm::vec3 maxExtents = glm::vec3(radius);
				glm::vec3 minExtents = -maxExtents;

				m_shadowMatrix[i].pos = frustumCenter - glm::normalize(m_ubl.direction) * -minExtents.z;
				glm::mat4 lightViewMatrix = glm::lookAt(m_shadowMatrix[i].pos, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, maxExtents.y, minExtents.y, 0.0f, maxExtents.z - minExtents.z);

				m_shadowMatrix[i].splitDepth = (nearClip + splitDist * clipRange) * -1.0f;
				m_shadowMatrix[i].projview = lightOrthoMatrix * lightViewMatrix;				
				lastSplitDist = cascadeSplits[i];
			}
		}

		if (m_ubl.status == 1)
		{
			glm::vec3 directions[6] = 
			{
			   glm::vec3(1.0f, 0.0f, 0.0f),  // Positive X
			   glm::vec3(-1.0f, 0.0f, 0.0f), // Negative X
			   glm::vec3(0.0f, 1.0f, 0.0f),  // Positive Y
			   glm::vec3(0.0f, -1.0f, 0.0f), // Negative Y
			   glm::vec3(0.0f, 0.0f, 1.0f),  // Positive Z
			   glm::vec3(0.0f, 0.0f, -1.0f)  // Negative Z
			};
			glm::vec3 ups[6] = 
			{
				glm::vec3(0.0f, 1.0f, 0.0f), // Positive X
				glm::vec3(0.0f, 1.0f, 0.0f), // Negative X
				glm::vec3(0.0f, 0.0f, 1.0f),  // Positive Y
				glm::vec3(0.0f, 0.0f, 1.0f), // Negative Y
				glm::vec3(0.0f, 1.0f, 0.0f), // Positive Z
				glm::vec3(0.0f, 1.0f, 0.0f)  // Negative Z
			};
			Camera* currentCamera = m_gdm->current_camera;
			for (uint32_t i = 0; i < SHADOW_MAP_CUBE_COUNT; i++)
			{
				m_shadowMatrix[i].splitDepth = 0.0f;
				glm::mat4 lightPerMatrix = glm::perspective(90.0f,1.0f, currentCamera->getNear(), currentCamera->getFar());
				m_shadowMatrix[i].projview = lightPerMatrix * glm::lookAt(m_ubl.position, m_ubl.position+directions[i], ups[i]);
				m_shadowMatrix[i].pos = m_ubl.position;
			}
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboShadow);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, m_ubl.shadowId * sizeof(ShadowMatrix), sizeof(ShadowMatrix)* m_shadowMatrix.size(), m_shadowMatrix.data());		
	}

	bool Lights::getshadow() const
	{
		return m_shadow;
	}

	void Lights::setRange(float r)
	{
		m_ubl.range = r;
		mapMemory();
	}

	void Lights::setSpotAngle(float r)
	{
		m_ubl.spotAngle = r;
		mapMemory();
	}

	float Lights::getSpotAngle() const
	{
		return m_ubl.spotAngle;
	}

	float Lights::getRange() const
	{
		return m_ubl.range;
	}

	void Lights::setColors(glm::vec3 color)
	{
		m_ubl.color = color;
		mapMemory();
	}

	glm::vec3 Lights::getColors() const
	{
		return m_ubl.color;
	}

	int Lights::getStatus() const
	{
		return m_ubl.status;
	}

	unsigned int Lights::getIndex() const
	{
		return m_index;
	}

	void Lights::setIndex(unsigned int i)
	{
		m_index = i;
	}

	void Lights::setShadowIndex(unsigned int i)
	{
		m_ubl.shadowId = i;
		mapMemory();
	}

	unsigned int Lights::getShadowIndex() const
	{
		return m_ubl.shadowId;
	}

	void Lights::onGUI()
	{
		GObject::onGUI();
		ImGui::TextColored(ImVec4(0.2f, 1, 0.2f, 1), "Light\n");
		if (ImGui::ColorEdit3("Color", (float*)&m_ubl.color))
		{
			setColors(m_ubl.color);
		}

		if (ImGui::DragFloat("Range", &m_ubl.range, 0.2f, 0.01f))
		{
			mapMemory();
		}

		if (m_ubl.status == 2)
		{
			if (ImGui::DragFloat("Angle", &m_ubl.spotAngle, 0.2f, 0.01f))
			{
				setSpotAngle(m_ubl.spotAngle);
			}
		}
		bool s = m_shadow;
		if (ImGui::Checkbox("Shadow", &s))
		{
			setshadow(s);
		}
	}

	Lights::~Lights()
	{
		if (m_shadow)
		{
			m_shadow = false;
			const ptrClass* pc = Engine::getPtrClassAddr();
			pc->lightManager->updateStorageShadow();
		}
	}
}