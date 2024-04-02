#ifndef __ENGINE_RENDERING_ENGINE__
#define __ENGINE_RENDERING_ENGINE__

#include "Debug.hpp"
struct ptrClass;

namespace Ge
{
    class RenderingEngine
    {
    public:
        RenderingEngine();
        bool initialize(ptrClass * p_ptrClass);
        void release();
        void drawFrame();
    private:
		ptrClass * m_ptrClass;        
    };
}

#endif //__ENGINE_RENDERING_ENGINE__