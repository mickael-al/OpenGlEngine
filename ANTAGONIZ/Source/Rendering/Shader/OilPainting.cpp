#include "OilPainting.hpp"
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
    bool OilPainting::initialize(GraphicsDataMisc* gdm)
    {
        m_pc = Engine::getPtrClassAddr();
        m_gdm = gdm;

        if (gdm->str_width > (unsigned int)INT_MAX || gdm->str_height > (unsigned int)INT_MAX)
        {
            Debug::Error("Window size conversion overflow - cannot build Oil painting FBO!");
            return false;
        }
        m_oilPainting = m_pc->graphiquePipelineManager->createPipeline("../Asset/Shader/oilpainting.fs.glsl", "../Asset/Shader/pp.vs.glsl");
        oilp_size = glGetUniformLocation(m_oilPainting->getProgram(), "u_resolution");
        oilp_radius = glGetUniformLocation(m_oilPainting->getProgram(), "u_radius");

        glGenTextures(1, &m_fColor);
        glBindTexture(GL_TEXTURE_2D, m_fColor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        return true;
    }

    void OilPainting::release()
    {
        m_pc->graphiquePipelineManager->destroyPipeline(m_oilPainting);
    }

    void OilPainting::resize(int width, int height)
    {
        glBindTexture(GL_TEXTURE_2D, m_fColor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, m_gdm->str_width, m_gdm->str_height, 0, GL_RGB, GL_FLOAT, NULL);
    }

    void OilPainting::compute(unsigned int frameBuffer, unsigned int texture, unsigned int depthtexture, ShapeBuffer* sb, PPSetting* settings)
    {
        glm::vec2 size((float)m_gdm->str_width, (float)m_gdm->str_height);
        if (settings->oil_painting)
        {
            glCopyImageSubData(
                texture, GL_TEXTURE_2D, 0, 0, 0, 0,
                m_fColor, GL_TEXTURE_2D, 0, 0, 0, 0,
                size.x, size.y, 1
            );
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
            glViewport(0, 0, size.x, size.y);
            glUseProgram(m_oilPainting->getProgram());
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_fColor);
            glUniform2f(oilp_size, size.x, size.y);
            glUniform1f(oilp_radius, settings->oilp_radius);
            glUniform1f(glGetUniformLocation(m_oilPainting->getProgram(), "u_time"), m_pc->time->GetTime());

            glBindVertexArray(sb->getVAO());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sb->getIBO());
            glDrawElements(GL_TRIANGLES, sb->getIndiceSize(), GL_UNSIGNED_INT, 0);            
        }

        glViewport(0, 0, size.x, size.y);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OilPainting::onGui(PPSetting* t, PPSetting* s)
    {
        if (ImGui::TreeNodeEx("Oil Painting"))
        {
            if (ImGui::Checkbox("Active##OP", &s->oil_painting))
            {
                if (t != nullptr) { t->oil_painting = s->oil_painting; }
            }
            int or = s->oilp_radius;
            if (ImGui::DragInt("Radius##OP", &or , 0.01f))
            {
                s->oilp_radius = or;
                if (t != nullptr) { t->oilp_radius = s->oilp_radius; }
            }
            ImGui::TreePop();
        }
    }
}