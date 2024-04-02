#include "RenderingEngine.hpp"

namespace Ge
{
    RenderingEngine::RenderingEngine()
    {

    }

    bool RenderingEngine::initialize(ptrClass *p_ptrClass)
    {
        if (p_ptrClass == nullptr)
        {
            Debug::Error("ptrClass nullptr RenderingEngine");
            return false;
        }

      /*  if (!RenderingEngine::m_computeShaderManager.initialize(&m_vulkanMisc))
        {
            Debug::INITFAILED("ComputeShaderManager");
            return false;
        }
        Debug::INITSUCCESS("RenderingEngine");*/

        return true;
    }

    void RenderingEngine::release()
    {
 
        Debug::RELEASESUCCESS("RenderingEngine");
    }

    void RenderingEngine::drawFrame()
    {		
		//m_cameraManager.updateFlyCam();
		
    }
}