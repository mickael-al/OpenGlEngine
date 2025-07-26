#include "ImageData.hpp"
#include "stb-cmake/stb_image.h"
#include "stb-cmake/stb_image_write.h"


ImageData::ImageData(const char* src, const char* dst) : ImageData(src)
{
    m_path = dst;
}

ImageData::ImageData(ImageData* id, int width, int height)
{
    m_channel_count = id->m_channel_count;
    m_height = height;
    m_width = width;    
    m_path = id->m_path;
    m_color = new Color * [m_height];
    for (int i = 0; i < m_height; i++)
    {
        m_color[i] = new Color[m_width];
    }
    m_isload = true;
}

ImageData::ImageData(const char * file_name)
{
    unsigned char* data = stbi_load(file_name, &m_width, &m_height, &m_channel_count, 0);
    m_path = file_name;
    if (data == NULL)
    {
        std::cerr << "Error loading image." << std::endl;
        m_isload = false;
        return;
    }
    m_color = new Color * [m_height];
    for (int i = 0; i < m_height; i++) 
    {
        m_color[i] = new Color[m_width];
    }

    int offset = 0;   

    if (m_channel_count == 1)
    {
        for (int i = 0; i < m_height; i++)
        {
            for (int j = 0; j < m_width; j++)
            {
                offset = (i * m_width + j);
                m_color[i][j] = Color(data[offset], 255, 255, 255);
            }
        }
    }
    else if (m_channel_count == 3)
    {
        for (int i = 0; i < m_height; i++)
        {
            for (int j = 0; j < m_width; j++)
            {
                offset = (i * m_width + j) * m_channel_count;
                m_color[i][j] = Color(data[offset], data[offset + 1], data[offset + 2], 255);
            }
        }
    }
    else if (m_channel_count == 4)
    {
        for (int i = 0; i < m_height; i++)
        {
            for (int j = 0; j < m_width; j++)
            {
                offset = (i * m_width + j) * m_channel_count;
                m_color[i][j] = Color(data[offset], data[offset + 1], data[offset + 2], data[offset + 3]);
            }
        }        
    }
    stbi_image_free(data);
    m_isload = true;
}

Color** ImageData::getColor() const 
{ 
    return m_color; 
}
int ImageData::getWidth() const 
{ 
    return m_width; 
}

int ImageData::getHeight() const 
{ 
    return m_height; 
}

int ImageData::getChannelCount() const 
{ 
    return m_channel_count; 
}

bool ImageData::isLoad() const
{
    return m_isload;
}

std::string ImageData::getPath() const
{
    return m_path;
}

stbimg ImageData::loadData(const std::string path)
{
    stbimg stbi;

    stbi.data = stbi_load(path.c_str(), &stbi.width, &stbi.height, &stbi.ch, 0);
    return stbi;
}

void ImageData::freeData(stbimg stbi)
{
    stbi_image_free(stbi.data);
}

void ImageData::write()
{
    int size = m_width * m_height * m_channel_count;
    unsigned char* data = new unsigned char[size];

    int offset = 0;
    for (int i = 0; i < m_height; i++)
    {
        for (int j = 0; j < m_width; j++)
        {
            offset = (i * m_width + j) * m_channel_count;
            data[offset] = m_color[i][j].GetR();
            data[offset + 1] = m_color[i][j].GetG();
            data[offset + 2] = m_color[i][j].GetB();
            if (m_channel_count == 4)
            {
                data[offset + 3] = m_color[i][j].GetA();
            }
        }
    }

    if (m_channel_count == 4)
    {
        stbi_write_png(m_path.c_str(), m_width, m_height, m_channel_count, data, m_width * m_channel_count);
    }
    else if (m_channel_count == 3)
    {
      //  stbi_write_jpg(m_path.c_str(), m_width, m_height, m_channel_count, data, m_width * m_channel_count);
    }
    delete[] data;
}


ImageData::~ImageData()
{
    for (int i = 0; i < m_height; i++)
    {
        delete[] m_color[i];
    }
    delete[] m_color;
}