#ifndef __TEXTURE_2D_ARRAY_GL__
#define __TEXTURE_2D_ARRAY_GL__

#define GL_RGBA8 0x8058
#define GL_RGBA 0x1908
#define GL_UNSIGNED_BYTE 0x1401

#include <vector>
#include <string>

class Texture2DArrayGL
{
public:
    Texture2DArrayGL(int w, int h, int numLayers, unsigned int internalFmt = GL_RGBA8, unsigned int fmt = GL_RGBA, unsigned int pixelType = GL_UNSIGNED_BYTE);
    bool loadLayer(const std::string& filePath, int layerIndex, bool flip = true);
    void generateMipmaps();
    void bind(int unit) const;
    ~Texture2DArrayGL();
private:
    unsigned int textureID = 0;
    int width = 0;
    int height = 0;
    int layers = 0;
    unsigned int internalFormat;
    unsigned int format;
    unsigned int type;
};
#endif //!__TEXTURE_2D_ARRAY_GL__