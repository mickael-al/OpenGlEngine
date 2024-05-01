#ifndef __ENGINE_RENDERING_ENGINE__
#define __ENGINE_RENDERING_ENGINE__

#include "Initializer.hpp"

struct GraphicsDataMisc;
struct ptrClass;

namespace Ge
{
	class Window;
	class TextureManager;
	class MaterialManager;
	class ModelManager;
	class CameraManager;
	class GraphiquePipelineManager;
	class Hud;
	class LightManager;
	class ShaderDataMisc;
	class FrameBuffer;
}

namespace Ge
{
    class RenderingEngine final : InitializerPtr
    {
    public:
        RenderingEngine(GraphicsDataMisc * graphicsDataMisc);
		~RenderingEngine();
        bool initialize(ptrClass * p_ptrClass);
        void release();
        void drawFrame();
    private:
		ptrClass * m_ptrClass;
		GraphicsDataMisc * m_graphicsDataMisc;
		FrameBuffer* m_frameBuffer;
		Window * m_window;
		TextureManager * m_textureManager;
		MaterialManager * m_materialManager;
		ModelManager * m_modelManager;
		CameraManager * m_cameraManager;
		GraphiquePipelineManager * m_graphiquePipelineManager;
		Hud * m_hud;
		LightManager * m_lightManager;
		ShaderDataMisc * m_shaderDataMisc;
    };
}

#endif //!__ENGINE_RENDERING_ENGINE__