#ifndef __ENGINE_WINDOW__
#define __ENGINE_WINDOW__

struct GraphicsDataMisc;
class GLFWwindow;

namespace Ge
{
	class FrameBuffer;
	class PostProcess;
	class Window final
	{
	public:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		GLFWwindow* getWindow();
	private:
		friend class RenderingEngine;
		Window(FrameBuffer * fb, PostProcess * pp);
		bool initialize(unsigned int Width, unsigned int Height, const char * name,const char * iconPath,bool vsync, GraphicsDataMisc * gdm);
		void release();
		bool getframebufferResized();
		void setframebufferResized(bool state);		
		void centerWindow(int width, int height);
		void setWindowPosition(int x, int y);
		void getWindowPosition(int * x, int * y);
	private:		
		friend class Window;
		bool m_framebufferResized = false;
		GLFWwindow * m_window;
		GraphicsDataMisc * m_gdm;
		FrameBuffer* m_fb;
		PostProcess* m_pp;
	};
}
#endif //!__ENGINE_WINDOW__