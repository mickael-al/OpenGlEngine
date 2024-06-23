#ifndef __PHYSICS_WRAPER__
#define __PHYSICS_WRAPER__

#include <glm/glm.hpp>
#include <vector>
#include "Behaviour.hpp"

namespace Ge
{
    class CommandQueue;
    class PhysicsEngine;
    class RigidBody;
    class CollisionBody;
    class Muscle;
    class CollisionShape;
    class Model;
    class ShapeBuffer;
    class GObject;
    class Materials;
    class GraphiquePipeline;
    class RigidWraper;
    class CollisionWraper;
    class PhysicsWraper : public Behaviour
    {
    public:
        PhysicsWraper(PhysicsEngine*pe, CommandQueue* queue);
        RigidWraper* AllocateRigidbody(CollisionShape* shape, bool hasInertia = true);
        std::vector<RigidWraper*> AllocateRigidbody(CollisionShape* shape, int nb, bool hasInertia = true);
        CollisionWraper* AllocateCollision(CollisionShape* shape);
        std::vector<CollisionWraper*> AllocateCollision(CollisionShape* shape, int nb);
        Muscle* AllocateMuscle(RigidBody* rb1, RigidBody* rb2, float degres, float scale = 1.0f, bool adaptePos = true);
        void ReleaseMuscle(Muscle* pBody);
        void AddRigidbody(RigidWraper* pbody, int group = 1, int mask = -1);
        void ReleaseRigidbody(RigidWraper* pBody);
        void AddCollision(CollisionWraper* body);
        void ReleaseCollision(CollisionWraper* pBody);
        bool raycast(const glm::vec3* start, const glm::vec3* end, glm::vec3* hitPoint);
        void DebugDrawCollider();
        void DebugClearCollider();
        void start();
        void fixedUpdate();
        void update();
        void stop();
        void onGUI();
    private:        
        PhysicsEngine* m_pe;
        CommandQueue* m_queue;
        bool m_debugDraw =false;
        std::vector<Model*> m_models;
        ShapeBuffer * m_shapeBuffer;
        Materials* m_material;
        GraphiquePipeline* m_cubeShader;
    };
}

#endif// !__PHYSICS_WRAPER__