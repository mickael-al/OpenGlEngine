#ifndef __ENGINE_TEXTURES__
#define __ENGINE_TEXTURES__

struct GraphicsDataMisc;
typedef unsigned char stbi_uc;

namespace Ge
{
	class Textures
	{
	public:
		Textures(stbi_uc* pc, int Width, int Height, unsigned int index, bool filter, GraphicsDataMisc * gdm);
		~Textures();		
		unsigned int getIndex() const;
		void setIndex(unsigned int index);
		unsigned int getTextureID() const;
	private:
		bool m_enableMipmaps = false;
		unsigned int m_index;
		unsigned int m_textureID;
	};
}

#endif //!__ENGINE_TEXTURES__