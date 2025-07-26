#ifndef __NOISE_GENERATION__
#define __NOISE_GENERATION__

#include "glm/glm.hpp"
#include "glm/gtc/noise.hpp"
#include "glm/gtc/constants.hpp"
#include <functional>
#include <cmath>

class NoiseGeneration
{
public:
    using ControlCurve = std::function<float(float)>;

    NoiseGeneration() : defaultCurve([](float x) { return x; }) {}

    inline void setControlCurve(ControlCurve curve)
    {
        defaultCurve = curve;
    }

    inline void setScale(float scale)
    {
        m_scale = scale;
    }

    inline void setOffset(const glm::vec3 & offset)
    {
        m_offset = offset;
    }

    void generatePerlinNoise(float* field, int size,float scale) const;
    void generateSimplexNoise(float* field, int size, float scale) const;
    void generateWorleyNoise(float* field, int size, float scale) const;
    void generateFractalNoise(float* field, int size, float scale) const;
    void generateValueNoise(float* field, int size, float scale) const;
    void generateGradientNoise(float* field, int size, float scale) const;
    void generateBillowNoise(float* field, int size, float scale) const;
    void generateRidgedMultifractalNoise(float* field, int size, float scale) const;
    void generateMandelbrotNoise(float* field, int size, float scale) const;
private:
    ControlCurve defaultCurve;
    float m_scale = 1;
    glm::vec3 m_offset;
};


#endif //!__NOISE_GENERATION__