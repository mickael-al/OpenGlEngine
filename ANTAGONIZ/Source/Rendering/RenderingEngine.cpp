#include "glcore.hpp"
#include "RenderingEngine.hpp"
#include "Debug.hpp"
#include "PointeurClass.hpp"
#include "GraphicsDataMisc.hpp"
#include "Window.hpp"
#include "TextureManager.hpp"
#include "MaterialManager.hpp"
#include "ModelManager.hpp"
#include "CameraManager.hpp"
#include "GraphiquePipelineManager.hpp"
#include "Hud.hpp"
#include "Model.hpp"
#include "Materials.hpp"
#include "GraphiquePipeline.hpp"
#include "ShapeBufferBase.hpp"
#include "LightManager.hpp"
#include "ShaderDataMisc.hpp"
#include "ShaderPair.hpp"
#include "CustomRenderer.hpp"
#include "FrameBuffer.hpp"
#include "Textures.hpp"
#include "Lights.hpp"
#include "SSAOBuffer.hpp"
#include "Camera.hpp"
#include "PostProcess.hpp"
#include <stack>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Skybox.hpp"

namespace Ge
{
    RenderingEngine::RenderingEngine(GraphicsDataMisc * graphicsDataMisc)
    {
		m_graphicsDataMisc = graphicsDataMisc;
		m_frameBuffer = new FrameBuffer();
		m_ssaoBuffer = new SSAOBuffer();
		m_postProcess = new PostProcess();
		m_window = new Window(m_frameBuffer, m_postProcess);
		m_textureManager = new TextureManager();
		m_materialManager = new MaterialManager();
		m_modelManager = new ModelManager();
		m_cameraManager = new CameraManager();
		m_graphiquePipelineManager = new GraphiquePipelineManager();
		m_hud = new Hud();
		m_lightManager = new LightManager();
		m_shaderDataMisc = new ShaderDataMisc();		
		m_skybox = new Skybox();
    }

	RenderingEngine::~RenderingEngine()
	{
		delete m_skybox;
		delete m_postProcess;
		delete m_frameBuffer;
		delete m_ssaoBuffer;
		delete m_shaderDataMisc;
		delete m_lightManager;
		delete m_cameraManager;
		delete m_modelManager;
		delete m_materialManager;
		delete m_textureManager;
		delete m_window;			
		m_graphicsDataMisc = nullptr;		
	}

	glm::vec3 RenderingEngine::getWorldCoordinates(int mouseX, int mouseY)
	{
		int screenWidth = m_ptrClass->settingManager->getWindowWidth();
		int screenHeight = m_ptrClass->settingManager->getWindowHeight();
		float depth;
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer->getFrameBuffer());
		glReadPixels(mouseX, screenHeight - mouseY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
		float ndcX = (2.0f * mouseX) / screenWidth - 1.0f;
		float ndcY = 1.0f - (2.0f * mouseY) / screenHeight;
		float ndcZ = 2.0f * depth - 1.0f;

		glm::vec4 clipSpacePos(ndcX, ndcY, ndcZ, 1.0f);

		Camera* cam = m_cameraManager->getCurrentCamera();
		glm::mat4 invProjectionMatrix = glm::inverse(cam->getProjectionMatrix());
		glm::vec4 viewSpacePos = invProjectionMatrix * clipSpacePos;
		viewSpacePos /= viewSpacePos.w;

		glm::mat4 invViewMatrix = glm::inverse(cam->getViewMatrix());
		glm::vec4 worldSpacePos = invViewMatrix * viewSpacePos;

		return glm::vec3(worldSpacePos);
	}


    bool RenderingEngine::initialize(ptrClass *p_ptrClass)
    {
        if (p_ptrClass == nullptr)
        {
            Debug::Error("ptrClass nullptr RenderingEngine");
            return false;
        }
		m_ptrClass = p_ptrClass;
		m_ptrClass->hud = m_hud;
		m_ptrClass->textureManager = m_textureManager;
		m_ptrClass->modelManager = m_modelManager;
		m_ptrClass->cameraManager = m_cameraManager;
		m_ptrClass->materialManager = m_materialManager;
		m_ptrClass->graphiquePipelineManager = m_graphiquePipelineManager;
		m_ptrClass->lightManager = m_lightManager;
		m_ptrClass->postProcess = m_postProcess;
		m_ptrClass->skybox = m_skybox;
		m_ptrClass->renderingEngine = this;

        Debug::Info("Initialisation du moteur de rendu");

		if (!m_window->initialize(p_ptrClass->settingManager->getWindowWidth(), p_ptrClass->settingManager->getWindowHeight(), p_ptrClass->settingManager->getName(), p_ptrClass->settingManager->getIconPath(), p_ptrClass->settingManager->getVsync(), m_graphicsDataMisc))
		{
			Debug::INITFAILED("Window");
			return false;
		}

		glewExperimental = GL_TRUE;
		GLenum error = glewInit();
		if (error != GLEW_OK)
		{
			Debug::Error("glewInit error code : %d", error);
			return false;
		}
		Debug::INITSUCCESS("OpenGL");     

		if (!m_frameBuffer->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("FrameBuffer");
			return false;
		}

		if (!m_textureManager->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("TextureManager");
			return false;
		}

		if (!m_graphiquePipelineManager->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("GraphiquePipelineManager");
			return false;
		}

		if (!m_materialManager->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("MaterialManager");
			return false;
		}

		if (!m_modelManager->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("ModelManager");
			return false;
		}

		if(!m_cameraManager->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("CameraManager");
			return false;
		}	

		if(!m_hud->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("ImGUI");
			return false;
		}

		if (!m_lightManager->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("LightManager");
			return false;
		}

		if (!m_shaderDataMisc->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("LightManager");
			return false;
		}
		if (!m_ssaoBuffer->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("SSAOManager");
			return false;
		}
		if (!m_postProcess->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("PostProcess");
			return false;
		}
		if (!m_skybox->initialize(m_graphicsDataMisc))
		{
			Debug::INITFAILED("Skybox");
			return false;
		}
        Debug::INITSUCCESS("RenderingEngine");

        return true;
    }

