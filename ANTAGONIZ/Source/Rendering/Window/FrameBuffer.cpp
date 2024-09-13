#include "glcore.hpp"
#include "FrameBuffer.hpp"
#include "Debug.hpp"
#include "GraphicsDataMisc.hpp"
#include "Textures.hpp"

namespace Ge
{
    bool FrameBuffer::initialize(GraphicsDataMisc* gdm)
    {
        m_gdm = gdm;

        glEnable(GL_MULTISAMPLE);

        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

        glGenTextures(1, &m_gPosition);
        glBindTexture(GL_TEXTURE_2D, m_gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPosition, 0);

        glGenTextures(1, &m_gNormal);
        glBindTexture(GL_TEXTURE_2D, m_gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormal, 0);

        glGenTextures(1, &m_gColor);
        glBindTexture(GL_TEXTURE_2D, m_gColor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gColor, 0);

        glGenTextures(1, &m_gOther);
        glBindTexture(GL_TEXTURE_2D, m_gOther);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_gdm->str_width, m_gdm->str_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_gOther, 0);

        glGenTextures(1, &m_gDepth);
        glBindTexture(GL_TEXTURE_2D, m_gDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_gdm->str_width, m_gdm->str_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_gDepth, 0);

        gdm->str_depth_texture = new Textures(m_gDepth, m_gdm->str_width, m_gdm->str_height);

        unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3 };
        glDrawBuffers(4, attachments);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::Error("Erreur lors de la creation du framebuffer.");
            return false;
        }

        glGenFramebuffers(1, &m_framebufferFoward);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferFoward);

        glGenTextures(1, &m_fColor);
        glBindTexture(GL_TEXTURE_2D, m_fColor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fColor, 0);

        unsigned int attachmentsF[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, attachmentsF);

        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::Error("Erreur lors de la creation du framebuffer forward.");
            return false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Debug::INITSUCCESS("FrameBuffer");
        return true;
    }

    void FrameBuffer::resize(int width, int height)
    {        
        glBindTexture(GL_TEXTURE_2D, m_gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGB, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_2D, m_gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_2D, m_gColor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_2D, m_gOther);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_gdm->str_width, m_gdm->str_height, 0, GL_RGB, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_2D, m_gDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_gdm->str_width, m_gdm->str_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::Error("Erreur lors de la recreation du framebuffer apres redimensionnement de la fenêtre.");
        }        

        glBindTexture(GL_TEXTURE_2D, m_fColor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGB, GL_FLOAT, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferFoward);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::Error("Erreur lors de la recreation du framebuffer forward apres redimensionnement de la fenêtre.");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int FrameBuffer::getFrameBuffer() const
    {
        return m_framebuffer;
    }

    unsigned int FrameBuffer::getFowardFrameBuffer() const
    {
        return m_framebufferFoward;
    }

    unsigned int FrameBuffer::getPosition() const
    {
        return m_gPosition;
    }

    unsigned int FrameBuffer::getNormal() const
    {
        return m_gNormal;
    }

    unsigned int FrameBuffer::getColor() const
    {
        return m_gColor;
    }

    unsigned int FrameBuffer::getColorFoward() const
    {
        return m_fColor;
    }

    unsigned int FrameBuffer::getOther() const
    {
        return m_gOther;
    }

    unsigned int FrameBuffer::getDepth() const
    {
        return m_gDepth;
    }

    void FrameBuffer::release()
    {        
        glDeleteFramebuffers(1, &m_framebuffer);
        glDeleteTextures(1, &m_gPosition);
        glDeleteTextures(1, &m_gNormal);
        glDeleteTextures(1, &m_gColor);
        glDeleteTextures(1, &m_gOther);        
        glDeleteTextures(1, &m_gDepth);
        glDeleteFramebuffers(1, &m_framebufferFoward);
        glDeleteTextures(1, &m_fColor);
        delete m_gdm->str_depth_texture;
    }
}