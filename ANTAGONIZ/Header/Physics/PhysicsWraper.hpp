#ifndef __PHYSICS_WRAPER__
#define __PHYSICS_WRAPER__

#include <glm/glm.hpp>

namespace Ge
{
    class CommandQueue;
    class PhysicsEngine;
    class RigidBody;
    class CollisionBody;
    class Muscle;
    class CollisionShape;
    class GObject;
    class PhysicsWraper
    {
    public:
        PhysicsWraper(PhysicsEngine*pe, CommandQueue* queue);
        RigidBody* AllocateRigidbody(CollisionShape* shape, bool hasInertia = true);
        CollisionBody* AllocateCollision(CollisionShape* shape);
        Muscle* AllocateMuscle(RigidBody* rb1, RigidBody* rb2, float degres, float scale = 1.0f, bool adaptePos = true);
        void ReleaseMuscle(Muscle* pBody);
        void AddRigidbody(RigidBody* body, int group = 1, int mask = -1);
        void ReleaseRigidbody(RigidBody* pBody);
        void AddCollision(CollisionBody* body);
        void ReleaseCollision(CollisionBody*& pBody);
        bool raycast(const glm::vec3* start, const glm::vec3* end, glm::vec3* hitPoint);
    private:
        PhysicsEngine* m_pe;
        CommandQueue* m_queue;
    };
}

#endif// !__PHYSICS_WRAPER__