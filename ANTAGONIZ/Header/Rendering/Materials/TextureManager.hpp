#ifndef __ENGINE_TEXTURE_MANAGER__
#define __ENGINE_TEXTURE_MANAGER__

#include "Initializer.hpp"
#include <vector>

namespace Ge
{
	class Textures;
}

namespace Ge
{
    class TextureManager final : public InitializerAPI
    {
    private:
        friend class RenderingEngine;
        bool initialize(GraphicsDataMisc * gdm);
        void release();  
    public:
        Textures * createTexture(const char * path, bool filter = true,bool mipmaps = true);
        Textures * createTexture(const std::vector<unsigned char> & imageData, bool filter = true, bool mipmaps = true);
        Textures * createTexture(unsigned char* pixel, int tw, int th, bool filter = true, bool mipmaps = true);
        Textures* generateNormal(Textures* base, bool filter = true, bool mipmaps = true);
		void destroyTexture(Textures * texture);
		void saveFramebufferToPNG(const char* filename, int width, int height);
    private:
        std::vector<Textures *> m_textures;		
		GraphicsDataMisc * m_gdm;
    };
}

#endif //!__ENGINE_TEXTURE_MANAGER__