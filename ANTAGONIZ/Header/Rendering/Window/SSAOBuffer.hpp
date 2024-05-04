#ifndef __SSAO_BUFFER__
#define __SSAO_BUFFER__

#include "Initializer.hpp"
#include <vector>
#include "glm/glm.hpp"

namespace Ge
{
	class SSAOBuffer final : public InitializerAPI
	{
	protected:
		friend class RenderingEngine;
		bool initialize(GraphicsDataMisc* gdm);
		unsigned int getNoiseTexture() const;
		unsigned int getSSBO() const;
		void release();
	private:
		GraphicsDataMisc* m_gdm;
		std::vector<glm::vec3> m_ssaoKernel;
		std::vector<glm::vec3> m_ssaoNoise;
		unsigned int m_noiseTexture;
		unsigned int m_ssbo;
	};
}
#endif // !__SSAO_BUFFER__
