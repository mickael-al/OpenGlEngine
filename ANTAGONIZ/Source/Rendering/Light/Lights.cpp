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
	}

	void Lights::setshadow(bool state)
	{
		m_shadow = state;
		const ptrClass * pc = Engine::getPtrClassAddr();
		pc->lightManager->updateStorageShadow();
	}

	std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
	{
		const auto inv = glm::inverse(proj * view);

		std::vector<glm::vec4> frustumCorners;
		for (unsigned int x = 0; x < 2; ++x)
		{
			for (unsigned int y = 0; y < 2; ++y)
			{
				for (unsigned int z = 0; z < 2; ++z)
				{
					const glm::vec4 pt =
						inv * glm::vec4(
							2.0f * x - 1.0f,
							2.0f * y - 1.0f,
							2.0f * z - 1.0f,
							1.0f);
					frustumCorners.push_back(pt / pt.w);
				}
			}
		}

		return frustumCorners;
	}

	void Lights::mapMemoryShadow()
	{					
		if (m_ubl.status == 0)
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

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboShadow);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, m_ubl.shadowId * sizeof(ShadowMatrix), sizeof(ShadowMatrix)* m_shadowMatrix.size(), m_shadowMatrix.data());		
	}


	/*void Lights::mapMemoryShadow()
	{
		if (m_ubl.status == 0)
		{
			m_ubl.direction = getDirection();
			Camera* currentCamera = m_gdm->current_camera;
			glm::vec3 center = glm::vec3(0, 0, 0);
			std::vector<glm::vec4> corners = getFrustumCornersWorldSpace(currentCamera->getProjectionMatrix(), currentCamera->getViewMatrix());
			for (const auto& v : corners)
			{
				center += glm::vec3(v);
			}
			center /= corners.size();

			const auto lightView = glm::lookAt(
				center + m_ubl.direction,
				center,
				glm::vec3(0.0f, 1.0f, 0.0f)
			);
			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::lowest();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::lowest();
			float minZ = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::lowest();
			for (const auto& v : corners)
			{
				const auto trf = lightView * v;
				minX = std::min(minX, trf.x);
				maxX = std::max(maxX, trf.x);
				minY = std::min(minY, trf.y);
				maxY = std::max(maxY, trf.y);
				minZ = std::min(minZ, trf.z);
				maxZ = std::max(maxZ, trf.z);
			}
			constexpr float zMult = 10.0f;
			if (minZ < 0)
			{
				minZ *= zMult;
			}
			else
			{
				minZ /= zMult;
			}
			if (maxZ < 0)
			{
				maxZ /= zMult;
			}
			else
			{
				maxZ *= zMult;
			}

			const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

			lightProjection * lightView;
		}
	}*/

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
	}

	Lights::~Lights()
	{
		
	}
}