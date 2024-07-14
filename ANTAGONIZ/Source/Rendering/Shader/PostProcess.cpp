#include "PostProcess.hpp"
#include "ShapeBuffer.hpp"
#include "Bloom.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"

namespace Ge
{
	bool PostProcess::initialize(GraphicsDataMisc* gdm)
	{
		m_gdm = gdm;
		m_setting = new PPSetting();
		m_setting->gamma = Engine::getPtrClassAddr()->settingManager->getGammaAddr();
		m_modules.push_back(new Bloom());
		for (int i = 0; i < m_modules.size(); i++)
		{
			m_modules[i]->initialize(gdm);
		}
		return true;
	}

	void PostProcess::compute(unsigned int frameBuffer, unsigned int texture, ShapeBuffer* sb)
	{
		for (int i = 0; i < m_modules.size(); i++)
		{
			m_modules[i]->compute(frameBuffer, texture, sb, m_setting);
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