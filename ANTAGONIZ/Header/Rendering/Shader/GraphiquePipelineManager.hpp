#ifndef __ENGINE_GRAPHIQUE_PIPELINE_MANAGER__
#define __ENGINE_GRAPHIQUE_PIPELINE_MANAGER__

#include "Initializer.hpp"
#include "Manager.hpp"
#include <map>
#include <vector>
#include <string>

namespace Ge
{
	class GraphiquePipeline;
	class ShaderPair;
}

class CustomRenderer;

namespace Ge
{
	class GraphiquePipelineManager final : InitializerAPI
	{
	protected:
		friend class RenderingEngine;
		bool initialize(GraphicsDataMisc *gdm);
		void release();
	public:
		GraphiquePipeline * createPipeline(const std::string & frag, const std::string & vert, bool back = false, bool multiS = true, bool transparency = false, int cullmode = 1);		
		void destroyPipeline(GraphiquePipeline * pipeline);
		std::vector<GraphiquePipeline *> & GetPipelines();
		std::vector<CustomRenderer*> & getCustomRenderer();
		void addCustomRenderer(CustomRenderer * c);
		void removeCustomRenderer(CustomRenderer * c);
	private:
		GraphicsDataMisc * m_gdm;
		std::vector<ShaderPair*> m_fileNameShaders;
		std::vector<GraphiquePipeline*> m_graphiquePipeline;//TODO Use ** to release shader without remove first *
		std::vector<CustomRenderer*> m_customRenderers;
	};
}

#endif //!__ENGINE_GRAPHIQUE_PIPELINE_MANAGER__