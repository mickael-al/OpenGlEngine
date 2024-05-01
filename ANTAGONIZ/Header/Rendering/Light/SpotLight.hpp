#ifndef __ENGINE_SPOT_LIGHT__
#define __ENGINE_SPOT_LIGHT__

#include "Lights.hpp"

namespace Ge
{
	class SpotLight : public Lights
	{
	public:
		SpotLight(int index, GraphicsDataMisc * gdm);
	};
}

#endif //__ENGINE_SPOT_LIGHT__