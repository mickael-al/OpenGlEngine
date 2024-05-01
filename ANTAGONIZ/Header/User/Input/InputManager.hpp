#ifndef __ENGINE_INPUT_MANAGER__
#define __ENGINE_INPUT_MANAGER__

#include <map>
#include "Initializer.hpp"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
struct GraphicsDataMisc;
class GLFWwindow;

namespace Ge
{
	class InputManager final : InitializerAPI
	{
	public:
		bool initialize(GraphicsDataMisc * gdm);
		void release();
	private:
		friend class Engine;
		void updateAxis();		
	public:
		const char * getGamepadName(int jid) const;
		bool getGamepadState(int jid,int key) const;
		float getGamepadAxis(int jid, int indice) const;
		bool getKey(int key) const;
		bool getKeyUp(int key);
		bool getKeyDown(int key);
		bool getMouse(int key, int pressType = GLFW_PRESS) const;
		void HideMouse(bool state);
		double axisMouseX();
		double axisMouseY();
		double getMousePosX() const;
		double getMousePosY() const;
	private:
		std::map<int, bool> mapPressedInput;
		std::map<int, bool> mapReleasedInput;
		glm::tvec2<double> m_lastMousePos;
		glm::tvec2<double> m_axisMouse;
		glm::tvec2<double> m_Mpos;
		GLFWwindow * m_window;
	};
}

#endif //!__ENGINE_INPUT_MANAGER__