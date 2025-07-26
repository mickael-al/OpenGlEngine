#include <glcore.hpp>
#include "stb-cmake/stb_image.h"
#include "Texture2DArrayGL.hpp"
#include <vector>
#include <string>
#include <iostream>

Texture2DArrayGL::Texture2DArrayGL(int w, int h, int numLayers,unsigned int internalFmt,unsigned int fmt,unsigned int pixelType)
{
    width = w;
    height = h;
    layers = numLayers;
    internalFormat = internalFmt;
    format = fmt;
    type = pixelType;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat,width, height, layers, 0, format, type, nullptr);

    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

bool Texture2DArrayGL::loadLayer(const std::string& filePath, int layerIndex, bool flip) 
{
    if (layerIndex < 0 || layerIndex >= layers) 
    {
        std::cerr << "Layer index out of range\n";
        return false;
    }

    stbi_set_flip_vertically_on_load(flip);
    int imgW, imgH, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &imgW, &imgH, &channels, 4);
    if (!data) 
    {
        std::cerr << "Failed to load image: " << filePath << "\n";
        return false;
    }

    if (imgW != width || imgH != height) 
    {
        std::cerr << "Image dimensions do not match array size: " << filePath << "\n";
        stbi_image_free(data);
        return false;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,0, 0, layerIndex,width, height, 1,format, type, data);

    stbi_image_free(data);
    return true;
}

void Texture2DArrayGL::generateMipmaps()
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

void Texture2DArrayGL::bind(int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
}

Texture2DArrayGL::~Texture2DArrayGL()
{
    if (textureID != 0) 
    {
        glDeleteTextures(1, &textureID);
    }
}