#include "PhysicsEngine.hpp"
#include "RigidBody.hpp"
#include "CollisionBody.hpp"
#include "Muscle.hpp"
#include "StaticMeshCollider.hpp"
#include "CommandQueue.hpp"

namespace Ge
{
	bool PhysicsEngine::Initialize(glm::vec3 const& gravity, CommandQueue* queue)
	{
		auto beginPhysicsInit = std::chrono::steady_clock::now();
		Debug::Info("Initialisation du moteur physique");
		m_queue = queue;
		m_pCollisionCongifuration = new btDefaultCollisionConfiguration();
		m_pCollisionDispatcher = new btCollisionDispatcher(m_pCollisionCongifuration);
		m_pBroadphaseInterface = new btDbvtBroadphase();
		m_pSequentialConstraintSolver = new btSequentialImpulseConstraintSolver();

		m_pDynamicWorld = new btDiscreteDynamicsWorld(
			m_pCollisionDispatcher,
			m_pBroadphaseInterface,
			m_pSequentialConstraintSolver,
			m_pCollisionCongifuration);

		m_pDynamicWorld->setGravity(btVector3(gravity.x, gravity.y, gravity.z));

		auto endPhysicsInit = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endPhysicsInit - beginPhysicsInit);

		return true;
	}

	void PhysicsEngine::Update(float dt,bool engine)
	{
		m_pDynamicWorld->stepSimulation(dt, 1);
		while (!m_queue->empty())
		{
			Command* command = m_queue->pop();
			command->execute();
			delete command;
		}
	}

	void PhysicsEngine::Shutdown()
	{
		for (int i = 0; i < m_muscle.size(); i++)
		{
			delete m_muscle[i];
		}
		m_muscle.clear();
		for (int i = 0; i < m_rigidbody.size(); i++)
		{
			m_pDynamicWorld->removeRigidBody(m_rigidbody[i]->m_pBody);			
			delete m_rigidbody[i];
		}
		m_rigidbody.clear();
		for (int i = 0; i < m_collisionbody.size(); i++)
		{
			m_pDynamicWorld->removeCollisionObject(m_collisionbody[i]->m_pBody);
			delete m_collisionbody[i];
		}
		m_collisionbody.clear();
		delete m_pCollisionCongifuration;
		delete m_pCollisionDispatcher;
		delete m_pBroadphaseInterface;
		delete m_pSequentialConstraintSolver;
		delete m_pDynamicWorld;
	}

	CollisionBody* PhysicsEngine::AllocateCollision(CollisionShape* shape)
	{
		return new CollisionBody(m_pDynamicWorld, shape);
	}

	void PhysicsEngine::AddCollision(CollisionBody* body)
	{
		if (body != nullptr && body->IsInitialized())
		{
			m_pDynamicWorld->addCollisionObject(body->GetBody());			
			m_collisionbody.push_back(body);
		}
		else
		{
			Debug::Error("PhysicsEngine::AddCollision(): Le corps donné n'est pas initialisé.");
		}
	}

	const std::vector<RigidBody*> PhysicsEngine::getRigidbody() const
	{
		return m_rigidbody;
	}

	const std::vector<CollisionBody*> PhysicsEngine::getCollisionbody() const
	{
		return m_collisionbody;
	}

	bool PhysicsEngine::raycast(const glm::vec3* start, const glm::vec3* end, glm::vec3* hitPoint)
	{
		btVector3 btstart = btVector3(start->x, start->y, start->z);
		btVector3 btend = btVector3(end->x, end->y, end->z);
		btCollisionWorld::ClosestRayResultCallback rayCallback(btstart, btend);
		rayCallback.m_collisionFilterMask = 1;
		m_pDynamicWorld->rayTest(btstart, btend, rayCallback);

		if (rayCallback.hasHit()) 
		{
			btVector3 hit;
			hit = rayCallback.m_hitPointWorld;
			*hitPoint = glm::vec3(hit.x(), hit.y(), hit.z());
			return true;
		}
		else 
		{
			return false;
		}
	}

	void PhysicsEngine::ReleaseCollision(CollisionBody* pBody)
	{
		std::vector<CollisionBody*>::iterator position = std::remove(m_collisionbody.begin(), m_collisionbody.end(), pBody);
		if (position != m_collisionbody.end())
		{
			m_collisionbody.erase(position);
		}
		m_pDynamicWorld->removeCollisionObject(pBody->m_pBody);
		delete pBody;
	}

	bool PhysicsEngine::isPointInsideCollision(const glm::vec3 * point,float testRadius)
	{
		// Crée une petite sphère à la position du point
		btSphereShape testShape(testRadius);

		// Transforme le point glm::vec3 en btVector3
		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(btVector3(point->x, point->y, point->z));

		// Crée un objet collision temporaire
		btCollisionObject obj;
		obj.setCollisionShape(&testShape);
		obj.setWorldTransform(transform);

		// Structure pour récupérer le résultat du test
		struct Callback : public btCollisionWorld::ContactResultCallback 
		{
			bool hasHit = false;
			btScalar addSingleResult(btManifoldPoint&, const btCollisionObjectWrapper*, int, int,const btCollisionObjectWrapper*, int, int) override 
			{
				hasHit = true;
				return 0;
			}
		} callback;

		m_pDynamicWorld->contactTest(&obj, callback);

		return callback.hasHit; // true = le point est à l'intérieur (ou touche)
	}

	RigidBody * PhysicsEngine::AllocateRigidbody(CollisionShape* shape, bool hasInertia)
	{
		return new RigidBody(m_pDynamicWorld, shape, hasInertia);
	}

	Muscle* PhysicsEngine::AllocateMuscle(RigidBody* rb1, RigidBody* rb2,float degres,float scale, bool adaptePos)
	{
		Muscle* m = new Muscle(m_pDynamicWorld, rb1->GetBody(), rb2->GetBody(), rb1->getPosition(), rb2->getPosition(), rb1->getScale()* scale,rb2->getScale()* scale,degres, adaptePos);
		m_muscle.push_back(m);
		return m;
	}

	void PhysicsEngine::ReleaseMuscle(Muscle* pBody)
	{
		std::vector<Muscle*>::iterator position = std::remove(m_muscle.begin(), m_muscle.end(), pBody);
		if (position != m_muscle.end())
		{
			m_muscle.erase(position);
		}
		delete pBody;
	}

	StaticMeshCollider* PhysicsEngine::AllocateStaticMeshCollider(btBvhTriangleMeshShape* meshShape)
	{
		StaticMeshCollider* smc = new StaticMeshCollider(m_pDynamicWorld, meshShape);
		m_staticMeshCollider.push_back(smc);
		return smc;
	}

	void PhysicsEngine::ReleaseStaticMeshCollider(StaticMeshCollider* pBody)
	{
		std::vector<StaticMeshCollider*>::iterator position = std::remove(m_staticMeshCollider.begin(), m_staticMeshCollider.end(), pBody);
		if (position != m_staticMeshCollider.end())
		{
			m_staticMeshCollider.erase(position);
		}
		delete pBody;
	}

	void PhysicsEngine::AddRigidbody(RigidBody* body,int group,int mask)
	{
		if (body != nullptr && body->IsInitialized())
		{				
			m_pDynamicWorld->addRigidBody(body->GetBody(),group,mask);
			m_rigidbody.push_back(body);
		}
		else
		{
			Debug::Error("PhysicsEngine::AddRigidbody(): Le corps donné n'est pas initialisé.");
		}
	}

	void PhysicsEngine::ReleaseRigidbody(RigidBody * pBody)
	{		
		std::vector<RigidBody*>::iterator position = std::remove(m_rigidbody.begin(), m_rigidbody.end(), pBody);
		if (position != m_rigidbody.end())
		{
			m_rigidbody.erase(position);
		}
		m_pDynamicWorld->removeRigidBody(pBody->m_pBody);
		delete pBody;
	}
}

