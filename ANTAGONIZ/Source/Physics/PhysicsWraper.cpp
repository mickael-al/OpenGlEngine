#include "PhysicsWraper.hpp"
#include "CommandQueue.hpp"
#include "PhysicsEngine.hpp"
#include "RigidWraper.hpp"
#include "Engine.hpp"
#include "RigidBody.hpp"
#include "PointeurClass.hpp"
#include "Materials.hpp"
#include "Model.hpp"
#include "CollisionWraper.hpp"
#include "CollisionBody.hpp"

namespace Ge
{
	PhysicsWraper::PhysicsWraper(PhysicsEngine* pe, CommandQueue* queue)
	{
		m_pe = pe;
		m_queue = queue;
	}

	RigidWraper* PhysicsWraper::AllocateRigidbody(CollisionShape* shape, bool hasInertia)
	{
		std::promise<RigidBody*> * promise = new std::promise<RigidBody*>();
		auto future = promise->get_future();
		MethodCommandReturn<PhysicsEngine, RigidBody*, CollisionShape*, bool> * command = new MethodCommandReturn<PhysicsEngine, RigidBody*, CollisionShape*, bool>(m_pe, &PhysicsEngine::AllocateRigidbody, promise, shape, hasInertia);
		m_queue->push((Command *)command);
		RigidBody * rb = future.get();
		delete promise;
		return new RigidWraper(rb, m_queue);
	}

	std::vector<RigidWraper*> PhysicsWraper::AllocateRigidbody(CollisionShape* shape, int nb, bool hasInertia)
	{
		std::vector<std::promise<RigidBody*>*> promises(nb);
		std::vector<std::future<RigidBody*>> futures;
		futures.reserve(nb);
		for (int i = 0; i < nb; ++i) 
		{
			promises[i] = new std::promise<RigidBody*>();
			futures.push_back(promises[i]->get_future());
			auto command = new Ge::MethodCommandReturn<PhysicsEngine, RigidBody*, CollisionShape*, bool>(m_pe, &PhysicsEngine::AllocateRigidbody, promises[i], shape, hasInertia);
			m_queue->push((Command*)command);
		}

		std::vector<RigidWraper*> results;
		results.reserve(nb);
		for (auto& future : futures) {
			RigidBody* rb = future.get();
			results.push_back(new RigidWraper(rb, m_queue));
		}

		for (auto promise : promises) {
			delete promise;
		}

		return results;
	}

	CollisionWraper* PhysicsWraper::AllocateCollision(CollisionShape* shape)
	{
		std::promise<CollisionBody*>* promise = new std::promise<CollisionBody*>();
		auto future = promise->get_future();
		MethodCommandReturn<PhysicsEngine, CollisionBody*, CollisionShape*>* command = new MethodCommandReturn<PhysicsEngine, CollisionBody*, CollisionShape*>(m_pe, &PhysicsEngine::AllocateCollision, promise, shape);
		m_queue->push((Command*)command);
		CollisionBody* cb = future.get();
		delete promise;
		return new CollisionWraper(cb,m_queue);
	}

