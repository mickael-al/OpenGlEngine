#include "NoiseGeneration.hpp"

void NoiseGeneration::generatePerlinNoise(float* field, int size, float scale) const
{
    glm::vec3 pos;
    for (int z = 0; z < size; z++)
    {
        for (int y = 0; y < size; y++)
        {
            for (int x = 0; x < size; x++)
            {
                pos.x = x * m_scale + m_offset.x;
                pos.y = y * m_scale + m_offset.y;
                pos.z = z * m_scale + m_offset.z;
                field[x + y * size + z * size * size] = defaultCurve(glm::perlin(pos*scale));
            }
        }
    }
}

void NoiseGeneration::generateSimplexNoise(float* field, int size, float scale) const
{
    glm::vec3 pos;
    for (int z = 0; z < size; z++)
    {
        for (int y = 0; y < size; y++)
        {
            for (int x = 0; x < size; x++)
            {
                pos.x = x * m_scale + m_offset.x;
                pos.y = y * m_scale + m_offset.y;
                pos.z = z * m_scale + m_offset.z;
                field[x + y * size + z * size * size] = defaultCurve(glm::simplex(pos * scale));
            }
        }
    }
}

void NoiseGeneration::generateWorleyNoise(float* field, int size, float scale) const
{
    for (int z = 0; z < size; z++)
    {
        for (int y = 0; y < size; y++)
        {
            for (int x = 0; x < size; x++)
            {
                
            }
        }
    }
}

void NoiseGeneration::generateFractalNoise(float* field, int size, float scale) const
{

}

void NoiseGeneration::generateValueNoise(float* field, int size, float scale) const
{

}

void NoiseGeneration::generateGradientNoise(float* field, int size, float scale) const
{

}

void NoiseGeneration::generateBillowNoise(float* field, int size, float scale) const
{

}

void NoiseGeneration::generateRidgedMultifractalNoise(float* field, int size, float scale) const
{
}

void NoiseGeneration::generateMandelbrotNoise(float* field, int size, float scale) const
{

}