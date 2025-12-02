#include "glcore.hpp"
#include "ParticleManager.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"
#include "GraphiquePipeline.hpp"

#include "Debug.hpp"
#include "GraphicsDataMisc.hpp"


namespace Ge
{
	bool ParticleManager::initialize(GraphicsDataMisc* gdm)
	{
		m_gdm = gdm;
		Debug::INITSUCCESS("ParticleManager");
		return true;
	}

	void ParticleManager::release()
	{
		Debug::RELEASESUCCESS("ParticleManager");
	}

	void ParticleManager::create(unsigned int size, Materials* mat)
	{

	}
}