#include "DirectionalLight.hpp"
#include "GraphicsDataMisc.hpp"

namespace Ge
{
	DirectionalLight::DirectionalLight(int index, GraphicsDataMisc * gdm) : Lights(index, gdm)
	{
		m_ubl.status = 0;		
	}
}