	std::vector<CollisionWraper*> PhysicsWraper::AllocateCollision(CollisionShape* shape, int nb)
	{
		std::vector<std::promise<CollisionBody*>*> promises(nb);
		std::vector<std::future<CollisionBody*>> futures;
		futures.reserve(nb);
		for (int i = 0; i < nb; ++i) 
		{
			promises[i] = new std::promise<CollisionBody*>();
			futures.push_back(promises[i]->get_future());
			auto command = new Ge::MethodCommandReturn<PhysicsEngine, CollisionBody*, CollisionShape*>(m_pe, &PhysicsEngine::AllocateCollision, promises[i], shape);
			m_queue->push((Command*)command);
		}

		std::vector<CollisionWraper*> results;
		results.reserve(nb);
		for (auto& future : futures) {
			CollisionBody* cb = future.get();
			results.push_back(new CollisionWraper(cb,m_queue));
		}

		for (auto promise : promises) {
			delete promise;
		}

		return results;
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

	void PhysicsWraper::AddRigidbody(RigidWraper* pbody, int group, int mask)
	{
		RigidBody * body = pbody->getRigidBody();
		MethodCommand<PhysicsEngine, RigidBody*, int, int>* command = new MethodCommand<PhysicsEngine, RigidBody*, int, int>(m_pe, &PhysicsEngine::AddRigidbody, body, group, mask);
		m_queue->push((Command*)command);
	}

	void PhysicsWraper::ReleaseRigidbody(RigidWraper* pBody)
	{
		RigidBody * body = pBody->getRigidBody();
		MethodCommand<PhysicsEngine, RigidBody*>* command = new MethodCommand<PhysicsEngine, RigidBody*>(m_pe, &PhysicsEngine::ReleaseRigidbody, body);
		m_queue->push((Command*)command);
		delete pBody;
	}

	void PhysicsWraper::AddCollision(CollisionWraper* pbody)
	{
		CollisionBody* body = pbody->getCollisionBody();
		MethodCommand<PhysicsEngine, CollisionBody*>* command = new MethodCommand<PhysicsEngine, CollisionBody*>(m_pe, &PhysicsEngine::AddCollision, body);
		m_queue->push((Command*)command);
	}

	void PhysicsWraper::ReleaseCollision(CollisionWraper* pBody)
	{
		CollisionBody* body = pBody->getCollisionBody();
		MethodCommand<PhysicsEngine, CollisionBody*>* command = new MethodCommand<PhysicsEngine, CollisionBody*>(m_pe, &PhysicsEngine::ReleaseCollision, body);
		m_queue->push((Command*)command);
		delete pBody;
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

	void PhysicsWraper::DebugDrawCollider()
	{
		if (!m_debugDraw)
		{
			const ptrClass* pc = Engine::getPtrClassAddr();
			const std::vector<RigidBody*> m_rigidBody = m_pe->getRigidbody();
			const std::vector<CollisionBody*> m_collisionBody = m_pe->getCollisionbody();
			m_cubeShader = pc->graphiquePipelineManager->createPipeline("../Asset/Shader/iso.fs.glsl", "../Asset/Shader/iso.vs.glsl");
			m_shapeBuffer = pc->modelManager->allocateBuffer("../Asset/Model/cube.obj");
			m_material = pc->materialManager->createMaterial();
			pc->behaviourManager->addBehaviour(this);
			m_material->setPipeline(m_cubeShader);
			m_material->setRoughness(0.05f);
			m_material->setCastShadow(false);
			for (int i = 0; i < m_rigidBody.size(); i++)
			{
				const CollisionShape*  cs = m_rigidBody[i]->getCollisionShape();
				if (const BoxShape* bs = dynamic_cast<const BoxShape*>(cs))
				{
					glm::vec3 scale = bs->getHalfExtents() *2.0f;
					Model * m = pc->modelManager->createModel(m_shapeBuffer);
					m->setMaterial(m_material);
					m->setScale(scale);
					m_models.push_back(m);					
				}
				else if (const CapsuleShape* cas = dynamic_cast<const CapsuleShape*>(cs))
				{
					glm::vec3 scale = glm::vec3(0.5f) * 2.0f;
					Model* m = pc->modelManager->createModel(m_shapeBuffer);
					m->setMaterial(m_material);
					m->setScale(scale);
					m_models.push_back(m);
				}
			}		
			for (int i = 0; i < m_collisionBody.size(); i++)
			{
				const CollisionShape* cs = m_collisionBody[i]->getCollisionShape();
				if (const BoxShape* bs = dynamic_cast<const BoxShape*>(cs))
				{
					glm::vec3 scale = bs->getHalfExtents() * 2.0f;
					Model* m = pc->modelManager->createModel(m_shapeBuffer);
					m->setMaterial(m_material);
					m->setScale(scale);
					m_models.push_back(m);
				}
				else if (const CapsuleShape* cas = dynamic_cast<const CapsuleShape*>(cs))
				{
					glm::vec3 scale = glm::vec3(0.5f) * 2.0f;
					Model* m = pc->modelManager->createModel(m_shapeBuffer);
					m->setMaterial(m_material);
					m->setScale(scale);
					m_models.push_back(m);
				}
			}
			m_debugDraw = true;
		}
	}

	void PhysicsWraper::DebugClearCollider()
	{
		if (m_debugDraw)
		{
			const ptrClass* pc = Engine::getPtrClassAddr();
			for (int i = 0; i < m_models.size(); i++)
			{
				pc->modelManager->destroyModel(m_models[i]);
			}
			m_models.clear();
			pc->materialManager->destroyMaterial(m_material);
			pc->modelManager->destroyBuffer(m_shapeBuffer);
			pc->graphiquePipelineManager->destroyPipeline(m_cubeShader);			
			pc->behaviourManager->removeBehaviour(this);
			m_debugDraw = false;
		}
	}

	void PhysicsWraper::start()
	{

	}

	void PhysicsWraper::fixedUpdate()
	{

	}

	void PhysicsWraper::update()
	{
		const std::vector<RigidBody*> m_rigidBody = m_pe->getRigidbody();
		for (int i = 0; i < m_rigidBody.size(); i++)
		{
			m_models[i]->setPosition(m_rigidBody[i]->getPosition());
			m_models[i]->setRotation(m_rigidBody[i]->getRotation());
		}
		const std::vector<CollisionBody*> m_collisionBody = m_pe->getCollisionbody();
		for (int i = 0; i < m_collisionBody.size(); i++)
		{
			m_models[i+ m_rigidBody.size()]->setPosition(m_collisionBody[i]->getPosition());
			m_models[i + m_rigidBody.size()]->setRotation(m_collisionBody[i]->getRotation());
		}
	}

	void PhysicsWraper::stop()
	{

	}

	void PhysicsWraper::onGUI()
	{

	}
}