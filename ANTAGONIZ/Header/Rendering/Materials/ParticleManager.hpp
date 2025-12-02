#ifndef __ENGINE_PARTICLE_MANAGER__
#define __ENGINE_PARTICLE_MANAGER__

#include "Initializer.hpp"
#include <vector>

namespace Ge
{
    class Materials;
    class ParticleManager final : public InitializerAPI
    {
    private:
        friend class RenderingEngine;
        bool initialize(GraphicsDataMisc* gdm);
        void release();
    public:
        void create(unsigned int size, Materials * mat);
    private:
        GraphicsDataMisc* m_gdm;
    };
}

#endif //!__ENGINE_PARTICLE_MANAGER__