#include <glcore.hpp>
#include "ParticleSystem.hpp"
#include "ShaderUtil.hpp"
#include "Engine.hpp"
#include "Camera.hpp"
#include "RenderingEngine.hpp"
#include "ShaderPair.hpp"

namespace Ge
{
    ParticleSystem::ParticleSystem()
    {
        m_pc = Engine::getPtrClassAddr();
        m_allocator = new DynamicGPUAllocator(1024 * 128);
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glBindVertexArray(0);
        m_baseRender = m_pc->graphiquePipelineManager->createPipeline("../Asset/Shader/particle.fs.glsl", "../Asset/Shader/particle.vs.glsl", false,true,true,2);
        m_baseCs = new ComputeShader("../Asset/Shader/particle_base.C.glsl");
        m_init = new ComputeShader("../Asset/Shader/init_particle.C.glsl");
        std::vector<std::function<void(const glm::vec2& invResolution)>>& cb = m_pc->renderingEngine->getRenderTransparencyCallback();
        cb.push_back(std::bind(&ParticleSystem::render, this, std::placeholders::_1));
    }

    ParticleSystem::~ParticleSystem()
    {        
        m_pc->graphiquePipelineManager->destroyPipeline(m_baseRender);
        delete m_init;
        delete m_baseCs;
        glDeleteVertexArrays(1, &m_vao);
        delete m_allocator;
        auto & call = m_pc->renderingEngine->getRenderTransparencyCallback();
        call.clear();//TODO : faire une vrai supresion utiliser l'id du push_back
    }

    size_t ParticleSystem::createGroup(void* settings, size_t sizeSettings, size_t nbParticule, Textures* tex, ComputeShader* cs, GraphiquePipeline* gp)
    {        
        ParticleGroup group;
        group.offsetSettings = m_allocator->allocate(settings, sizeSettings);
        group.countSettings = size_t(double(sizeSettings + (sizeof(float)-1))/sizeof(float));
        group.offsetData = m_allocator->allocate(nullptr, nbParticule * sizeof(Particle));
        group.countData = nbParticule;
        if (cs != nullptr)
        {
            group.cs = cs;
        }
        else
        {
            group.cs = m_baseCs;
        }
        if (gp != nullptr)
        {
            group.pipeline = gp;
        }
        else
        {
            group.pipeline = m_baseRender;
        }
        group.texture = tex;    

        int x, y, z;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_allocator->getBufferID());        
        m_init->use();
        glUniform1ui(glGetUniformLocation(m_init->getProgram(), "u_offset"), (unsigned int)group.offsetData);
        glUniform1ui(glGetUniformLocation(m_init->getProgram(), "u_count"), (unsigned int)group.countData);        
        ShaderUtil::CalcWorkSize(group.countData, &x, &y, &z);
        m_init->dispatch(x, y, z);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        m_groups.push_back(group);
        return m_groups.size() - 1;
    }

    void ParticleSystem::updateGroup(size_t groupID, void* settings, size_t sizeSettings)
    {
        if (groupID >= m_groups.size())
        {
            return;
        }
        auto& group = m_groups[groupID];
        m_allocator->update(group.offsetSettings, settings, sizeSettings);
    }

    void ParticleSystem::removeGroup(size_t groupID)
    {
        if (groupID >= m_groups.size())
        {
            return;
        }
        auto& group = m_groups[groupID];
        m_allocator->free(group.offsetData);
        m_groups.erase(m_groups.begin() + groupID);
    }

    inline uint16_t PackScale(float scale)
    {
        scale = std::clamp(scale, 0.0f, 64.0f);
        return uint16_t((scale / 64.0f) * 65535.0f);
    }

    inline uint16_t PackVelocity(float v)
    {
        v /= 256.0f;
        v = std::clamp(v, -1.0f, 1.0f);

        float norm = (v + 1.0f) * 0.5f;
        return uint16_t(norm * 65535.0f);
    }

    void ParticleSystem::render(glm::vec2 invRes)
    {
        glBindVertexArray(m_vao);

        for (auto& group : m_groups)
        {
            ShaderPair* sp = group.pipeline->getShaderPair();
            if (sp->cullMode == 2)
            {
                glDisable(GL_CULL_FACE);
            }
            glUseProgram(group.pipeline->getProgram());
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_allocator->getBufferID());
            glUniform1ui(glGetUniformLocation(group.pipeline->getProgram(), "u_offset"), (unsigned int)group.offsetData);
            glUniform1ui(glGetUniformLocation(group.cs->getProgram(), "u_offsetSettings"), (unsigned int)group.offsetSettings);
            glUniform1ui(glGetUniformLocation(group.pipeline->getProgram(), "u_count"), (unsigned int)group.countData);
            glUniform2fv(glGetUniformLocation(group.pipeline->getProgram(), "u_invResolution"), 1, &invRes.x);
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_2D, group.texture->getTextureID());
            glDrawArrays(GL_TRIANGLES, 0, group.countData*3);
            if (sp->cullMode == 2)
            {
                glEnable(GL_CULL_FACE);
            }
            int x, y, z;
            group.cs->use();
            glUniform1ui(glGetUniformLocation(group.cs->getProgram(), "u_offset"), (unsigned int)group.offsetData);
            glUniform1ui(glGetUniformLocation(group.cs->getProgram(), "u_offsetSettings"), (unsigned int)group.offsetSettings);
            glUniform1ui(glGetUniformLocation(group.cs->getProgram(), "u_count"), (unsigned int)group.countData);
            glUniform1f(glGetUniformLocation(group.cs->getProgram(), "u_dt"), glm::clamp(m_pc->time->getDeltaTime(),0.0f,1.0f/32.0f));
            ShaderUtil::CalcWorkSize(group.countData, &x, &y, &z);
            group.cs->dispatch(x, y, z);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        }        

        glBindVertexArray(0);
        glUseProgram(0);
    }
}