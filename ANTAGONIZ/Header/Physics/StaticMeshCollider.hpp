#ifndef __STATIC_MESH_COLLIDER__
#define __STATIC_MESH_COLLIDER__

#include <btBulletDynamicsCommon.h>

namespace Ge
{
    class StaticMeshCollider
    {
    public:
        StaticMeshCollider(btDynamicsWorld* world, btBvhTriangleMeshShape* meshShape);
        ~StaticMeshCollider();
    private:
        btDynamicsWorld* m_world;
        btBvhTriangleMeshShape* m_shape;
        btRigidBody* m_body;
        btMotionState* m_motionState;
    };
}

#endif //! __STATIC_MESH_COLLIDER__
