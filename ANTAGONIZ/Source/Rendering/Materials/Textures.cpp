#include "glcore.hpp"
#include "Textures.hpp"
#include "stb-cmake/stb_image.h"
#include <cmath>
#include <cstring>
#include "imgui-cmake/Header/imgui.h"
#include "GraphicsDataMisc.hpp"

namespace Ge
{
	Textures::Textures(stbi_uc* pc, int Width, int Height,unsigned int index, bool filter, GraphicsDataMisc * gdm)
	{		
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pc);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (m_enableMipmaps)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else 
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter ? GL_LINEAR : GL_NEAREST);
		}
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

	void Textures::setIndex(unsigned int index)
	{
		m_index = index;
	}
}