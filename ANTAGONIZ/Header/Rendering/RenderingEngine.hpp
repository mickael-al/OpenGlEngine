#ifndef __ENGINE_RENDERING_ENGINE__
#define __ENGINE_RENDERING_ENGINE__

#include "Initializer.hpp"
#include "glm/glm.hpp"
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
	class SSAOBuffer;
	class PostProcess;
	class Skybox;
}

namespace Ge
{
    class RenderingEngine final : InitializerPtr
    {
    public:
        RenderingEngine(GraphicsDataMisc * graphicsDataMisc);
		~RenderingEngine();
		glm::vec3 getWorldCoordinates(int mouseX, int mouseY);
        bool initialize(ptrClass * p_ptrClass);
        void release();
        void drawFrame();
    private:
		ptrClass * m_ptrClass;
		GraphicsDataMisc * m_graphicsDataMisc;
		FrameBuffer* m_frameBuffer;
		SSAOBuffer* m_ssaoBuffer;
		Window * m_window;
		TextureManager * m_textureManager;
		MaterialManager * m_materialManager;
		ModelManager * m_modelManager;
		CameraManager * m_cameraManager;
		GraphiquePipelineManager * m_graphiquePipelineManager;
		Hud * m_hud;
		LightManager * m_lightManager;
		ShaderDataMisc * m_shaderDataMisc;
		PostProcess* m_postProcess;
		Skybox* m_skybox;
    };
}

#endif //!__ENGINE_RENDERING_ENGINE__