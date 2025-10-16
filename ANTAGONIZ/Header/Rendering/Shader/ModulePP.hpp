#ifndef __MODULE_POST_PROCESS__
#define __MODULE_POST_PROCESS__

#include "Initializer.hpp"
#include <glm/glm.hpp>
struct GraphicsDataMisc;

namespace Ge
{
	struct PPSetting
	{
		bool fog;
		float fog_min_distance;
		float fog_max_distance;
		int fog_mode;
		glm::vec3 fog_color;
		bool bloom;
		float bloom_filter;
		float bloom_threshold;
		float bloom_intensity;
		float exposure;
		float* gamma;	
		bool oil_painting;
		unsigned int oilp_radius;		
		PPSetting()
		{
			bloom = true;
			bloom_filter = 0.001f;
			bloom_threshold = 0.01f;
			bloom_intensity = 1.0f;
			exposure = 1.0;
			gamma = nullptr;

			fog = false;
			fog_min_distance = 100.0f;
			fog_max_distance = 800.0f;
			fog_mode = 0;
			fog_color = glm::vec3(1,1,1);

			oil_painting = false;
			oilp_radius = 1;
		}
		void copy(PPSetting pp)
		{
			bloom = pp.bloom;
			bloom_filter = pp.bloom_filter;
			bloom_threshold = pp.bloom_threshold;
			bloom_intensity = pp.bloom_intensity;
			exposure = pp.exposure;			

			fog = pp.fog;
			fog_min_distance = pp.fog_min_distance;
			fog_max_distance = pp.fog_max_distance;
			fog_mode = pp.fog_mode;
			fog_color = pp.fog_color;

			oil_painting = pp.oil_painting;
			oilp_radius = pp.oilp_radius;
		}		
	};
	class ShapeBuffer;
	class ModulePP : public InitializerAPI
	{
	public:		
		virtual bool initialize(GraphicsDataMisc* gdm) = 0;
		virtual void release() = 0;
		virtual void resize(int width, int height) = 0;
		virtual void compute(unsigned int frameBuffer, unsigned int texture, unsigned int depthTexture, ShapeBuffer * sb, PPSetting * settings) = 0;
		virtual void onGui(PPSetting* target, PPSetting* current) = 0;
	};
}

#endif //!__MODULE_POST_PROCESS__