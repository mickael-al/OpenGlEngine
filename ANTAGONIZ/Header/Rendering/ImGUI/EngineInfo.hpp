#ifndef __ENGINE_INFO__
#define __ENGINE_INFO__

#include "ImguiBlock.hpp"
typedef int ImGuiWindowFlags;

namespace Ge
{
	class InputManager;
}

namespace Ge
{
	class EngineInfo : public ImguiBlock
	{
	public:
		void init(GraphicsDataMisc * gdm);
		void render(GraphicsDataMisc * gdm);
	private:
		bool m_open = false;
		const char* m_renderer;
		ImGuiWindowFlags m_window_flags;
		InputManager * m_inputManager;
	};

}

#endif //!__ENGINE_INFO__
