#include "glcore.hpp"
#include "SSAOBuffer.hpp"
#include "Debug.hpp"
#include "GraphicsDataMisc.hpp"
#include <random>

namespace Ge
{
    float lerp(float a, float b, float f)
    {
        return a + f * (b - a);
    }

    bool SSAOBuffer::initialize(GraphicsDataMisc* gdm)
    {
        m_gdm = gdm;
        std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
        std::default_random_engine generator;
        for (unsigned int i = 0; i < 64; ++i)
        {
            glm::vec3 sample(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator)
            );

            sample = glm::normalize(sample);
            sample *= randomFloats(generator);


            float scale = (float)i / 64.0;
            scale = lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            m_ssaoKernel.push_back(sample);
        }

        glGenBuffers(1, &m_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3) * m_ssaoKernel.size(), m_ssaoKernel.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);


        for (unsigned int i = 0; i < 16; i++)
        {
            glm::vec3 noise(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                0.0f);
            m_ssaoNoise.push_back(noise);
        }
        
        glGenTextures(1, &m_noiseTexture);
        glBindTexture(GL_TEXTURE_2D, m_noiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &m_ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        Debug::INITSUCCESS("SSAOBuffer");
        return true;
    }

    unsigned int SSAOBuffer::getSSBO() const
    {
        return m_ssbo;
    }

    unsigned int SSAOBuffer::getNoiseTexture() const
    {
        return m_noiseTexture;
    }

    void SSAOBuffer::release()
    {
        glDeleteTextures(1,&m_noiseTexture);
        glDeleteBuffers(1, &m_ssbo);
    }
}