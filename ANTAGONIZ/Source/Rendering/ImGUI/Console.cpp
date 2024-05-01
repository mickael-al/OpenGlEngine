#include "Console.hpp"
#include "GraphicsDataMisc.hpp"
#include "Debug.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"

namespace Ge
{
	void Console::init(GraphicsDataMisc * gdm)
	{
		m_autoScroll = true;
		Clear();
		Debug::console = this;
		m_inputManager = Engine::getPtrClass().inputManager;
	}

	void Console::render(GraphicsDataMisc * gdm)
	{
		if (m_inputManager->getKeyDown(GLFW_KEY_F1))
		{
			m_open = !m_open;
		}
		if (!m_open)
		{
			return;
		}
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		ImGui::SetNextWindowBgAlpha(0.55f);
		if (!ImGui::Begin("Console", &m_open, window_flags))
		{
			ImGui::End();
			return;
		}
		ImVec2 mainWindowPos = ImGui::GetMainViewport()->Pos;
		ImGui::SetWindowPos("Console", ImVec2(mainWindowPos.x, mainWindowPos.y+ gdm->str_height - gdm->str_height / 3.55f));
		ImGui::SetWindowSize("Console", ImVec2(gdm->str_width, gdm->str_height / 3.55f));
		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &m_autoScroll);
			ImGui::EndPopup();
		}

		// Main window
		if (ImGui::Button("Options"))
		{
			ImGui::OpenPopup("Options");
		}
		ImGui::SameLine();
		bool clear = ImGui::Button("Clear");
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		m_filter.Draw("filter", -100.0f);

		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (clear)
		{
			Clear();
		}
		if (copy)
		{
			ImGui::LogToClipboard();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = m_buf.begin();
		const char* buf_end = m_buf.end();
		if (m_filter.IsActive())
		{
			for (int line_no = 0; line_no < m_lineOffsets.Size; line_no++)
			{
				const char* line_start = buf + m_lineOffsets[line_no];
				const char* line_end = (line_no + 1 < m_lineOffsets.Size) ? (buf + m_lineOffsets[line_no + 1] - 1) : buf_end;
				if (m_filter.PassFilter(line_start, line_end))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, m_colorListe[(line_no + 1) % m_colorListe.size()]);
					ImGui::TextUnformatted(line_start, line_end);
					ImGui::PopStyleColor();
				}
			}
		}
		else
		{
			ImGuiListClipper clipper;
			clipper.Begin(m_lineOffsets.Size);
			while (clipper.Step())
			{
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
				{
					const char* line_start = buf + m_lineOffsets[line_no];
					const char* line_end = (line_no + 1 < m_lineOffsets.Size) ? (buf + m_lineOffsets[line_no + 1] - 1) : buf_end;
					ImGui::PushStyleColor(ImGuiCol_Text, m_colorListe[(line_no + 1) % m_colorListe.size()]);
					ImGui::TextUnformatted(line_start, line_end);
					ImGui::PopStyleColor();
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();
		ImGui::End();
	}

	void Console::Clear()
	{
		m_buf.clear();
		m_colorListe.clear();
		m_lineOffsets.clear();
		m_colorListe.push_back(ImColor(1.0f, 1.0f, 1.0f, 1.0f));
		m_lineOffsets.push_back(0);
	}

	void Console::setBaseColor(ImColor ic)
	{
		m_corlorBase = ic;
	}
}