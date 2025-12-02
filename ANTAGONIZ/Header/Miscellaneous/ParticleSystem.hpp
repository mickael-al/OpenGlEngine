#ifndef __PARTICLE_SYSTEM__
#define __PARTICLE_SYSTEM__

#include <vector>
#include <glm/glm.hpp>
#include "DynamicGPUAllocator.hpp"
#include "ComputeShader.hpp"
#include "GraphiquePipeline.hpp"
#include "Textures.hpp"
#include "PointeurClass.hpp"

namespace Ge
{
    enum ParticleFlags : uint32_t
    {
        AttachToEmitPoint = 1u << 0,
        BoxEmit = 1u << 1,
        FollowTarget = 1u << 2,
        Rotate = 1u << 3,
        VelOrientation = 1u << 4,
    };

    struct ParticleBaseGroup
    {
        float lifeTimeMin;
        float lifeTimeMax;
        glm::vec3 emitPosition;
        glm::vec3 emitDirection;
        float spreadAngle;
        uint32_t flags;

        float velocityMin;
        float velocityMax;
        float scaleStart;
        float scaleEnd;

        glm::vec3 emitScaleMin;
        glm::vec3 emitScaleMax;

        glm::vec3 gravity;
        glm::vec3 externalForce;
        float drag;

        // Rotation
        float rotationMin;
        float rotationMax;
        float rotationScaleMin;
        float rotationScaleMax;

        // Color / Render
        glm::vec4 colorStart;
        glm::vec4 colorEnd;

        glm::vec3 target;
        float followStrength;
        float followRadius;

        ParticleBaseGroup()
        {
            // Durée de vie (1 à 2 secondes)
            lifeTimeMin = 1.0f;
            lifeTimeMax = 2.0f;

            // Position et direction d’émission
            emitPosition = glm::vec3(0.0f);
            emitDirection = glm::vec3(0.0f, 1.0f, 0.0f); // Vers le haut
            spreadAngle = 15.0f; // Petit cône
            flags = 0;

            // Scale initial / final
            scaleStart = 1.0f;
            scaleEnd = 0.0f; // se dissipe

            // Vitesse
            velocityMin = 8.0f;
            velocityMax = 10.0f;

            // Forces
            gravity = glm::vec3(0.0f, -9.81f, 0.0f);
            externalForce = glm::vec3(0.0f); // pas de vent
            drag = 0.1f; // amortissement léger

            // Rotation
            rotationMin = -360.0f;
            rotationMax = 360.0f;
            rotationScaleMin = 1.0f;
            rotationScaleMax = 2.0f;

            colorStart = glm::vec4(1.0f, 0.8f, 0.3f, 1.0f);
            colorEnd = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);

            emitScaleMin = glm::vec3(-1.0f, -1.0f, -1.0f);
            emitScaleMax = glm::vec3(1.0f, 1.0f, 1.0f);
            target = glm::vec3(0, 0, 0);
            followStrength = 0.1f;
            followRadius = 2.0f;
        }
    };

    struct Particle
    {
        glm::vec3 position;
        uint16_t cr;
        uint16_t cg;
        uint16_t cb;
        uint16_t ca;
        float time;
        uint16_t scale;
        uint16_t vx;
        uint16_t vy;
        uint16_t vz;
    };

    struct ParticleGroup
    {        
        size_t offsetSettings;
        size_t countSettings;
        size_t offsetData;
        size_t countData;        
        ComputeShader* cs = nullptr;
        GraphiquePipeline* pipeline = nullptr;
        Textures* texture = nullptr;
    };

    class ParticleSystem
    {
    public:
        ParticleSystem();
        ~ParticleSystem();

        size_t createGroup(void * settings,size_t sizeSettings, size_t nbParticule, Textures* tex, ComputeShader* cs = nullptr, GraphiquePipeline* gp = nullptr);
        void updateGroup(size_t groupID, void* settings, size_t sizeSettings);
        void removeGroup(size_t groupID);
        void render(glm::vec2 invRes);
    private:
        const ptrClass* m_pc = nullptr;
        DynamicGPUAllocator * m_allocator;
        std::vector<ParticleGroup> m_groups;
        unsigned int m_vao = 0;
        GraphiquePipeline* m_baseRender = nullptr;
        ComputeShader * m_baseCs = nullptr;
        ComputeShader * m_init = nullptr;
    };
}

#endif //!__PARTICLE_SYSTEM__