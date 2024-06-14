#include "BehaviourTest.hpp"
#include "PhysicsWraper.hpp"
#include "CollisionShape.hpp"

void BehaviourTest::start()
{
	m_pc = Engine::getPtrClassAddr();
	m_cb = new BoxShape(glm::vec3(10.0f,10.0f,10.0f), 1.0f);
	m_cbBody = m_pc->physicsEngine->AllocateCollision(m_cb);
}

void BehaviourTest::fixedUpdate() 
{
	
}

void BehaviourTest::update()
{
	Debug::Log("Update BehaviourTest");
}

void BehaviourTest::stop() 
{
	delete m_cb;
}

void BehaviourTest::onGUI()
{

}