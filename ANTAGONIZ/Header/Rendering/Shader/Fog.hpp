#ifndef __FOG__
#define __FOG__

#include "ModulePP.hpp"
#include "glm/glm.hpp"
#include <vector>

namespace Ge
{
	class GraphiquePipeline;
	class Fog final : public ModulePP
	{
	public:
		bool initialize(GraphicsDataMisc* gdm);
		void resize(int width, int height);
		void compute(unsigned int frameBuffer, unsigned int texture, unsigned int depthtexture, ShapeBuffer* sb, PPSetting* settings);
		void onGui(PPSetting* target, PPSetting* current);
		void release();
	private:
		const ptrClass* m_pc;
		GraphicsDataMisc* m_gdm;
		GraphiquePipeline* m_fog = nullptr;
		unsigned int fog_size;
		unsigned int fog_color;
		unsigned int fog_min;
		unsigned int fog_max;
		unsigned int fog_mode;
		unsigned int fog_time;
		unsigned int fog_cd;	
	};
}

#endif // !__FOG__
