#include "SpotLight.hpp"
#include "GraphicsDataMisc.hpp"

namespace Ge
{
	SpotLight::SpotLight(int index, GraphicsDataMisc * gdm) : Lights(index, gdm)
	{
		m_ubl.status = 2;
		m_shadowMatrix.resize(SHADOW_MAP_SPOT_COUNT);
	}
}