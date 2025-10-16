#include "Fog.hpp"
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
    bool Fog::initialize(GraphicsDataMisc* gdm)
    {
        m_pc = Engine::getPtrClassAddr();
        m_gdm = gdm;

        if (gdm->str_width > (unsigned int)INT_MAX || gdm->str_height > (unsigned int)INT_MAX)
        {
            Debug::Error("Window size conversion overflow - cannot build Oil painting FBO!");
            return false;
        }
        m_fog = m_pc->graphiquePipelineManager->createPipeline("../Asset/Shader/fog.fs.glsl", "../Asset/Shader/pp.vs.glsl");
        fog_size = glGetUniformLocation(m_fog->getProgram(), "u_resolution");
        fog_color = glGetUniformLocation(m_fog->getProgram(), "u_color");
        fog_min = glGetUniformLocation(m_fog->getProgram(), "u_min");
        fog_max = glGetUniformLocation(m_fog->getProgram(), "u_max");
        fog_mode = glGetUniformLocation(m_fog->getProgram(), "u_mode");
        fog_time = glGetUniformLocation(m_fog->getProgram(), "u_time");
        fog_cd = glGetUniformLocation(m_fog->getProgram(), "u_cameraData");
        return true;
    }

    void Fog::release()
    {
        m_pc->graphiquePipelineManager->destroyPipeline(m_fog);
    }

    void Fog::resize(int width, int height){ }

    void Fog::compute(unsigned int frameBuffer, unsigned int texture, unsigned int depthtexture, ShapeBuffer* sb, PPSetting* settings)
    {
        glm::vec2 size((float)m_gdm->str_width, (float)m_gdm->str_height);        
        if (settings->fog)
        {
            glViewport(0, 0, size.x, size.y);
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);            
            glUseProgram(m_fog->getProgram());
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, depthtexture);
            glUniform2f(fog_size, size.x, size.y);
            glUniform3f(fog_color, settings->fog_color.x, settings->fog_color.y, settings->fog_color.z);
            glUniform1f(fog_min, settings->fog_min_distance);
            glUniform1f(fog_max, settings->fog_max_distance);
            glUniform1i(fog_mode, settings->fog_mode);
            glUniform2f(fog_cd, m_gdm->current_camera->getNear(), m_gdm->current_camera->getFar());
            glUniform1f(fog_time, m_pc->time->GetTime());

            glBindVertexArray(sb->getVAO());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sb->getIBO());
            glDrawElements(GL_TRIANGLES, sb->getIndiceSize(), GL_UNSIGNED_INT, 0);
        }

        glViewport(0, 0, size.x, size.y);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Fog::onGui(PPSetting* t, PPSetting* c)
    {
        if (ImGui::TreeNodeEx("Fog"))
        {
            if (ImGui::Checkbox("Active##Fog", &c->fog))
            {
                if (t != nullptr) { t->fog = c->fog; }
            }
            if (ImGui::ColorEdit3("Color##Fog", &c->fog_color.x))
            {
                if (t != nullptr) { t->fog_color = c->fog_color; }
            }
            if (ImGui::DragFloat("Min Distance##Fog", &c->fog_min_distance, 0.01f))
            {
                if (t != nullptr) { t->fog_min_distance = c->fog_min_distance; }
            }
            if (ImGui::DragFloat("Max Distance##Fog", &c->fog_max_distance, 0.01f))
            {
                if (t != nullptr) { t->fog_max_distance = c->fog_max_distance; }
            }
            const char* fog_modes[] = { "Lineaire", "Exponentiel", "ExponentielSQRT"};
            if (ImGui::Combo("Mode##Fog", &c->fog_mode, fog_modes, IM_ARRAYSIZE(fog_modes)))
            {
                if (t != nullptr) { t->fog_mode = c->fog_mode; }
            }
            ImGui::TreePop();
        }
    }
}