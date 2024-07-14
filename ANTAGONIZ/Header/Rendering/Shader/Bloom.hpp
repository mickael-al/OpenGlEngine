#ifndef __BLOOM__
#define __BLOOM__

#include "ModulePP.hpp"
#include "glm/glm.hpp"
#include <vector>

struct bloomMip
{
	glm::vec2 size;
	glm::ivec2 intSize;
	unsigned int texture;
};

namespace Ge
{
	class GraphiquePipeline;
	class Bloom final : public ModulePP
	{
	public:
		bool initialize(GraphicsDataMisc* gdm);
		void resize(int width, int height);
		void compute(unsigned int frameBuffer, unsigned int texture, ShapeBuffer* sb, PPSetting* settings);
		void release();
	private:
		GraphicsDataMisc* m_gdm;
		int mipChainLength = 5;
		std::vector<bloomMip> mMipChain;
		GraphiquePipeline* m_downSampleShader;
		GraphiquePipeline* m_upSampleShader;
		GraphiquePipeline* m_tomeGammaShader;
		int srcResolutionLocation;
		int filterRadiusLocation;
		int gammaLocation;
		int exposureLocation;
		int intensityLocation;
		int thresholdLocation;
	};
}

#endif // !__BLOOM__
