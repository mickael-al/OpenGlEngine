#ifndef __ENGINE_GRAPHIQUE_PIPELINE_MANAGER__
#define __ENGINE_GRAPHIQUE_PIPELINE_MANAGER__
/*
#include "VulkanMisc.hpp"
#include "Debug.hpp"
#include <vector>
#include "GraphiquePipeline.hpp"
#include <map>

namespace Ge
{
    struct GPData
    {
        GraphiquePipeline* gp;
    };

    class GraphiquePipelineManager
    {
    public:
        bool initialize(VulkanMisc *vM);
        void release();
		GraphiquePipeline * createPipeline(const std::string & frag,const std::string & vert,bool back = false, bool multiS = true,bool transparency = false,int cullmode = 1);        
        //GPData * createPipelineD(const std::string& frag, const std::string& vert, bool back = false, bool multiS = true, bool transparency = false, int cullmode = 1);
        void destroyPipeline(GraphiquePipeline * pipeline);
		static std::vector<GraphiquePipeline *> & GetPipelines();
    private:
        VulkanMisc *vulkanM;
        std::vector<ShaderPair *> m_fileNameShaders;
        static std::vector<GraphiquePipeline *> m_graphiquePipeline;
        std::vector<GPData*> m_gpdata;

    };
}
*/
#endif //__ENGINE_GRAPHIQUE_PIPELINE_MANAGER__