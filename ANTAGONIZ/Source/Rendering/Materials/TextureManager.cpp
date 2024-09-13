#include "glcore.hpp"
#include "TextureManager.hpp"
#include "Textures.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"
#include "GraphiquePipeline.hpp"
#include "ShapeBuffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb-cmake/stb_image.h"
#define __STDC_LIB_EXT1__
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb-cmake/stb_image_write.h"

#include "imgui-cmake/Header/imgui.h"
#include "Debug.hpp"
#include "GraphicsDataMisc.hpp"
#include <algorithm>

#ifdef _WIN32
#include <filesystem>
namespace fs = std::filesystem;
#elif __unix__
#include <iostream>
#include <unistd.h>
#endif

namespace Ge
{
	bool TextureManager::initialize(GraphicsDataMisc * gdm)
	{
		m_gdm = gdm;
		unsigned char data[] = { 255,255,255,255 };
		Textures * default_texture = new Textures(data, 1, 1, 0, false,false, gdm);
		unsigned char data_norm[] = { 127,127,255,255 };
		Textures* default_normal_texture = new Textures(data_norm, 1, 1, 0, false,false, gdm);
		m_textures.push_back(default_texture);
		m_textures.push_back(default_normal_texture);
		m_gdm->str_default_texture = default_texture;
		m_gdm->str_default_normal_texture = default_normal_texture;
		m_gdm->str_dataMisc.textureCount = m_textures.size();
		Debug::INITSUCCESS("TextureManager");
		return true;
	}

	void TextureManager::release()
	{
		for (int i = 0; i < m_textures.size(); i++)
		{
			delete m_textures[i];
		}
		m_textures.clear();
		Debug::RELEASESUCCESS("TextureManager");
	}

#ifdef __unix__
	bool fileExists(const char* path) 
	{
		return access(path, F_OK) != -1;
	}
#endif
	Textures *TextureManager::createTexture(const char *path, bool filter, bool mipmaps)
	{
#ifdef _WIN32

		if (fs::exists(path))
#elif __unix__
		if (fileExists(path))
#endif
		{
			int tw, th, tc;
			unsigned char * pixel = stbi_load(path, &tw, &th, &tc, STBI_rgb_alpha);
			if (!pixel)
			{
				Debug::Warn("Echec du chargement de la texture");
				return nullptr;
			}
			Textures *texture = new Textures(pixel, tw, th, m_textures.size(), filter, mipmaps, m_gdm);
			m_textures.push_back(texture);
			stbi_image_free(pixel);
			m_gdm->str_dataMisc.textureCount = m_textures.size();
			return texture;
		}
		Debug::Warn("Le fichier n'existe pas");
		return nullptr;
	}

	Textures* TextureManager::generateNormal(Textures* base, bool filter, bool mipmaps)
	{
		unsigned int fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		unsigned int normalMap;
		glGenTextures(1, &normalMap);
		glBindTexture(GL_TEXTURE_2D, normalMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, base->getWidth(), base->getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalMap, 0);
		const ptrClass * pc = Engine::getPtrClassAddr();
		GraphiquePipeline * gpn =  pc->graphiquePipelineManager->createPipeline("../Asset/Shader/albedo_normal.fs.glsl", "../Asset/Shader/albedo_normal.vs.glsl");

		glUseProgram(gpn->getProgram());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, base->getTextureID());
		int us = glGetUniformLocation(gpn->getProgram(), "strength");
		glUniform1f(us, 2.5f);
		ShapeBuffer * sb = pc->modelManager->getFullScreenTriangle();
		glBindVertexArray(sb->getVAO());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sb->getIBO());
		glDrawElements(GL_TRIANGLES, sb->getIndiceSize(), GL_UNSIGNED_INT, 0);

		pc->graphiquePipelineManager->destroyPipeline(gpn);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		Textures* texture = new Textures(normalMap, base->getWidth(), base->getHeight(), m_textures.size(), filter, mipmaps, m_gdm);
		m_textures.push_back(texture);
		m_gdm->str_dataMisc.textureCount = m_textures.size();
		return texture;
	}

	Textures* TextureManager::createTexture(unsigned char * pixel, int tw, int th, bool filter,bool mipmaps)
	{
		if (!pixel)
		{
			Debug::Warn("Echec du chargement de la texture");
			return nullptr;
		}
		Textures* texture = new Textures(pixel, tw, th, m_textures.size(), filter, mipmaps, m_gdm);
		m_textures.push_back(texture);
		m_gdm->str_dataMisc.textureCount = m_textures.size();
		return texture;
	}

	void TextureManager::destroyTexture(Textures *texture)
	{
		m_textures.erase(std::remove(m_textures.begin(), m_textures.end(), texture), m_textures.end());
		delete(texture);		
		for(int i = 0; i < m_textures.size(); i++)
		{
			m_textures[i]->setIndex(i);
		}		
		m_gdm->str_dataMisc.textureCount = m_textures.size();		
	}

	void TextureManager::saveFramebufferToPNG(const char* filename, int width, int height)
	{
		std::vector<unsigned char> pixels(width * height * 4);
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
		
		for (int i = 0; i < height / 2; ++i) 
		{
			for (int j = 0; j < width * 4; ++j) 
			{
				std::swap(pixels[i * width * 4 + j], pixels[(height - i - 1) * width * 4 + j]);
			}
		}

		stbi_write_png(filename, width, height, 4, pixels.data(), width * 4);
	}
}