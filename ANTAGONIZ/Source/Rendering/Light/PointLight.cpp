#include "PointLight.hpp"
#include "GraphicsDataMisc.hpp"

namespace Ge
{
	PointLight::PointLight(int index, GraphicsDataMisc * gdm) : Lights(index,gdm)
	{
		m_ubl.status = 1;
		m_shadowMatrix.resize(SHADOW_MAP_CUBE_COUNT);
	}
}