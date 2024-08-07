#include "glcore.hpp"
#include "ShaderDataMisc.hpp"
#include "GraphicsDataMisc.hpp"
#include "Debug.hpp"
#include "Time.hpp"
#include "SettingManager.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"
#include "Camera.hpp"

bool ShaderDataMisc::initialize(GraphicsDataMisc * gdm)
{
	m_gdm = gdm;
	glGenBuffers(1, &m_ssbo);
	m_gdm->str_ssbo.str_misc = m_ssbo;
	updateStorage();
	ptrClass pc = Engine::getPtrClass();
	m_time = pc.time;
	m_settingManager = pc.settingManager;
	Debug::INITSUCCESS("ShaderDataMisc");
	return true;
}

void ShaderDataMisc::release()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glDeleteBuffers(1, &m_ssbo);
}

void ShaderDataMisc::updateStorage()
{	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(UniformBufferDiver), nullptr, GL_DYNAMIC_DRAW);
}

void ShaderDataMisc::update(Camera * cam)
{
	m_ubd.u_time = m_time->getTime();
	m_ubd.maxLight = m_gdm->str_dataMisc.lightCount;
	m_ubd.gamma = m_settingManager->getGamma();
	m_ubd.ambiant = m_settingManager->getAmbient();
	m_ubd.fov = cam->getFieldOfView();
	m_ubd.ortho = cam->getOrtho();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(UniformBufferDiver), &m_ubd);
}