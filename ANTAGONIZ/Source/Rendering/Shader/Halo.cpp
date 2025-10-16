#include "Halo.hpp"
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
    bool Halo::initialize(GraphicsDataMisc* gdm)
    {
        m_pc = Engine::getPtrClassAddr();
        m_gdm = gdm;

        if (gdm->str_width > (unsigned int)INT_MAX || gdm->str_height > (unsigned int)INT_MAX)
        {
            Debug::Error("Window size conversion overflow - cannot build Oil painting FBO!");
            return false;
        }
        m_halo = m_pc->graphiquePipelineManager->createPipeline("../Asset/Shader/halo.fs.glsl", "../Asset/Shader/pp.vs.glsl");

        halo_planetPos = glGetUniformLocation(m_halo->getProgram(), "planetPos");        
        halo_atmosphereRadius = glGetUniformLocation(m_halo->getProgram(), "atmosphereRadius");
        halo_invProjection = glGetUniformLocation(m_halo->getProgram(), "invProjection");
        halo_invView = glGetUniformLocation(m_halo->getProgram(), "invView");
        halo_cameraPos = glGetUniformLocation(m_halo->getProgram(), "cameraPos");
        halo_cameraData = glGetUniformLocation(m_halo->getProgram(), "cameraData");

        return true;
    }

    void Halo::release()
    {
        m_pc->graphiquePipelineManager->destroyPipeline(m_halo);
    }

    void Halo::resize(int width, int height) { }

    void Halo::compute(unsigned int frameBuffer, unsigned int texture, unsigned int depthtexture, ShapeBuffer* sb, PPSetting* settings)
    {
        glm::vec2 size((float)m_gdm->str_width, (float)m_gdm->str_height);
        glViewport(0, 0, size.x, size.y);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glUseProgram(m_halo->getProgram());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthtexture);

        glUniform3f(halo_planetPos, m_halo_planetPos.x, m_halo_planetPos.y, m_halo_planetPos.z);        
        glUniform1f(halo_atmosphereRadius, m_halo_atmosphereRadius);
        Camera* c = m_pc->cameraManager->getCurrentCamera();
        glUniform2f(halo_cameraData, c->getNear(), c->getFar());
        glm::vec3 pos = c->getPosition();
        glm::mat4 p = glm::inverse(c->getProjectionMatrix());
        glUniformMatrix4fv(halo_invProjection, 1, GL_FALSE, &p[0][0]);
        glm::mat4 v = c->getModelMatrix();
        glUniformMatrix4fv(halo_invView, 1, GL_FALSE, &v[0][0]);
        glUniform3f(halo_cameraPos, pos.x, pos.y, pos.z);

        glBindVertexArray(sb->getVAO());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sb->getIBO());
        glDrawElements(GL_TRIANGLES, sb->getIndiceSize(), GL_UNSIGNED_INT, 0);

        glViewport(0, 0, size.x, size.y);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Halo::onGui(PPSetting* t, PPSetting* c)
    {
        if (ImGui::TreeNodeEx("Halo"))
        {
            ImGui::DragFloat3("Planet Pos##Halo", &m_halo_planetPos.x);
            ImGui::DragFloat("Atmosphere Radius##Halo", &m_halo_atmosphereRadius, 0.01f);
            ImGui::TreePop();
        }
    }

    void Halo::setData(glm::vec3 pos, glm::vec2 ra)
    {
        m_halo_planetPos = pos;
        m_halo_atmosphereRadius = ra.y;
    }
}