#include "RenderTarget.h"

#define GLEW_STATIC 1
#include "GL/glew.h"

#include <cassert>

void RenderTarget::Create(uint32_t w, uint32_t h, bool hasDepth)
{
    // 1. creer l'objet FBO
    glGenFramebuffers(1, &m_FBO);
    // 2. creer les attachements (cad les textures)
    Bind();
    m_Width = w;
    m_Height = h;

    glGenTextures(1, &m_ColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_ColorBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, m_Width, m_Height);
    // desactive le mip-mapping en forcant le filtrage bilineaire
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // 3. attacher (et verifier) les buffers
    glFramebufferTexture2D(GL_FRAMEBUFFER
        , GL_COLOR_ATTACHMENT0, // ancre (attachment)
        GL_TEXTURE_2D, m_ColorBuffer, 0); // description de l'attachment


    // idem pour le depth buffer
    if (hasDepth)
    {
        glGenTextures(1, &m_DepthBuffer);
        glBindTexture(GL_TEXTURE_2D, m_DepthBuffer);
        glTexStorage2D(GL_TEXTURE_2D, 1
            , GL_DEPTH_COMPONENT24 // 1 pixel de depth en 24 bits entier
            , m_Width, m_Height);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER
            , GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D, m_DepthBuffer, 0);
    }


    GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(error == GL_FRAMEBUFFER_COMPLETE);
}

void RenderTarget::Destroy()
{
    glDeleteTextures(1, &m_DepthBuffer);
    m_DepthBuffer = 0;
    glDeleteTextures(1, &m_ColorBuffer);
    m_ColorBuffer = 0;
    glDeleteFramebuffers(1, &m_FBO);
    m_FBO = 0;
}

void RenderTarget::Bind() {
    // GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    // glViewport doit etre respecifie si != du backbuffer
    glViewport(0, 0, m_Width, m_Height);
}

void RenderTarget::Unbind() {
    // '0' indique le framebuffer par defaut (cad le back buffer)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}