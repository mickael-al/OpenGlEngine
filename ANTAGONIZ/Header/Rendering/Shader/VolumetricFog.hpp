#ifndef __VOLUMETRIC_FOG__
#define __VOLUMETRIC_FOG__

#include "ModulePP.hpp"
#include "glm/glm.hpp"
#include <vector>

namespace Ge
{
	class GraphiquePipeline;
	class VolumetricFog final : public ModulePP
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
		GraphiquePipeline* m_vfog = nullptr;
		unsigned int fog_color;
		unsigned int fog_time;
		unsigned int fog_cd;
		unsigned int fog_invProjection;
		unsigned int fog_invView;
		unsigned int fog_cp;
	};
}

#endif // !__VOLUMETRIC_FOG__