    void RenderingEngine::release()
    {        	
		RenderingEngine::m_skybox->release();
		RenderingEngine::m_postProcess->release();
		RenderingEngine::m_shaderDataMisc->release();
		RenderingEngine::m_ssaoBuffer->release();
		RenderingEngine::m_lightManager->release();
		RenderingEngine::m_hud->release();
		RenderingEngine::m_cameraManager->release();
		RenderingEngine::m_modelManager->release();
		RenderingEngine::m_materialManager->release();
		RenderingEngine::m_graphiquePipelineManager->release();
		RenderingEngine::m_textureManager->release();
		RenderingEngine::m_frameBuffer->release();
		RenderingEngine::m_window->release();
		m_ptrClass = nullptr;
        Debug::RELEASESUCCESS("RenderingEngine");
    }

    void RenderingEngine::drawFrame()
    {
		m_shaderDataMisc->update(m_cameraManager->getCurrentCamera());
		m_cameraManager->updateFlyCam();
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer->getFrameBuffer());
		glViewport(0, 0, m_graphicsDataMisc->str_width, m_graphicsDataMisc->str_height);
		const glm::vec4 & clear = m_ptrClass->settingManager->getClearColor();
		glClearColor(clear.x, clear.y, clear.z, clear.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_skybox->render(m_cameraManager->getSSBO());
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		std::unordered_map<Materials*, std::unordered_map<ShapeBuffer*, std::vector<Model*>>> & instanced_models = m_modelManager->getInstancedModels();		
		const std::vector<Materials *> & matlist = m_materialManager->getMaterials();
		Materials * currentMaterial = nullptr;
		ShapeBuffer * currentShape = nullptr;
		GraphiquePipeline * currentPipeline = nullptr;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_cameraManager->getSSBO());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_modelManager->getSSBO());		
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_materialManager->getSSBO());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_shaderDataMisc->getSSBO());
		bool transparency = false;
		bool currentDepthTest = true;
		bool currentDepthWrite = true;
		for (auto& material : matlist)
		{
			auto& mku = instanced_models[material];
			currentMaterial = material;
			if (currentMaterial == nullptr)
			{
				continue;
			}
			if (!currentMaterial->getDraw())
			{
				continue;
			}
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, currentMaterial->getAlbedoTexture()->getTextureID());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, currentMaterial->getNormalTexture()->getTextureID());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, currentMaterial->getMetallicTexture()->getTextureID());
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, currentMaterial->getRoughnessTexture()->getTextureID());
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, currentMaterial->getOclusionTexture()->getTextureID());
			if (currentMaterial->getDepthTest() != currentDepthTest)
			{
				currentDepthTest = currentMaterial->getDepthTest();
				if (currentDepthTest)
				{
					glEnable(GL_DEPTH_TEST);
				}
				else
				{
					glDisable(GL_DEPTH_TEST);
				}
			}

			if (currentMaterial->getDepthWrite() != currentDepthWrite)
			{
				currentDepthWrite = currentMaterial->getDepthWrite();
				if (currentDepthWrite)
				{
					glDepthMask(GL_TRUE);
				}
				else
				{
					glDepthMask(GL_FALSE);
				}
			}
			currentPipeline = currentMaterial->getPipeline();
			unsigned int program = currentPipeline->getProgram();
			glUseProgram(program);	
			ShaderPair * sp = currentPipeline->getShaderPair();
			if (currentPipeline->getShaderPair()->transparency && !transparency)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				transparency = true;
			}
			if (sp->cullMode == 2)
			{
				glDisable(GL_CULL_FACE);
			}

			std::vector<unsigned int> & assbo = currentMaterial->getAditionalSSBO();
			for (int j = 0; j < assbo.size(); j++)
			{
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, j+4, assbo[j]);				
			}
			for (auto& buffer : mku)
			{
				if (buffer.second.size() > 0)
				{
					currentShape = buffer.first;
					glBindVertexArray(currentShape->getVAO());
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentShape->getIBO());
					glUniform1i(glGetUniformLocation(program, "offsetUbo"), buffer.second[0]->getIndex());
					glDrawElementsInstanced(GL_TRIANGLES, currentShape->getIndiceSize(), GL_UNSIGNED_INT,nullptr, buffer.second.size()+currentMaterial->getAditionalInstanced());
				}
			}
			if (sp->cullMode == 2)
			{
				glEnable(GL_CULL_FACE);
			}
		}		
		if (!currentDepthWrite)
		{
			currentDepthWrite = true;
			glDepthMask(GL_TRUE);
		}
		/*  Shadow  */
		
		//glEnable(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
	//	glCullFace(GL_FRONT);
		glDisable(GL_BLEND);		
		const std::vector<unsigned int>& frameBufferDepthShadow = m_lightManager->getFrameShadowBuffer();
		for (int i = 0; i < frameBufferDepthShadow.size(); i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, frameBufferDepthShadow[i]);
			glClear(GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, TEXTURE_DIM, TEXTURE_DIM);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightManager->getSsboShadow());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_modelManager->getSSBO());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_materialManager->getSSBO());
			currentPipeline = m_graphicsDataMisc->str_default_pipeline_shadow;
			unsigned int program = currentPipeline->getProgram();
			glUseProgram(program);
			int uboU = glGetUniformLocation(program, "offsetUbo");
			int shadowU = glGetUniformLocation(program, "offsetShadow");

			for (auto& material : matlist)
			{
				auto& mku = instanced_models[material];
				currentMaterial = material;
				if (currentMaterial == nullptr)
				{
					continue;
				}
				if (!currentMaterial->getDraw() || !currentMaterial->getCastShadow())
				{
					continue;
				}
				if (currentMaterial->getDepthTest() != currentDepthTest)
				{
					currentDepthTest = currentMaterial->getDepthTest();
					if (!currentDepthTest)
					{
						continue;
					}
				}
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, currentMaterial->getAlbedoTexture()->getTextureID());
				ShaderPair* sp = currentMaterial->getPipeline()->getShaderPair();
				if (sp->cullMode == 2)
				{
					//glDisable(GL_CULL_FACE);
				}
				for (auto& buffer : mku)
				{
					if (buffer.second.size() > 0)
					{
						currentShape = buffer.first;
						glBindVertexArray(currentShape->getVAO());
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentShape->getIBO());
						glUniform1i(uboU, buffer.second[0]->getIndex());
						glUniform1i(shadowU, i);
						glDrawElementsInstanced(GL_TRIANGLES, currentShape->getIndiceSize(), GL_UNSIGNED_INT, nullptr, buffer.second.size() + currentMaterial->getAditionalInstanced());
					}
				}
				if (sp->cullMode == 2)
				{
					//glEnable(GL_CULL_FACE);
				}
			}
		}
		glCullFace(GL_BACK);
		/*  Shadow  */

		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer->getFowardFrameBuffer());		
		glDisable(GL_DEPTH_TEST);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_cameraManager->getSSBO());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_lightManager->getSSBO());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_shaderDataMisc->getSSBO());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_lightManager->getSsboShadow());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_ssaoBuffer->getSSBO());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_frameBuffer->getPosition());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_frameBuffer->getNormal());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_frameBuffer->getColor());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_frameBuffer->getOther());
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_lightManager->getTextureShadowArray());
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, m_ssaoBuffer->getNoiseTexture());
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox->getIrradianceMap());
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox->getPrefilterMap());
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, m_skybox->getBrdfLUTTexture());
		glUseProgram(m_graphicsDataMisc->str_default_pipeline_forward->getProgram());
		glViewport(0, 0, m_graphicsDataMisc->str_width, m_graphicsDataMisc->str_height);

		ShapeBuffer* sb = m_modelManager->getFullScreenTriangle();
		glBindVertexArray(sb->getVAO());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sb->getIBO());
		glDrawElements(GL_TRIANGLES, sb->getIndiceSize(), GL_UNSIGNED_INT, 0);

		m_postProcess->compute(m_frameBuffer->getFowardFrameBuffer(), m_frameBuffer->getColorFoward(), m_modelManager->getDefferedQuad());

		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameBuffer->getFowardFrameBuffer());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(
			0, 0, m_graphicsDataMisc->str_width, m_graphicsDataMisc->str_height,
			0, 0, m_graphicsDataMisc->str_width, m_graphicsDataMisc->str_height,
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST
		);
		
		m_hud->render();		
		glfwSwapBuffers(m_graphicsDataMisc->str_window);
    }
}