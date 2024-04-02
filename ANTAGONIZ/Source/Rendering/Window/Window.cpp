#include "Window.hpp"
#include "stb-cmake/stb_image.h"
#include "Debug.hpp"

namespace Ge
{

	bool Window::initialize(uint32_t Width, uint32_t Height, const char * name, const char * iconPath, VulkanMisc * vM)
	{
		glfwInit();//init la fenetre

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		this->m_window = glfwCreateWindow(Width, Height, name, nullptr, nullptr);
		glfwSetWindowUserPointer(this->m_window, this);//;on fournie la class a la fenetre
		glfwSetFramebufferSizeCallback(this->m_window, this->framebufferResizeCallback);
		vM->str_VulkanDeviceMisc->str_window = this->m_window;
		Debug::INITSUCCESS("Window");		
		if (iconPath && iconPath[0])
		{									
			GLFWimage images;
			images.pixels = stbi_load(iconPath, &images.width, &images.height, 0, 4);
			glfwSetWindowIcon(this->m_window, 1, &images);
			stbi_image_free(images.pixels);
		}		
		return true;
	}

	void Window::release()
	{
		glfwDestroyWindow(this->m_window);
		glfwTerminate();
		Debug::RELEASESUCCESS("Window");
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{		
		auto win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		win->framebufferResized = true;
		Debug::Info("Redimension de la fenetre : %d , %d", width,height);
	}
}