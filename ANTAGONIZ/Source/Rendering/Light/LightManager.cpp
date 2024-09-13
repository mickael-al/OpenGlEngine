#include "glcore.hpp"
#include "LightManager.hpp"
#include "Lights.hpp"
#include "PointLight.hpp"
#include "DirectionalLight.hpp"
#include "SpotLight.hpp"
#include "Debug.hpp"
#include "GraphicsDataMisc.hpp"
#include <algorithm>
#include "ShadowMatrix.hpp"

bool LightManager::initialize(GraphicsDataMisc * gdm)
{
	m_gdm = gdm;
	glGenBuffers(1, &m_ssbo);
	glGenBuffers(1, &m_ssboShadow);	
	m_gdm->str_ssbo.str_light = m_ssbo;
	m_gdm->str_ssbo.str_shadow = m_ssboShadow;
	glGenTextures(1, &m_textureShadowArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureShadowArray);	
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, TEXTURE_DIM, TEXTURE_DIM, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	Debug::INITSUCCESS("LightManager");
	return true;
}

unsigned int LightManager::getSsboShadow() const
{
	return m_ssboShadow;
}

unsigned int LightManager::getTextureShadowArray() const
{
	return m_textureShadowArray;
}

const std::vector<unsigned int>& LightManager::getFrameShadowBuffer() const
{
	return m_frameBufferDepthShadow;
}

void LightManager::release()
{
	for (int i = 0; i < m_lights.size(); i++)
	{
		m_pool.deleteObject(m_lights[i]);
	}
	m_lights.clear();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glDeleteBuffers(1, &m_ssbo);
	
	glDeleteFramebuffers(m_frameBufferDepthShadow.size(), m_frameBufferDepthShadow.data());
	glDeleteTextures(1, &m_textureShadowArray);
	Debug::RELEASESUCCESS("LightManager");
}

void LightManager::updateShadowCascadeMatrix()
{
	for (int i = 0; i < m_lights.size(); i++)
	{
		if (m_lights[i]->getshadow() && m_lights[i]->getStatus() == 0)
		{			
			m_lights[i]->mapMemoryShadow();			
		}
	}
}

void LightManager::updateStorageShadow()
{	
	int countTotal = 0;
	for (int i = 0; i < m_lights.size(); i++)
	{
		if (m_lights[i]->getshadow())
		{			
			m_lights[i]->setShadowIndex(countTotal);
			countTotal += m_lights[i]->getStatus() == 0 ? SHADOW_MAP_CASCADE_COUNT : (m_lights[i]->getStatus() == 1 ? SHADOW_MAP_CUBE_COUNT : SHADOW_MAP_SPOT_COUNT);
		}
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboShadow);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ShadowMatrix) * countTotal, nullptr, GL_DYNAMIC_DRAW);
	for (int i = 0; i < m_lights.size(); i++)
	{
		if (m_lights[i]->getshadow())
		{
			m_lights[i]->mapMemoryShadow();
		}
	}
	glDeleteTextures(1, &m_textureShadowArray);
	glGenTextures(1, &m_textureShadowArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureShadowArray);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, TEXTURE_DIM, TEXTURE_DIM, countTotal, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (countTotal < m_frameBufferDepthShadow.size())
	{
		int reste = m_frameBufferDepthShadow.size() - countTotal;
		for (int i = 0; i < reste; i++)
		{			
			glDeleteFramebuffers(1, &m_frameBufferDepthShadow[m_frameBufferDepthShadow.size() - 1]);
			m_frameBufferDepthShadow.pop_back();
		}
	}

	unsigned int frameBufferID = 0;
	
	for (int i = 0; i < countTotal; ++i)
	{
		if (i >= m_frameBufferDepthShadow.size())
		{
			glGenFramebuffers(1, &frameBufferID);
			glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);

			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_textureShadowArray,0, i);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
			{
				Debug::Error("Erreur lors de la creation du framebuffer.");
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			m_frameBufferDepthShadow.push_back(frameBufferID);
		}	
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureShadowArray);
}

void LightManager::updateStorage()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(UniformBufferLight)*m_lights.size(), nullptr, GL_DYNAMIC_DRAW);
	for (int i = 0; i < m_lights.size(); i++)
	{
		m_lights[i]->setIndex(i);
		m_lights[i]->mapMemory();
	}
}

SpotLight * LightManager::createSpotLight(glm::vec3 position, glm::vec3 color, glm::vec3 euler, float angle, std::string name)
{
	SpotLight * light = m_pool.newDerivedObject<SpotLight>(m_lights.size(), m_gdm);
	m_lights.push_back(light);
	m_gdm->str_dataMisc.lightCount = m_lights.size();
	updateStorage();
	light->setPosition(position);
	light->setEulerAngles(euler);
	light->setColors(color);
	light->setSpotAngle(angle);
	light->setName(name);
	return (SpotLight *)light;
}

DirectionalLight * LightManager::createDirectionalLight(glm::vec3 euler, glm::vec3 color, std::string name)
{	
	DirectionalLight * light = m_pool.newDerivedObject<DirectionalLight>(m_lights.size(), m_gdm);
	m_lights.push_back(light);
	m_gdm->str_dataMisc.lightCount = m_lights.size();
	updateStorage();
	light->setColors(color);
	light->setEulerAngles(euler);
	light->setName(name);
	return (DirectionalLight *)light;
}

PointLight * LightManager::createPointLight(glm::vec3 position, glm::vec3 color, std::string name)
{	
	PointLight * light = m_pool.newDerivedObject<PointLight>(m_lights.size(), m_gdm);
	m_lights.push_back(light);
	m_gdm->str_dataMisc.lightCount = m_lights.size();
	updateStorage();
	light->setPosition(position);
	light->setColors(color);
	light->setName(name);	
	return (PointLight *)light;
}

void LightManager::destroyLight(Lights *light)
{
	m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), light), m_lights.end());
	m_pool.deleteObject(light);
	m_gdm->str_dataMisc.lightCount = m_lights.size();
	updateStorage();
}