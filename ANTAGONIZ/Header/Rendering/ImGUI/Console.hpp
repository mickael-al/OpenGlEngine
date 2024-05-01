#ifndef __ENGINE_CONSOLE__
#define __ENGINE_CONSOLE__

#include "ImguiBlock.hpp"
#include <vector>

namespace Ge
{
	class InputManager;
}

namespace Ge
{
	class Console final : public ImguiBlock
	{
	public:		
		void init(GraphicsDataMisc * gdm);
		void render(GraphicsDataMisc * gdm);
		void setBaseColor(ImColor ic);
		void Clear();
		void AddLog(const char* fmt, ...) IM_FMTARGS(2)
		{
			int old_size = m_buf.size();
			va_list args;
			va_start(args, fmt);
			m_buf.appendfv(fmt, args);
			va_end(args);
			for (int new_size = m_buf.size(); old_size < new_size; old_size++)
			{
				if (m_buf[old_size] == '\n')
				{
					m_lineOffsets.push_back(old_size + 1);
					m_colorListe.emplace_back(m_corlorBase);
				}
			}
		}
	private:				
		bool m_open = false;
		ImGuiTextBuffer     m_buf;
		ImGuiTextFilter     m_filter;
		ImVector<int>       m_lineOffsets;
		bool                m_autoScroll;
		std::vector<ImVec4> m_colorListe;
		ImColor m_corlorBase = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
		InputManager * m_inputManager;
	};
}

#endif //!__ENGINE_CONSOLE__