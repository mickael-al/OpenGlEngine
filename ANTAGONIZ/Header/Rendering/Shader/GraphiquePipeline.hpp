#ifndef __ENGINE_GRAPHIQUE_PIPELINE__
#define __ENGINE_GRAPHIQUE_PIPELINE__

/*
#include "Debug.hpp"

namespace Ge
{
    class GraphiquePipeline
    {
    public:
        GraphiquePipeline(VulkanMisc *vM, ShaderPair * sp);
        ~GraphiquePipeline();
		int getIndex() const;
		ShaderPair * getShaderPair() const;
        static ShaderElement LoadShader(const std::string &filename, const char *entry, VkDevice device, bool isVertex,VulkanMisc * vM);
        static void DestroyShaderElement(VkDevice device, ShaderElement se);
        static std::vector<char> readFile(const std::string &filename);
        static VkShaderModule createShaderModule(const std::vector<char> &code, VkDevice device);
    public:
        GraphiquePipelineElement m_graphiquePipelineElement;		
    private:
		ShaderPair * m_shaderPair;
        VulkanMisc *vulkanM;
		int m_index;
    };
}
*/
#endif //__ENGINE_GRAPHIQUE_PIPELINE__