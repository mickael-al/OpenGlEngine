#include "SettingManager.hpp"

namespace Ge
{
    double SettingManager::getFps() const
    {
        return m_settingInfo.m_fps;
    }

    void SettingManager::setFps(double fps)
    {
        m_settingInfo.m_fps = fps;
    }

    const char *SettingManager::getName() const
    {
        return m_settingInfo.m_name;
    }

    void SettingManager::setName(const char *name)
    {
        m_settingInfo.m_name = name;
    }
    double SettingManager::getWindowHeight() const
    {

        return m_settingInfo.m_sizeHeightWindow;
    }

    void SettingManager::setWindowHeight(double height)
    {

        m_settingInfo.m_sizeHeightWindow = height;
    }

    double SettingManager::getWindowWidth() const
    {

        return m_settingInfo.m_sizeWidthWindow;
    }
    void SettingManager::setWindowWidth(double Width)
    {

        m_settingInfo.m_sizeWidthWindow = Width;
    }
	glm::vec3 SettingManager::getGravity() const
    {
        return m_settingInfo.m_gravity;
    }
    void SettingManager::setGravity(glm::vec3 gravity)
    {

        m_settingInfo.m_gravity = gravity;
    }
    void SettingManager::setVersion(Version v)
    {

        m_settingInfo.m_version = v;
    }

    Version SettingManager::getVersion() const
    {
        return m_settingInfo.m_version;
    }

    void SettingManager::setClearColor(glm::vec4 color)
    {
        m_settingInfo.m_clearColor = color;
    }

	const glm::vec4 & SettingManager::getClearColor() const
    {
        return m_settingInfo.m_clearColor;
    }

    void SettingManager::setGamma(float gamma)
    {
        m_settingInfo.m_gamma = gamma;
    }

    float SettingManager::getGamma() const
    {
        return m_settingInfo.m_gamma;
    }

    void SettingManager::setAmbient(float ambiant)
    {
        m_settingInfo.m_ambient = ambiant;
    }

    float SettingManager::getAmbient() const
    {
        return m_settingInfo.m_ambient;
    }

    void SettingManager::setEditor(bool state)
    {
        m_settingInfo.m_editor = state;
    }

    bool SettingManager::getEditor() const
    {
        return m_settingInfo.m_editor;
    }

    void SettingManager::setEditorPath(const char* path)
    {
        m_settingInfo.projectEditorPath = path;
    }

    const char* SettingManager::getEditorPath() const
    {
        return m_settingInfo.projectEditorPath;
    }

	void SettingManager::setIconPath(const char * path)
	{
		m_settingInfo.iconPath = path;
	}

	void SettingManager::setVSync(bool vsync)
	{
		m_settingInfo.m_vsync = vsync;
	}

    bool SettingManager::getVsync() const
	{
		return m_settingInfo.m_vsync;
	}

	float const * SettingManager::getFramerate() const
	{
		return m_settingInfo.m_framerate;
	}

	void SettingManager::setFramerate(float * fr)
	{
		m_settingInfo.m_framerate = fr;
	}

    float* SettingManager::getGammaAddr()
    {
        return &m_settingInfo.m_gamma;
    }

	const char * SettingManager::getIconPath() const
	{
		return m_settingInfo.iconPath;
	}
}