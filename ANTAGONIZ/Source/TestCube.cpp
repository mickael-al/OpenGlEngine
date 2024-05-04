#include "TestCube.hpp"

TestCube::TestCube(Model * m)
{
	m_pc = &Engine::getPtrClass();
	m_model = m;
}

void TestCube::start()
{

}

void TestCube::fixedUpdate()
{

}

void TestCube::update()
{
	//m_model->setEulerAngles(glm::vec3((m_pc->time->getTime()*36.0f),0,0));
	if (m_pc->inputManager->getKey(GLFW_KEY_J))
	{
		glm::vec3 pos = m_model->getPosition();
		pos.y += m_pc->time->getDeltaTime();
		m_model->setPosition(pos);
	}
	if (m_pc->inputManager->getKey(GLFW_KEY_H))
	{
		glm::vec3 pos = m_model->getPosition();
		pos.y -= m_pc->time->getDeltaTime();
		m_model->setPosition(pos);
	}
}

void TestCube::stop()
{

}

void TestCube::onGUI()
{

}