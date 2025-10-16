#ifndef __HALO__
#define __HALO__

#include "ModulePP.hpp"
#include "glm/glm.hpp"
#include <vector>

namespace Ge
{
	class GraphiquePipeline;
	class Halo final : public ModulePP
	{
	public:
		bool initialize(GraphicsDataMisc* gdm);
		void resize(int width, int height);
		void compute(unsigned int frameBuffer, unsigned int texture, unsigned int depthtexture, ShapeBuffer* sb, PPSetting* settings);
		void onGui(PPSetting* target, PPSetting* current);
		void setData(glm::vec3 pos, glm::vec2 ra);
		void release();
	private:
		const ptrClass* m_pc;
		GraphicsDataMisc* m_gdm;
		GraphiquePipeline* m_halo = nullptr;
		unsigned int halo_planetPos;
		unsigned int halo_atmosphereRadius;
		unsigned int halo_invProjection;
		unsigned int halo_invView;
		unsigned int halo_cameraPos;
		unsigned int halo_cameraData;
		glm::vec3 m_halo_planetPos = glm::vec3(0.0f);
		float m_halo_atmosphereRadius = 1.0f;

	};
}

#endif // !__HALO__
