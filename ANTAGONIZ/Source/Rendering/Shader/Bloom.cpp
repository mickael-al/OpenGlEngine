#include "Bloom.hpp"
#include "glcore.hpp"
#include "GraphicsDataMisc.hpp"
#include "Debug.hpp"
#include "GraphiquePipelineManager.hpp"
#include "GraphiquePipeline.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"
#include "ShapeBuffer.hpp"

namespace Ge
{	
	bool Bloom::initialize(GraphicsDataMisc* gdm)
	{
        const ptrClass& pc = Engine::getPtrClass();
		m_gdm = gdm;
                
        glm::vec2 mipSize((float)gdm->str_width, (float)gdm->str_height);
        glm::ivec2 mipIntSize((int)gdm->str_width, (int)gdm->str_height);
    
        if (gdm->str_width > (unsigned int)INT_MAX || gdm->str_height > (unsigned int)INT_MAX) 
        {
            Debug::Error("Window size conversion overflow - cannot build bloom FBO!");
            return false;
        }

        for (unsigned int i = 0; i < mipChainLength; i++)
        {
            bloomMip mip;

            mipSize *= 0.5f;
            mipIntSize /= 2;
            mip.size = mipSize;
            mip.intSize = mipIntSize;

            glGenTextures(1, &mip.texture);
            glBindTexture(GL_TEXTURE_2D, mip.texture);
            // we are downscaling an HDR color buffer, so we need a float texture format
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
                (int)mipSize.x, (int)mipSize.y,
                0, GL_RGB, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            mMipChain.emplace_back(mip);
        }

        m_downSampleShader = pc.graphiquePipelineManager->createPipeline("../Asset/Shader/downsampling.fs.glsl", "../Asset/Shader/pp.vs.glsl");
        m_upSampleShader = pc.graphiquePipelineManager->createPipeline("../Asset/Shader/upsampling.fs.glsl", "../Asset/Shader/pp.vs.glsl");
        m_tomeGammaShader = pc.graphiquePipelineManager->createPipeline("../Asset/Shader/tomeMapping.fs.glsl", "../Asset/Shader/pp.vs.glsl");
        srcResolutionLocation = glGetUniformLocation(m_downSampleShader->getProgram(), "srcResolution");
        thresholdLocation = glGetUniformLocation(m_downSampleShader->getProgram(), "threshold");
        filterRadiusLocation = glGetUniformLocation(m_upSampleShader->getProgram(), "filterRadius");
        gammaLocation = glGetUniformLocation(m_tomeGammaShader->getProgram(), "gamma");
        exposureLocation = glGetUniformLocation(m_tomeGammaShader->getProgram(), "exposure");        
        intensityLocation = glGetUniformLocation(m_tomeGammaShader->getProgram(), "intensity");
        //threshold;

		return true;
	}
	
	void Bloom::release()
	{
        const ptrClass& pc = Engine::getPtrClass();
        for (unsigned int i = 0; i < mMipChain.size(); i++)
        {
            glDeleteTextures(1, &mMipChain[i].texture);
        }
        mMipChain.clear();
        
        pc.graphiquePipelineManager->destroyPipeline(m_downSampleShader);
        pc.graphiquePipelineManager->destroyPipeline(m_upSampleShader);
        pc.graphiquePipelineManager->destroyPipeline(m_tomeGammaShader);
	}

	void Bloom::resize(int width, int height)
	{
        glm::vec2 mipSize((float)m_gdm->str_width, (float)m_gdm->str_height);
        glm::ivec2 mipIntSize((int)m_gdm->str_width, (int)m_gdm->str_height);
        for (unsigned int i = 0; i < mMipChain.size(); i++)
        {
            glBindTexture(GL_TEXTURE_2D, mMipChain[i].texture);

            mipSize *= 0.5f;
            mipIntSize /= 2;
            mMipChain[i].size = mipSize;
            mMipChain[i].intSize = mipIntSize;

            glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, (int)mipSize.x, (int)mipSize.y, 0, GL_RGB, GL_FLOAT, nullptr);
        }
	}

	void Bloom::compute(unsigned int frameBuffer, unsigned int texture, ShapeBuffer* sb, PPSetting* settings)
	{          
        glm::vec2 size((float)m_gdm->str_width, (float)m_gdm->str_height);
        float filterRadius = settings->bloom_filter;
        if (settings->bloom)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

            glUseProgram(m_downSampleShader->getProgram());
            glUniform2f(srcResolutionLocation, size.x, size.y);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            for (int i = 0; i < mMipChain.size(); i++)
            {
                const bloomMip& mip = mMipChain[i];
                glViewport(0, 0, mip.size.x, mip.size.y);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

                glBindVertexArray(sb->getVAO());
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sb->getIBO());
                glDrawElements(GL_TRIANGLES, sb->getIndiceSize(), GL_UNSIGNED_INT, 0);

                // Set current mip resolution as srcResolution for next iteration            
                glUniform2f(srcResolutionLocation, mip.size.x, mip.size.y);
                glUniform1f(thresholdLocation, i == 0 ? settings->bloom_threshold : -1.0f);
                // Set current mip as texture input for next iteration
                glBindTexture(GL_TEXTURE_2D, mip.texture);
            }

            glUseProgram(m_upSampleShader->getProgram());
            glUniform1f(filterRadiusLocation, filterRadius);

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);

            for (int i = mMipChain.size() - 1; i > 0; i--)
            {
                const bloomMip& mip = mMipChain[i];
                const bloomMip& nextMip = mMipChain[i - 1];

                // Bind viewport and texture from where to read
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mip.texture);

                // Set framebuffer render target (we write to this texture)
                glViewport(0, 0, nextMip.size.x, nextMip.size.y);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, nextMip.texture, 0);

                // Render screen-filled quad of resolution of current mip
                glBindVertexArray(sb->getVAO());
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sb->getIBO());
                glDrawElements(GL_TRIANGLES, sb->getIndiceSize(), GL_UNSIGNED_INT, 0);
            }
        }
        // Disable additive blending
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Restore if this was default
        glDisable(GL_BLEND);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        glViewport(0, 0, size.x, size.y);
        glUseProgram(m_tomeGammaShader->getProgram());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mMipChain[1].texture);
        glUniform1f(gammaLocation, settings->gamma == nullptr ? 2.2f : *settings->gamma);
        glUniform1f(exposureLocation, settings->exposure);
        glUniform1f(intensityLocation, settings->bloom ? settings->bloom_intensity : 0.0f);
        
        glBindVertexArray(sb->getVAO());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sb->getIBO());
        glDrawElements(GL_TRIANGLES, sb->getIndiceSize(), GL_UNSIGNED_INT, 0);

        
        glViewport(0, 0, size.x, size.y);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);        
	}
}