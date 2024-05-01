#ifndef __ENGINE_INTERFACE_IMGUI_BLOCK__
#define __ENGINE_INTERFACE_IMGUI_BLOCK__

#define GLFW_INCLUDE_NONE
#include "imgui-cmake/Header/imgui.h"
#include "imgui-cmake/Header/imgui_impl_glfw.h"

struct GraphicsDataMisc;

namespace Ge
{
	class ImguiBlock
	{
	public:
		virtual void init(GraphicsDataMisc * gdm) = 0;
		virtual void render(GraphicsDataMisc * gdm) = 0;
	};
}

#endif //!__ENGINE_INTERFACE_IMGUI_BLOCK__