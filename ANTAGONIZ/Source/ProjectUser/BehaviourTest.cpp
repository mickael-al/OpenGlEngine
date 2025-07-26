#include "BehaviourTest.hpp"
#include "PhysicsWraper.hpp"
#include "CollisionShape.hpp"
#include "Model.hpp"
#include "Time.hpp"

void BehaviourTest::start()
{
	m_pc = Engine::getPtrClassAddr();
	m_cb2 = new BoxShape(glm::vec3(0.5f, 0.5f, 0.5f), 1.0f);
	m_rw = m_pc->physicsEngine->AllocateRigidbody(m_cb2, countObject);
	for (int i = 0; i < countObject; i++)
	{		
		m_pc->physicsEngine->AddRigidbody(m_rw[i]);
		m_rw[i]->setPosition(glm::vec3(0, 10+i, 0));
	}
	m_shape = m_pc->modelManager->allocateBuffer("../Asset/Model/cube.obj");
	for (int i = 0; i < countObject; i++)
	{
		m_model.push_back(m_pc->modelManager->createModel(m_shape));
	}	
}

void BehaviourTest::fixedUpdate() 
{
	
}

void BehaviourTest::update()
{
	for (int i = 0; i < countObject; i++)
	{
		m_model[i]->setPosition(m_rw[i]->getPosition());
		m_model[i]->setRotation(m_rw[i]->getRotation());		
	}
}

void BehaviourTest::stop() 
{	
	for (int i = 0; i < countObject; i++)
	{		
		m_pc->modelManager->destroyModel(m_model[i]);
		m_pc->physicsEngine->ReleaseRigidbody(m_rw[i]);
	}
	m_model.clear();
	m_rw.clear();
	m_pc->modelManager->destroyBuffer(m_shape);		
	delete m_cb2;
}

void BehaviourTest::onGUI()
{

}