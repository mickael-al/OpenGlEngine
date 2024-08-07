#ifndef __MODULE_POST_PROCESS__
#define __MODULE_POST_PROCESS__

#include "Initializer.hpp"
struct GraphicsDataMisc;

namespace Ge
{
	struct PPSetting
	{
		bool bloom;
		float bloom_filter;
		float bloom_threshold;
		float bloom_intensity;
		float exposure;
		float* gamma;
		PPSetting()
		{
			bloom = true;
			bloom_filter = 0.001f;
			bloom_threshold = 0.01f;
			bloom_intensity = 1.0f;
			exposure = 1.0;
			gamma = nullptr;
		}
		void copy(PPSetting pp)
		{
			bloom = pp.bloom;
			bloom_filter = pp.bloom_filter;
			bloom_threshold = pp.bloom_threshold;
			bloom_intensity = pp.bloom_intensity;
			exposure = pp.exposure;			
		}
	};
	class ShapeBuffer;
	class ModulePP : public InitializerAPI
	{
	public:		
		virtual bool initialize(GraphicsDataMisc* gdm) = 0;
		virtual void release() = 0;
		virtual void resize(int width, int height) = 0;
		virtual void compute(unsigned int texture, unsigned int frameBuffer, ShapeBuffer * sb, PPSetting * settings) = 0;
	};
}

#endif //!__MODULE_POST_PROCESS__