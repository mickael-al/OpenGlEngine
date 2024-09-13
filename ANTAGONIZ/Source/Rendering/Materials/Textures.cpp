#include "glcore.hpp"
#include "Textures.hpp"
#include "stb-cmake/stb_image.h"
#include <cmath>
#include <cstring>
#include "imgui-cmake/Header/imgui.h"
#include "GraphicsDataMisc.hpp"

namespace Ge
{
	Textures::Textures(stbi_uc* pc, int Width, int Height,unsigned int index, bool filter, bool mipmaps, GraphicsDataMisc * gdm)
	{		
		m_width = Width;
		m_height = Height;
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pc);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter ? GL_LINEAR : GL_NEAREST);
		if (mipmaps)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else 
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter ? GL_LINEAR : GL_NEAREST);
		}
		m_filter = filter;
		m_index = index;
	}

	Textures::Textures(unsigned int textureId,int Width, int Height, unsigned int index, bool filter, bool mipmaps, GraphicsDataMisc* gdm)
	{
		m_width = Width;
		m_height = Height;
		m_textureID = textureId;
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter ? GL_LINEAR : GL_NEAREST);
		if (mipmaps)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter ? GL_LINEAR : GL_NEAREST);
		}
		m_filter = filter;
		m_index = index;
	}

	Textures::Textures(unsigned int textureId, int Width, int Height, unsigned int index)
	{
		m_width = Width;
		m_height = Height;
		m_textureID = textureId;
		m_filter = true;
		m_index = index;
	}

	Textures::~Textures()
	{
		glDeleteTextures(1, &m_textureID);
	}

	unsigned int Textures::getIndex() const
	{
		return m_index;
	}

	unsigned int Textures::getTextureID() const
	{
		return m_textureID;
	}

	const bool Textures::getFilter() const
	{
		return m_filter;
	}

	int Textures::getWidth() const
	{
		return m_width;
	}

	int Textures::getHeight() const
	{
		return m_height;
	}

	void Textures::setIndex(unsigned int index)
	{
		m_index = index;
	}
}