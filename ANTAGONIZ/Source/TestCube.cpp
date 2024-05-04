#include "TestCube.hpp"
#include "DirectionalLight.hpp"

TestCube::TestCube(Model * m)
{
	m_pc = &Engine::getPtrClass();
	m_model = m;
	m_dir = m_pc->lightManager->createDirectionalLight(glm::vec3(-45.0f, 45.0f, 0.0f), glm::vec3(1, 1, 1));
	m_dir->setshadow(true);
}

void TestCube::start()
{

}

void TestCube::fixedUpdate()
{

}

void TestCube::update()
{
	m_dir->setEulerAngles(glm::vec3(-45.0f, (m_pc->time->getTime() * 36.0f),0));
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