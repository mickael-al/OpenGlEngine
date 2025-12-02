#include "glcore.hpp"
#include "PostProcess.hpp"
#include "ShapeBuffer.hpp"
#include "Bloom.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"
#include "Fog.hpp"
//#include "OilPainting.hpp"

namespace Ge
{
	bool PostProcess::initialize(GraphicsDataMisc* gdm)
	{
		m_gdm = gdm;
		m_setting = new PPSetting();
		m_setting->gamma = Engine::getPtrClassAddr()->settingManager->getGammaAddr();
		m_modules.push_back(new Fog());
		m_modules.push_back(new Bloom());
		//m_modules.push_back(new OilPainting());
		for (int i = 0; i < m_modules.size(); i++)
		{
			m_modules[i]->initialize(gdm);
		}
		return true;
	}

	std::vector<ModulePP*>& PostProcess::getModules()
	{
		return m_modules;
	}

	void PostProcess::onGui(PPSetting* t)
	{
		if (ImGui::DragFloat("Gamma", m_setting->gamma))
		{
			if (t != nullptr) { t->gamma = m_setting->gamma; }
		}
		if (ImGui::DragFloat("Exposure##Bloom", &m_setting->exposure, 0.1f))
		{
			if (t != nullptr) { t->exposure = m_setting->exposure; }
		}
		for (int i = 0; i < m_modules.size(); i++)
		{
			m_modules[i]->onGui(t, m_setting);
		}
	}

	void PostProcess::compute(unsigned int frameBuffer, unsigned int texture, unsigned int depthTexture, ShapeBuffer* sb)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);//fix ghost triangle bug !!!
		for (int i = 0; i < m_modules.size(); i++)
		{
			m_modules[i]->compute(frameBuffer, texture, depthTexture, sb, m_setting);
		}
	}

	void PostProcess::resize(int width, int height)
	{
		for (int i = 0; i < m_modules.size(); i++)
		{
			m_modules[i]->resize(width, height);
		}
	}

	void PostProcess::release()
	{
		for (int i = 0; i < m_modules.size(); i++)
		{
			m_modules[i]->release();
			delete m_modules[i];
		}		
		m_modules.clear();
		delete m_setting;
	}

	PPSetting* PostProcess::getPPSetting()
	{
		return m_setting;
	}
}