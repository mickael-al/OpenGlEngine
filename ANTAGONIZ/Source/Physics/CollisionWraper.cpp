#include "CollisionWraper.hpp"
#include "CommandQueue.hpp"
#include "CollisionBody.hpp"

namespace Ge
{
    CollisionWraper::CollisionWraper(CollisionBody* cb, CommandQueue* queue)
    {
        m_cb = cb;
        m_queue = queue;
    }

    void CollisionWraper::Build(CollisionShape* shape)
    {
        MethodCommand<CollisionBody, CollisionShape*>* command = new MethodCommand<CollisionBody, CollisionShape*>(m_cb, &CollisionBody::Build, shape);
        m_queue->push((Command*)command);
    }

    void CollisionWraper::setPosition(glm::vec3 pos)
    {
        MethodCommand<CollisionBody, glm::vec3>* command = new MethodCommand<CollisionBody, glm::vec3>(m_cb, &CollisionBody::setPosition, pos);
        m_queue->push((Command*)command);
    }

    void CollisionWraper::setRotation(glm::quat rot)
    {
        MethodCommand<CollisionBody, glm::quat>* command = new MethodCommand<CollisionBody, glm::quat>(m_cb, &CollisionBody::setRotation, rot);
        m_queue->push((Command*)command);
    }

    void CollisionWraper::setEulerAngles(glm::vec3 eul)
    {
        MethodCommand<CollisionBody, glm::vec3>* command = new MethodCommand<CollisionBody, glm::vec3>(m_cb, &CollisionBody::setEulerAngles, eul);
        m_queue->push((Command*)command);
    }

    void CollisionWraper::setScale(glm::vec3 scale)
    {
        MethodCommand<CollisionBody, glm::vec3>* command = new MethodCommand<CollisionBody, glm::vec3>(m_cb, &CollisionBody::setScale, scale);
        m_queue->push((Command*)command);
    }

    glm::vec3 CollisionWraper::getPosition() const
    {
        return m_cb->getPosition();
    }

    glm::quat CollisionWraper::getRotation() const
    {
        return m_cb->getRotation();
    }

    glm::vec3 CollisionWraper::getEulerAngles()
    {
        return m_cb->getEulerAngles();
    }

    glm::vec3 CollisionWraper::getScale() const
    {
        return m_cb->getScale();
    }

    void CollisionWraper::setFriction(float friction)
    {
        MethodCommand<CollisionBody, float>* command = new MethodCommand<CollisionBody, float>(m_cb, &CollisionBody::setFriction, friction);
        m_queue->push((Command*)command);
    }

    void CollisionWraper::setRestitution(float restitution)
    {
        MethodCommand<CollisionBody, float>* command = new MethodCommand<CollisionBody, float>(m_cb, &CollisionBody::setRestitution, restitution);
        m_queue->push((Command*)command);
    }

    bool CollisionWraper::IsInitialized()
    {
        return m_cb->IsInitialized();
    }

    const CollisionShape* CollisionWraper::getCollisionShape() const
    {
        return m_cb->getCollisionShape();
    }

    void CollisionWraper::mapMemory()
    {
        MethodCommand<CollisionBody>* command = new MethodCommand<CollisionBody>(m_cb, &CollisionBody::mapMemory);
        m_queue->push((Command*)command);
    }

    CollisionBody* CollisionWraper::getCollisionBody() const
    {
        return m_cb;
    }

}