#include "MuscleWraper.hpp"
#include "CommandQueue.hpp"
#include "Muscle.hpp"

namespace Ge
{

	MuscleWraper::MuscleWraper(Muscle* mu, CommandQueue* queue)
	{
		m_mu = mu;
		m_queue = queue;
	}

	void MuscleWraper::setContraction(glm::vec3 m_value, float dt)
	{
		MethodCommand<Muscle, glm::vec3 ,float>* command = new MethodCommand<Muscle, glm::vec3,float>(m_mu, &Muscle::setContraction, m_value,dt);
		m_queue->push((Command*)command);
	}

	void MuscleWraper::setWing(bool state)
	{
		MethodCommand<Muscle, bool>* command = new MethodCommand<Muscle, bool>(m_mu, &Muscle::setWing, state);
		m_queue->push((Command*)command);
	}

	void MuscleWraper::reset()
	{
		MethodCommand<Muscle>* command = new MethodCommand<Muscle>(m_mu, &Muscle::reset);
		m_queue->push((Command*)command);
	}

	glm::vec3 MuscleWraper::getContraction() const
	{
		return m_mu->getContraction();
	}

	Muscle* MuscleWraper::getMuscle() const
	{
		return m_mu;
	}
}