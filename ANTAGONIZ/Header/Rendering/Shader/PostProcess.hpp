#ifndef __POST_PROCCESS__
#define __POST_PROCCESS__

#include "Initializer.hpp"
#include "ModulePP.hpp"
#include <vector>

namespace Ge
{
	class ShapeBuffer;
	class PostProcess final : public InitializerAPI
	{
	public:
		PPSetting* getPPSetting();
	protected:
		friend class RenderingEngine;
		bool initialize(GraphicsDataMisc* gdm);		
		void compute(unsigned int frameBuffer, unsigned int texture, ShapeBuffer * fullScreenTriangle);
		void release();
	private:
		friend class Window;
		void resize(int width, int height);
	private:
		GraphicsDataMisc* m_gdm;
		std::vector<ModulePP*> m_modules;
		PPSetting* m_setting;
	};
}

#endif //!__POST_PROCCESS__