#ifndef _PHYSICS_ENGINE_HPP__
#define _PHYSICS_ENGINE_HPP__

#include "glm/glm.hpp"
#include <chrono>
#include "btBulletDynamicsCommon.h"
#include "Debug.hpp"
#include <vector>
#include <algorithm>

namespace Ge
{

class RigidBody;
class CollisionBody;
class Muscle;
class CollisionShape;
class GObject;
class CommandQueue;

class PhysicsEngine
{
public:
    bool Initialize(glm::vec3 const& gravity, CommandQueue* queue);
    void Update(float dt,bool engine = true);
    void Shutdown();
public:
    RigidBody* AllocateRigidbody(CollisionShape* shape, bool hasInertia = true);
    CollisionBody* AllocateCollision(CollisionShape* shape);
    Muscle* AllocateMuscle(RigidBody* rb1, RigidBody* rb2, float degres, float scale = 1.0f, bool adaptePos = true);
    void ReleaseMuscle(Muscle* pBody);
    void AddRigidbody(RigidBody* body,int group = 1,int mask = -1);
    void ReleaseRigidbody(RigidBody * pBody);
    void AddCollision(CollisionBody* body);
    void ReleaseCollision(CollisionBody* pBody);
    bool raycast(const glm::vec3* start, const glm::vec3* end, glm::vec3* hitPoint);
    const std::vector<RigidBody*> getRigidbody() const;
    const std::vector<CollisionBody*> getCollisionbody() const;
private:
    std::vector<RigidBody*> m_rigidbody;
    std::vector<CollisionBody*> m_collisionbody;
    std::vector<Muscle*> m_muscle;
    CommandQueue* m_queue;
    btDynamicsWorld* m_pDynamicWorld;
    btBroadphaseInterface               * m_pBroadphaseInterface;
    btCollisionDispatcher               * m_pCollisionDispatcher;
    btDefaultCollisionConfiguration     * m_pCollisionCongifuration;
    btSequentialImpulseConstraintSolver * m_pSequentialConstraintSolver;
};

static glm::vec3 toGlmVec3(const btVector3& vec)
{
    return glm::vec3(vec.getX(), vec.getY(), vec.getZ());
}

static btVector3 toBtVec3(const glm::vec3& vec)
{
    return btVector3(vec.x, vec.y, vec.z);
}

}

#endif

