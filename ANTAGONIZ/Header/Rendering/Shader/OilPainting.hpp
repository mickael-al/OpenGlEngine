#ifndef __OIL_PAINTING__
#define __OIL_PAINTING__

#include "ModulePP.hpp"
#include "glm/glm.hpp"
#include <vector>

namespace Ge
{
	class GraphiquePipeline;
	class OilPainting final : public ModulePP
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
		GraphiquePipeline* m_oilPainting = nullptr;
		unsigned int oilp_radius = 0;
		unsigned int oilp_size = 0;
		unsigned int m_fColor = 0;
	};
}

#endif // !__OIL_PAINTING__
