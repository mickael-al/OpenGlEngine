#include "glcore.hpp"
#include "FrameBuffer.hpp"
#include "Debug.hpp"
#include "GraphicsDataMisc.hpp"

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPosition, 0);

        glGenTextures(1, &m_gNormal);
        glBindTexture(GL_TEXTURE_2D, m_gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormal, 0);

        glGenTextures(1, &m_gColorSpec);
        glBindTexture(GL_TEXTURE_2D, m_gColorSpec);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gColorSpec, 0);


        glGenTextures(1, &m_gOther);
        glBindTexture(GL_TEXTURE_2D, m_gOther);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_gOther, 0);

        glGenTextures(1, &m_gDepth);
        glBindTexture(GL_TEXTURE_2D, m_gDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_gdm->str_width, m_gdm->str_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_gDepth, 0);

        unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3 };
        glDrawBuffers(4, attachments);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::Error("Erreur lors de la cr�ation du framebuffer.");
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Debug::INITSUCCESS("FrameBuffer");
        return true;
    }

    void FrameBuffer::resize(int width, int height)
    {
        glBindTexture(GL_TEXTURE_2D, m_gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_2D, m_gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_2D, m_gColorSpec);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_2D, m_gOther);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGBA, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_2D, m_gDepth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_gdm->str_width, m_gdm->str_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::Error("Erreur lors de la recr�ation du framebuffer apr�s redimensionnement de la fen�tre.");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int FrameBuffer::getFrameBuffer() const
    {
        return m_framebuffer;
    }

    unsigned int FrameBuffer::getPosition() const
    {
        return m_gPosition;
    }

    unsigned int FrameBuffer::getNormal() const
    {
        return m_gNormal;
    }

    unsigned int FrameBuffer::getColorSpec() const
    {
        return m_gColorSpec;
    }

    unsigned int FrameBuffer::getOther() const
    {
        return m_gOther;
    }

    void FrameBuffer::release()
    {
        glDeleteFramebuffers(1, &m_framebuffer);
        glDeleteTextures(1, &m_gPosition);
        glDeleteTextures(1, &m_gNormal);
        glDeleteTextures(1, &m_gColorSpec);
        glDeleteTextures(1, &m_gOther);
        glDeleteTextures(1, &m_gDepth);
    }
}