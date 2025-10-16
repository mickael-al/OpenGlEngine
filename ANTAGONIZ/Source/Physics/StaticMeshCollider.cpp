#include "StaticMeshCollider.hpp"

namespace Ge
{
    StaticMeshCollider::StaticMeshCollider(btDynamicsWorld* world, btBvhTriangleMeshShape* meshShape) : m_world(world), m_shape(meshShape), m_body(nullptr)
    {
        if (!m_world || !m_shape)
        {
            return;
        }

        btTransform transform;
        transform.setIdentity();
        btScalar mass(0.0f);
        btVector3 inertia(0, 0, 0);
        m_motionState = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_motionState, m_shape, inertia);
        m_body = new btRigidBody(rbInfo);
        m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

        m_world->addRigidBody(m_body);
    }

    StaticMeshCollider::~StaticMeshCollider()
    {
        if (m_body && m_world)
        {
            m_world->removeRigidBody(m_body);
        }

        delete m_body;
        delete m_motionState;
    }

}
