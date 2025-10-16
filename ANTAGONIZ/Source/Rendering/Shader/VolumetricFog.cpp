#include "VolumetricFog.hpp"
#include "glcore.hpp"
#include "GraphicsDataMisc.hpp"
#include "Debug.hpp"
#include "GraphiquePipelineManager.hpp"
#include "GraphiquePipeline.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"
#include "ShapeBuffer.hpp"
#include "Camera.hpp"

namespace Ge
{
    bool VolumetricFog::initialize(GraphicsDataMisc* gdm)
    {
        m_pc = Engine::getPtrClassAddr();
        m_gdm = gdm;

        if (gdm->str_width > (unsigned int)INT_MAX || gdm->str_height > (unsigned int)INT_MAX)
        {
            Debug::Error("Window size conversion overflow - cannot build Oil painting FBO!");
            return false;
        }
        m_vfog = m_pc->graphiquePipelineManager->createPipeline("../Asset/Shader/vfog.fs.glsl", "../Asset/Shader/pp.vs.glsl");
        fog_invProjection = glGetUniformLocation(m_vfog->getProgram(), "invProjection");
        fog_invView = glGetUniformLocation(m_vfog->getProgram(), "invView");
        fog_color = glGetUniformLocation(m_vfog->getProgram(), "color");
        fog_time = glGetUniformLocation(m_vfog->getProgram(), "time");
        fog_cp = glGetUniformLocation(m_vfog->getProgram(), "cameraPos");
        fog_cd = glGetUniformLocation(m_vfog->getProgram(), "cameraData");

        return true;
    }

    void VolumetricFog::release()
    {
        m_pc->graphiquePipelineManager->destroyPipeline(m_vfog);
    }

    void VolumetricFog::resize(int width, int height) { }

    void VolumetricFog::compute(unsigned int frameBuffer, unsigned int texture, unsigned int depthtexture, ShapeBuffer* sb, PPSetting* settings)
    {
        glm::vec2 size((float)m_gdm->str_width, (float)m_gdm->str_height);
        glViewport(0, 0, size.x, size.y);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glUseProgram(m_vfog->getProgram());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthtexture);
        glUniform3f(fog_color, 1, 1, 1);
        Camera* c = m_gdm->current_camera;
        glm::vec3 pos = c->getPosition();
        glUniform3f(fog_cp, pos.x, pos.y, pos.z);
        glUniform2f(fog_cd, c->getNear(), c->getFar());
        glUniform1f(fog_time, m_pc->time->GetTime());
        glm::mat4 p = glm::inverse(c->getProjectionMatrix());
        glUniformMatrix4fv(fog_invProjection, 1, GL_FALSE, &p[0][0]);
        glm::mat4 v = c->getModelMatrix();
        glUniformMatrix4fv(fog_invView, 1, GL_FALSE, &v[0][0]);

        glBindVertexArray(sb->getVAO());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sb->getIBO());
        glDrawElements(GL_TRIANGLES, sb->getIndiceSize(), GL_UNSIGNED_INT, 0);
        glViewport(0, 0, size.x, size.y);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void VolumetricFog::onGui(PPSetting* t, PPSetting* c)
    {
       
    }
}