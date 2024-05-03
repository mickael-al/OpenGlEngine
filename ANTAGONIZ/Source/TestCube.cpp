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
	m_model->setEulerAngles(glm::vec3((m_pc->time->getTime()*36.0f),0,0));
}

void TestCube::stop()
{

}

void TestCube::onGUI()
{

}