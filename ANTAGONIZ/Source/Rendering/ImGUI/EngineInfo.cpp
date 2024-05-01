#include "glcore.hpp"
#include "EngineInfo.hpp"
#include "GraphicsDataMisc.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"

namespace Ge
{
	void EngineInfo::init(GraphicsDataMisc * gdm)
	{
		m_renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		m_window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
		m_inputManager = Engine::getPtrClass().inputManager;		
	}

	void EngineInfo::render(GraphicsDataMisc * gdm)
	{
		if (m_inputManager->getKeyDown(GLFW_KEY_F2))
		{
			m_open = !m_open;
		}
		if (!m_open)
		{
			return;
		}
		ImGuiIO& io = ImGui::GetIO();		
		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);		
		ImVec2 mainWindowPos = ImGui::GetMainViewport()->Pos;
		ImGui::SetNextWindowPos(ImVec2(mainWindowPos.x + gdm->str_width - 350.0f, mainWindowPos.y), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.55f);
		if (ImGui::Begin("Engine Info", &m_open, m_window_flags))
		{
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Image par seconde : %.1f (FPS)", io.Framerate);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Temps entre chaque image  : %.3f ms/frame", 1000.0f / io.Framerate);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Nombre d'images : %d images", ImGui::GetFrameCount());
			ImGui::Text("Temps depuis demarage %.1f", ImGui::GetTime());
			ImGui::Text("Nombre de modeles : %d", gdm->str_dataMisc.modelCount);
			ImGui::Text("Nombre de textures : %d",  gdm->str_dataMisc.textureCount);
			ImGui::Text("Nombre de materiaux : %d",  gdm->str_dataMisc.materialCount);
			ImGui::Text("Nombre de lumieres : %d",  gdm->str_dataMisc.lightCount);
			ImGui::Text("Carte Graphique (GPU): %s", m_renderer);
			ImGui::Text("Resolution : %.0f x %.0f", io.DisplaySize.x, io.DisplaySize.y);
		}
		ImGui::End();
		ImGui::SetWindowFocus("Engine Info");
	}
}