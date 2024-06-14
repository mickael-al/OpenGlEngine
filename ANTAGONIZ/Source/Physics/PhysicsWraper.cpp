#include "PhysicsWraper.hpp"
#include "CommandQueue.hpp"
#include "PhysicsEngine.hpp"

namespace Ge
{
	PhysicsWraper::PhysicsWraper(PhysicsEngine* pe, CommandQueue* queue)
	{
		m_pe = pe;
		m_queue = queue;
	}

	RigidBody* PhysicsWraper::AllocateRigidbody(CollisionShape* shape, bool hasInertia)
	{
		std::promise<RigidBody*> * promise = new std::promise<RigidBody*>();
		auto future = promise->get_future();
		MethodCommandReturn<PhysicsEngine, RigidBody*, CollisionShape*, bool> * command = new MethodCommandReturn<PhysicsEngine, RigidBody*, CollisionShape*, bool>(m_pe, &PhysicsEngine::AllocateRigidbody, promise, shape, hasInertia);
		m_queue->push((Command *)command);
		RigidBody * rb = future.get();
		delete promise;
		return rb;
	}

	CollisionBody* PhysicsWraper::AllocateCollision(CollisionShape* shape)
	{
		std::promise<CollisionBody*>* promise = new std::promise<CollisionBody*>();
		auto future = promise->get_future();
		MethodCommandReturn<PhysicsEngine, CollisionBody*, CollisionShape*>* command = new MethodCommandReturn<PhysicsEngine, CollisionBody*, CollisionShape*>(m_pe, &PhysicsEngine::AllocateCollision, promise, shape);
		m_queue->push((Command*)command);
		CollisionBody* cb = future.get();
		delete promise;
		return cb;
	}

	Muscle* PhysicsWraper::AllocateMuscle(RigidBody* rb1, RigidBody* rb2, float degres, float scale, bool adaptePos)
	{
		std::promise<Muscle*>* promise = new std::promise<Muscle*>();
		auto future = promise->get_future();
		MethodCommandReturn<PhysicsEngine, Muscle*, RigidBody*, RigidBody*, float, float, bool>* command = new MethodCommandReturn<PhysicsEngine, Muscle*, RigidBody*, RigidBody*, float, float, bool>(m_pe, &PhysicsEngine::AllocateMuscle, promise, rb1, rb2, degres, scale, adaptePos);
		m_queue->push((Command*)command);
		Muscle* cb = future.get();
		delete promise;
		return cb;
	}

	void PhysicsWraper::ReleaseMuscle(Muscle* pBody)
	{
		MethodCommand<PhysicsEngine, Muscle*>* command = new MethodCommand<PhysicsEngine, Muscle*>(m_pe, &PhysicsEngine::ReleaseMuscle, pBody);
		m_queue->push((Command*)command);
	}

	void PhysicsWraper::AddRigidbody(RigidBody* body, int group, int mask)
	{
		MethodCommand<PhysicsEngine, RigidBody*, int, int>* command = new MethodCommand<PhysicsEngine, RigidBody*, int, int>(m_pe, &PhysicsEngine::AddRigidbody, body, group, mask);
		m_queue->push((Command*)command);
	}

	void PhysicsWraper::ReleaseRigidbody(RigidBody* pBody)
	{
		MethodCommand<PhysicsEngine, RigidBody*>* command = new MethodCommand<PhysicsEngine, RigidBody*>(m_pe, &PhysicsEngine::ReleaseRigidbody, pBody);
		m_queue->push((Command*)command);
	}

	void PhysicsWraper::AddCollision(CollisionBody* body)
	{
		MethodCommand<PhysicsEngine, CollisionBody*>* command = new MethodCommand<PhysicsEngine, CollisionBody*>(m_pe, &PhysicsEngine::AddCollision, body);
		m_queue->push((Command*)command);
	}

	void PhysicsWraper::ReleaseCollision(CollisionBody*& pBody)
	{
		MethodCommand<PhysicsEngine, CollisionBody*>* command = new MethodCommand<PhysicsEngine, CollisionBody*>(m_pe, &PhysicsEngine::AddCollision, pBody);
		m_queue->push((Command*)command);
	}

	bool PhysicsWraper::raycast(const glm::vec3 * start,const glm::vec3 * end, glm::vec3 * hitPoint)
	{
		std::promise<bool>* promise = new std::promise<bool>();
		auto future = promise->get_future();
		MethodCommandReturn<PhysicsEngine, bool, const glm::vec3 *, const glm::vec3 *, glm::vec3 *>* command = new MethodCommandReturn<PhysicsEngine, bool, const glm::vec3*, const glm::vec3*, glm::vec3*>(m_pe, &PhysicsEngine::raycast, promise, start, end, hitPoint);
		m_queue->push((Command*)command);
		bool cb = future.get();
		delete promise;
		return cb;
	}
}