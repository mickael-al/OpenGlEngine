#include "glcore.hpp"
#include "Hud.hpp"
#include "imgui-cmake/Header/imgui.h"
#include "imgui-cmake/Header/imgui_impl_glfw.h"
#include "imgui-cmake/Header/imgui_impl_opengl3.h"
#include "GraphicsDataMisc.hpp"
#include "Debug.hpp"
#include "EngineInfo.hpp"
#include "Console.hpp"
#include "ImguiBlock.hpp"
#include <algorithm>
#include "ImGuizmo.h"
#include "EngineHeader.hpp"
#include "PathManager.hpp"

namespace Ge
{
	Hud::Hud()
	{
		m_engineInfo = new EngineInfo();
		m_console = new Console();
	}

	Hud::~Hud()
	{
		delete m_engineInfo;
		delete m_console;
	}

	bool Hud::initialize(GraphicsDataMisc * gdm)
	{
		m_gdm = gdm;
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		m_io = &io;
		pathImgui = "";
		pathImgui += PathManager::getHomeDirectory();
		pathImgui += "/imgui.ini";
		io.IniFilename = pathImgui.c_str();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;	
		//io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
#if defined(_WIN32) || defined(_WIN64)
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;		
#endif // defined(_WIN32) || defined(_WIN64)		
		
		if (!ImGui_ImplGlfw_InitForOpenGL(m_gdm->str_window, true))
		{
			Debug::Error("ImGui_ImplGlfw_InitForOpenGL");
			return false;
		}
		if (!ImGui_ImplOpenGL3_Init("#version 450"))
		{
			Debug::Error("ImGui_ImplOpenGL3_Init");
			return false;
		}

		m_imguiBlock.push_back(m_engineInfo);
		m_imguiBlock.push_back(m_console);
		m_engineInfo->init(m_gdm);
		m_console->init(m_gdm);

		Debug::INITSUCCESS("ImGUI");
		ImGui::StyleColorsDark();
		Engine::getPtrClass().settingManager->setFramerate(&io.Framerate);
		return true;
	}

	void Hud::addImgui(ImguiBlock * ib)
	{
		m_imguiBlock.push_back(ib);
		ib->init(m_gdm);
	}

	void Hud::removeImgui(ImguiBlock * ib)
	{
		m_imguiBlock.erase(std::remove(m_imguiBlock.begin(), m_imguiBlock.end(), ib), m_imguiBlock.end());
	}

	void Hud::release()
	{		
		m_imguiBlock.clear();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		Debug::RELEASESUCCESS("ImGUI");
	}

	bool Hud::IsMouseOverAnyWindow()
	{
		return m_io->WantCaptureMouse;
	}

	void Hud::pass()
	{
		m_passRender = true;
	}

	void Hud::render()
	{
		if (m_passRender)
		{
			m_passRender = false;
			return;
		}
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();		
		ImGui::NewFrame();		
		ImGuizmo::BeginFrame();

		for (int i = 0; i < m_imguiBlock.size(); i++)
		{
			m_imguiBlock[i]->render(m_gdm);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		if (m_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) 
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
}