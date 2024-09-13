#include "RigidWraper.hpp"
#include "CommandQueue.hpp"
#include "RigidBody.hpp"

namespace Ge
{

	RigidWraper::RigidWraper(RigidBody* rb, CommandQueue* queue)
	{
		m_rb = rb;
		m_queue = queue;
	}

	void RigidWraper::Translate(glm::vec3 const& translate)
	{
		MethodCommand<RigidBody, glm::vec3 const&>* command = new MethodCommand<RigidBody, glm::vec3 const&>(m_rb, &RigidBody::Translate, translate);
		m_queue->push((Command*)command);
	}

	void RigidWraper::setPosition(glm::vec3 pos)
	{
		MethodCommand<RigidBody, glm::vec3>* command = new MethodCommand<RigidBody, glm::vec3>(m_rb, &RigidBody::setPosition, pos);
		m_queue->push((Command*)command);
	}

	void RigidWraper::setRotation(glm::quat rot)
	{
		MethodCommand<RigidBody, glm::quat>* command = new MethodCommand<RigidBody, glm::quat>(m_rb, &RigidBody::setRotation, rot);
		m_queue->push((Command*)command);
	}

	void RigidWraper::setEulerAngles(glm::vec3 eul)
	{
		MethodCommand<RigidBody, glm::vec3>* command = new MethodCommand<RigidBody, glm::vec3>(m_rb, &RigidBody::setEulerAngles, eul);
		m_queue->push((Command*)command);
	}

	glm::vec3 RigidWraper::getPosition() const
	{
		return m_rb->getPosition();
	}

	glm::quat RigidWraper::getRotation() const
	{
		return m_rb->getRotation();
	}

	glm::vec3 RigidWraper::getEulerAngles()
	{
		return m_rb->getEulerAngles();
	}

	glm::vec3 RigidWraper::getGravity() const
	{
		return m_rb->getGravity();
	}

	void RigidWraper::SetMass(float mass)
	{
		MethodCommand<RigidBody, float>* command = new MethodCommand<RigidBody, float>(m_rb, &RigidBody::SetMass, mass);
		m_queue->push((Command*)command);
	}

	void RigidWraper::setGravity(glm::vec3 const& gravity)
	{
		MethodCommand<RigidBody, glm::vec3 const&>* command = new MethodCommand<RigidBody, glm::vec3 const&>(m_rb, &RigidBody::setGravity, gravity);
		m_queue->push((Command*)command);
	}

	void RigidWraper::SetRestitution(float coef)
	{
		MethodCommand<RigidBody, float>* command = new MethodCommand<RigidBody, float>(m_rb, &RigidBody::SetRestitution, coef);
		m_queue->push((Command*)command);
	}

	void RigidWraper::SetLinearVelocity(const glm::vec3 velocity)
	{
		MethodCommand<RigidBody,const glm::vec3>* command = new MethodCommand<RigidBody, const glm::vec3>(m_rb, &RigidBody::SetLinearVelocity, velocity);
		m_queue->push((Command*)command);
	}

	void RigidWraper::BuildPhysics(CollisionShape* shape, bool hasInertia)
	{
		MethodCommand<RigidBody, CollisionShape*, bool>* command = new MethodCommand<RigidBody, CollisionShape*, bool>(m_rb, &RigidBody::BuildPhysics, shape, hasInertia);
		m_queue->push((Command*)command);
	}

	bool RigidWraper::IsInitialized()
	{
		return m_rb->IsInitialized();
	}

	glm::vec3 RigidWraper::GetLinearVelocity()
	{
		return m_rb->GetLinearVelocity();
	}

	void RigidWraper::ApplyImpulse(glm::vec3 impulse, glm::vec3 real_pos)
	{
		MethodCommand<RigidBody, glm::vec3, glm::vec3>* command = new MethodCommand<RigidBody, glm::vec3, glm::vec3>(m_rb, &RigidBody::ApplyImpulse, impulse,real_pos);
		m_queue->push((Command*)command);
	}

	void RigidWraper::forceActivationState(int newState)
	{
		MethodCommand<RigidBody, int>* command = new MethodCommand<RigidBody, int>(m_rb, &RigidBody::forceActivationState, newState);
		m_queue->push((Command*)command);
	}

	void RigidWraper::SetSleepingThreshold(float linear, float angular)
	{
		MethodCommand<RigidBody, float, float>* command = new MethodCommand<RigidBody, float, float>(m_rb, &RigidBody::SetSleepingThreshold, linear, angular);
		m_queue->push((Command*)command);
	}

	void RigidWraper::lockRotation()
	{
		MethodCommand<RigidBody>* command = new MethodCommand<RigidBody>(m_rb, &RigidBody::lockRotation);
		m_queue->push((Command*)command);
	}

	void RigidWraper::onGUI()
	{

	}

	RigidBody* RigidWraper::getRigidBody() const
	{
		return m_rb;
	}
}