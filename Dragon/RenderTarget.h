#pragma once

#include <cstdint>

struct RenderTarget
{
    uint32_t m_FBO = 0;
    uint32_t m_ColorBuffer = 0;
    uint32_t m_DepthBuffer = 0;

    uint32_t m_Width = 0;
    uint32_t m_Height = 0;

    void Create(uint32_t w, uint32_t h, bool hasDepth = false);
    void Destroy();

    void Bind();
    void Unbind();
};


