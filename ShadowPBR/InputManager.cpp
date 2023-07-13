#include "InputManager.hpp"

	InputManager::InputManager(GLFWwindow* window)
	{
		m_window = window;
	}
	void InputManager::updateAxis()
	{
		glfwGetCursorPos(m_window, &m_Mpos.x, &m_Mpos.y);
		m_axisMouse = m_Mpos - m_lastMousePos;	
		m_lastMousePos = m_Mpos;
	}

	const char * InputManager::getGamepadName(int jid)
	{
		return glfwGetGamepadName(jid);
	}

	bool InputManager::getGamepadState(int jid, int key)
	{
		GLFWgamepadstate state;
		glfwGetGamepadState(jid, &state);
		if (key > 15)
		{
			return false;
		}
		return state.buttons[key] == GLFW_PRESS;
	}

	float InputManager::getGamepadAxis(int jid, int indice)
	{
		GLFWgamepadstate state;
		glfwGetGamepadState(jid, &state);
		if (indice > 5)
		{
			return -2;
		}
		return state.axes[indice];
	}

	bool InputManager::getKey(int key)
	{
		return glfwGetKey(m_window, key) == GLFW_PRESS;
	}

	bool InputManager::getKeyUp(int key)
	{
		if (mapReleasedInput.find(key) == mapReleasedInput.end())
		{
			mapReleasedInput[key] = false;
		}
		if (glfwGetKey(m_window, key) == GLFW_PRESS)
		{
			mapReleasedInput[key] = true;
			return false;
		}
		else
		{
			if (mapReleasedInput[key])
			{
				mapReleasedInput[key] = false;
				return true;
			}

		}
		return false;
	}

	bool InputManager::getKeyDown(int key)
	{
		if (mapPressedInput.find(key) == mapPressedInput.end())
		{
			mapPressedInput[key] = false;
		}
		if (glfwGetKey(m_window, key) == GLFW_PRESS)
		{
			if (!mapPressedInput[key])
			{
				mapPressedInput[key] = true;
				return true;
			}
		}
		else
		{
			mapPressedInput[key] = false;
		}
		return false;
	}

	bool InputManager::getMouse(int key)
	{
		return glfwGetMouseButton(m_window, key) == GLFW_PRESS;
	}

	double InputManager::axisMouseX()
	{
		return m_axisMouse.x;
	}

	double InputManager::axisMouseY()
	{
		return m_axisMouse.y;
	}

	double InputManager::getMousePosX()
	{
		return m_Mpos.x;
	}

	double InputManager::getMousePosY()
	{
		return m_Mpos.y;
	}
