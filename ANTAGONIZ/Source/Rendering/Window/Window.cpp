#include "Window.hpp"
#include "Debug.hpp"
#include "GLFW/glfw3.h"
#include "GraphicsDataMisc.hpp"
#include "stb-cmake/stb_image.h"
#include "Engine.hpp"
#include "EngineHeader.hpp"
#include "FrameBuffer.hpp"
#include "PostProcess.hpp"

namespace Ge
{
	Window::Window(FrameBuffer* fb, PostProcess * pp)
	{
		m_fb = fb;
		m_pp = pp;
	}

	bool Window::initialize(uint32_t Width, uint32_t Height, const char * name, const char * iconPath,bool vsync, GraphicsDataMisc * gdm)
	{
		if (!glfwInit())
		{
			return false;
		}
		m_gdm = gdm;
		gdm->str_width = Width;
		gdm->str_height = Height;
		this->m_window = glfwCreateWindow(Width, Height, name, NULL, NULL);
		if (!this->m_window)
		{
			glfwTerminate();
			Debug::Error("glfwCreateWindow");
			return false;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);

		glfwSetWindowUserPointer(this->m_window, this);
		glfwSetFramebufferSizeCallback(this->m_window, this->framebufferResizeCallback);
		gdm->str_window = this->m_window;
		glfwMakeContextCurrent(this->m_window);
		if (!vsync)
		{
			glfwSwapInterval(0);//VSYNC
		}
		Debug::INITSUCCESS("Glfw Window");
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
		glfwDestroyWindow(this->m_window);/*DestroyWindow*/
		glfwTerminate();
		Debug::RELEASESUCCESS("Window");
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		win->m_gdm->str_width = width;
		win->m_gdm->str_height = height;
		if (width == 0 && height == 0)
		{
			return;
		}
		win->m_framebufferResized = true;
		ptrClass pc = Engine::getPtrClass();
		pc.cameraManager->updateStorage();
		pc.settingManager->setWindowHeight(height);
		pc.settingManager->setWindowWidth(width);
		win->m_fb->resize(width, height);
		win->m_pp->resize(width, height);
		Debug::Info("Redimension de la fenetre : %d , %d", width, height);
	}

	void Window::setframebufferResized(bool state)
	{
		m_framebufferResized = state;
	}

	bool Window::getframebufferResized()
	{
		return m_framebufferResized;
	}

	GLFWwindow * Window::getWindow()
	{
		return m_window;
	}
}