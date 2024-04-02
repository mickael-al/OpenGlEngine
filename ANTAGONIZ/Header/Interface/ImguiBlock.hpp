#ifndef __ENGINE_INTERFACE_IMGUI_BLOCK__
#define __ENGINE_INTERFACE_IMGUI_BLOCK__

#define GLFW_INCLUDE_NONE


namespace Ge
{
	class ImguiBlock
	{
	public:
		virtual void preRender() = 0;
		virtual void render() = 0;
	};
}

#endif //__ENGINE_INTERFACE_IMGUI_BLOCK__