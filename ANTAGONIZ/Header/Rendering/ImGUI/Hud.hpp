#ifndef __ENGINE_HUD__
#define __ENGINE_HUD__

#include "Initializer.hpp"
#include <vector>
#include <string>
#include <map>

namespace Ge
{
	class ImguiBlock;
	class EngineInfo;
	class Console;
}

struct ImGuiIO;
struct ImFont;
namespace Ge
{
	class Hud final : public InitializerAPI
	{
	public:
		Hud();
		~Hud();
		void addImgui(ImguiBlock * ib);
		void removeImgui(ImguiBlock * ib);
		ImFont* addFont(std::string path, float size);
		bool IsMouseOverAnyWindow();
		void pass();
	protected:
		friend class RenderingEngine;				
		bool initialize(GraphicsDataMisc * gdm);
		void release();
		void render();
	public:
		GraphicsDataMisc * m_gdm;
		std::map<size_t, std::pair<unsigned int, ImFont*>> m_fontID;
		std::vector<ImguiBlock *> m_imguiBlock;
		EngineInfo * m_engineInfo;
		Console * m_console;
		ImGuiIO * m_io;
		std::string pathImgui;
		bool m_passRender = false;
	};
}
#endif //!__ENGINE_HUD__