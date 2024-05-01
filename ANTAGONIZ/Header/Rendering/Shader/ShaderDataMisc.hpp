#ifndef __ENGINE_SHADER_DATA_MISC__
#define __ENGINE_SHADER_DATA_MISC__

#include "Initializer.hpp"
#include "Manager.hpp"
#include "UniformBufferDiver.hpp"
#include "SettingManager.hpp"

namespace Ge
{
	class Time;	
	class Camera;
}

namespace Ge
{
	class ShaderDataMisc final : public InitializerAPI, public Manager
	{
	protected:
		friend class RenderingEngine;
		bool initialize(GraphicsDataMisc * gdm);
		void release();
		void updateStorage();
		void update(Camera * cam);
	private:		
		GraphicsDataMisc * m_gdm;
		UniformBufferDiver m_ubd;
		Time * m_time;
		SettingManager * m_settingManager;
	};
}

#endif //!__ENGINE_SHADER_DATA_MISC__