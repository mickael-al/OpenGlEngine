#include "glcore.hpp"
#include "Skybox.hpp"
#include "stb-cmake/stb_image.h"
#include <cmath>
#include <cstring>
#include "imgui-cmake/Header/imgui.h"
#include "Debug.hpp"
#include "GObject.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"
#include "GraphiquePipeline.hpp"

namespace Ge
{	
	bool Skybox::initialize(GraphicsDataMisc* gdm)
	{
		m_pc = Engine::getPtrClassAddr();
		glGenFramebuffers(1, &m_captureFBO);
		glGenRenderbuffers(1, &m_captureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SKY_SIZE, SKY_SIZE);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO);
	
		glGenTextures(1, &m_envCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, SKY_SIZE, SKY_SIZE, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenTextures(1, &m_irradianceMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, SKY_IR_SIZE, SKY_IR_SIZE, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenTextures(1, &m_prefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, SKY_FILTER_SIZE, SKY_FILTER_SIZE, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glGenTextures(1, &m_brdfLUTTexture);		
		glBindTexture(GL_TEXTURE_2D, m_brdfLUTTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SKY_SIZE, SKY_SIZE, 0, GL_RG, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GraphiquePipeline * gpbrdf = m_pc->graphiquePipelineManager->createPipeline("../Asset/Shader/brdf.fs.glsl", "../Asset/Shader/brdf.vs.glsl");
		unsigned int quadVAO = 0;
		unsigned int quadVBO = 0;
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SKY_SIZE, SKY_SIZE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLUTTexture, 0);

		glViewport(0, 0, SKY_SIZE, SKY_SIZE);
		glUseProgram(gpbrdf->getProgram());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_pc->graphiquePipelineManager->destroyPipeline(gpbrdf);
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);


		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			 // bottom face
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 // top face
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &m_cubeVAO);
		glGenBuffers(1, &m_cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(m_cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		
		m_cubeMapCompute = m_pc->graphiquePipelineManager->createPipeline("../Asset/Shader/cubeMapProjection.fs.glsl", "../Asset/Shader/cubeMapProjection.vs.glsl");
		m_cubeMapRender = m_pc->graphiquePipelineManager->createPipeline("../Asset/Shader/background.fs.glsl", "../Asset/Shader/background.vs.glsl");		
		m_cubeMapConvolutionCompute = m_pc->graphiquePipelineManager->createPipeline("../Asset/Shader/irradiance_convolution.fs.glsl", "../Asset/Shader/cubeMapProjection.vs.glsl");
		m_cubeMapFilter = m_pc->graphiquePipelineManager->createPipeline("../Asset/Shader/prefilter.fs.glsl", "../Asset/Shader/cubeMapProjection.vs.glsl");
		return true;
	}

	void Skybox::render(unsigned int cameraSSBO)
	{
		if (m_hdrTexture == 0)
		{
			return;
		}
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glUseProgram(m_cubeMapRender->getProgram());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cameraSSBO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
		glBindVertexArray(m_cubeVAO);//render Cube
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

	}

	unsigned int Skybox::getCubemap() const
	{
		return m_envCubemap;
	}

	unsigned int Skybox::getIrradianceMap() const
	{
		return m_irradianceMap;
	}

	unsigned int Skybox::getPrefilterMap() const
	{
		return m_prefilterMap;
	}

	unsigned int Skybox::getBrdfLUTTexture() const
	{
		return m_brdfLUTTexture;
	}

	void Skybox::clearTextureSkybox()
	{
		if (m_hdrTexture != 0)
		{
			glDeleteTextures(1, &m_hdrTexture);
			m_hdrTexture = 0;
		}
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
		glUseProgram(m_cubeMapCompute->getProgram());
		int up = glGetUniformLocation(m_cubeMapCompute->getProgram(), "projection");
		int uv = glGetUniformLocation(m_cubeMapCompute->getProgram(), "view");
		glUniformMatrix4fv(up, 1, false, &captureProjection[0][0]);
		glViewport(0, 0, SKY_SIZE, SKY_SIZE); // don't forget to configure the viewport to the capture dimensions.
		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
		glDisable(GL_CULL_FACE);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_envCubemap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		glViewport(0, 0, SKY_IR_SIZE, SKY_IR_SIZE);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(uv, 1, false, &captureViews[i][0][0]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		glViewport(0, 0, SKY_FILTER_SIZE, SKY_FILTER_SIZE);
		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			// reisze framebuffer according to mip-level size.
			unsigned int mipWidth = SKY_FILTER_SIZE * std::pow(0.5, mip);
			unsigned int mipHeight = SKY_FILTER_SIZE * std::pow(0.5, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
			glViewport(0, 0, mipWidth, mipHeight);
			for (unsigned int i = 0; i < 6; ++i)
			{
				glUniformMatrix4fv(uv, 1, false, &captureViews[i][0][0]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilterMap, mip);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Skybox::loadTextureSkybox(const std::string path)
	{
		if (m_hdrTexture != 0)
		{
			glDeleteTextures(1, &m_hdrTexture);
			m_hdrTexture = 0;
		}
		stbi_set_flip_vertically_on_load(true);
		int width, height, nrComponents;
		float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);		
		if (data)
		{
			glGenTextures(1, &m_hdrTexture);
			glBindTexture(GL_TEXTURE_2D, m_hdrTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_FLOAT, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			Debug::Error("Failed to load HDR image.");
			stbi_set_flip_vertically_on_load(false);
			return;
		}
		stbi_set_flip_vertically_on_load(false);
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glUseProgram(m_cubeMapCompute->getProgram());
		int uem = glGetUniformLocation(m_cubeMapCompute->getProgram(), "equirectangularMap");
		int up = glGetUniformLocation(m_cubeMapCompute->getProgram(), "projection");
		int uv = glGetUniformLocation(m_cubeMapCompute->getProgram(), "view");
		glUniform1i(uem, 0);		
		glUniformMatrix4fv(up, 1, false, &captureProjection[0][0]);		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_hdrTexture);

		glViewport(0, 0, SKY_SIZE, SKY_SIZE); // don't forget to configure the viewport to the capture dimensions.
		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
		glDisable(GL_CULL_FACE);
		for (unsigned int i = 0; i < 6; ++i)
		{			
			glUniformMatrix4fv(uv, 1, false, &captureViews[i][0][0]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_envCubemap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(m_cubeVAO);//render Cube
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}
		glUseProgram(m_cubeMapConvolutionCompute->getProgram());
		up = glGetUniformLocation(m_cubeMapConvolutionCompute->getProgram(), "projection");
		uv = glGetUniformLocation(m_cubeMapConvolutionCompute->getProgram(), "view");
		glUniformMatrix4fv(up, 1, false, &captureProjection[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
		glViewport(0, 0, SKY_IR_SIZE, SKY_IR_SIZE);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(uv, 1, false, &captureViews[i][0][0]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(m_cubeVAO);//render Cube
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}
		
		glViewport(0, 0, SKY_FILTER_SIZE, SKY_FILTER_SIZE);
		glUseProgram(m_cubeMapFilter->getProgram());
		uem = glGetUniformLocation(m_cubeMapFilter->getProgram(), "environmentMap");
		up = glGetUniformLocation(m_cubeMapFilter->getProgram(), "projection");
		uv = glGetUniformLocation(m_cubeMapFilter->getProgram(), "view");
		int ur = glGetUniformLocation(m_cubeMapFilter->getProgram(), "roughness");
	//	glUniform1i(uem, 0);
		glUniformMatrix4fv(up, 1, false, &captureProjection[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);

		glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			// reisze framebuffer according to mip-level size.
			unsigned int mipWidth = SKY_FILTER_SIZE * std::pow(0.5, mip);
			unsigned int mipHeight = SKY_FILTER_SIZE * std::pow(0.5, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
			//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);			
			glUniform1f(ur, roughness);
			for (unsigned int i = 0; i < 6; ++i)
			{
				glUniformMatrix4fv(uv, 1, false, &captureViews[i][0][0]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilterMap, mip);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glBindVertexArray(m_cubeVAO);//render Cube
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Skybox::release()
	{
		if (m_hdrTexture != 0)
		{
			glDeleteTextures(1, &m_hdrTexture);
		}
		glDeleteTextures(1, &m_envCubemap);
		glDeleteTextures(1, &m_irradianceMap);
		glDeleteTextures(1, &m_prefilterMap);
		glDeleteTextures(1, &m_brdfLUTTexture);
		glDeleteFramebuffers(1, &m_captureFBO);
		glDeleteRenderbuffers(1, &m_captureRBO);		

		glDeleteVertexArrays(1, &m_cubeVAO);
		glDeleteBuffers(1, &m_cubeVBO);
		m_pc->graphiquePipelineManager->destroyPipeline(m_cubeMapCompute);
		m_pc->graphiquePipelineManager->destroyPipeline(m_cubeMapRender);
		m_pc->graphiquePipelineManager->destroyPipeline(m_cubeMapConvolutionCompute);
		m_pc->graphiquePipelineManager->destroyPipeline(m_cubeMapFilter);
	}
}