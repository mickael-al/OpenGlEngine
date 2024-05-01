#include "GraphiquePipelineManager.hpp"
#include "GraphiquePipeline.hpp"
#include "ShaderPair.hpp"
#include "Debug.hpp"
#include <algorithm>
#include "GraphicsDataMisc.hpp"
#include <algorithm>

namespace Ge
{
	bool GraphiquePipelineManager::initialize(GraphicsDataMisc *gdm)
	{
		m_gdm = gdm;
		for (int i = 0; i < m_fileNameShaders.size(); i++)
		{
			createPipeline(m_fileNameShaders[i]->Frag, m_fileNameShaders[i]->Vert, m_fileNameShaders[i]->back, m_fileNameShaders[i]->multiSampling, m_fileNameShaders[i]->transparency, m_fileNameShaders[i]->cullMode);
		}
		if (m_fileNameShaders.size() == 0)
		{
			m_gdm->str_default_pipeline = createPipeline("../Asset/Shader/opaque.fs.glsl", "../Asset/Shader/opaque.vs.glsl");
			m_gdm->str_default_pipeline_forward = createPipeline("../Asset/Shader/forward.fs.glsl", "../Asset/Shader/forward.vs.glsl");
			m_gdm->str_default_pipeline_shadow = createPipeline("../Asset/Shader/shadow.fs.glsl", "../Asset/Shader/shadow.vs.glsl");
		}
		return true;
	}

	void GraphiquePipelineManager::release()
	{
		m_fileNameShaders.clear();
		for (int i = 0; i < m_graphiquePipeline.size(); i++)
		{
			m_fileNameShaders.push_back(m_graphiquePipeline[i]->getShaderPair());
			delete (m_graphiquePipeline[i]);
		}
		m_graphiquePipeline.clear();
	}

	void GraphiquePipelineManager::addCustomRenderer(CustomRenderer * c)
	{
		m_customRenderers.push_back(c);
	}

	void GraphiquePipelineManager::removeCustomRenderer(CustomRenderer * c)
	{
		m_customRenderers.erase(std::remove(m_customRenderers.begin(), m_customRenderers.end(), c), m_customRenderers.end());
	}

	GraphiquePipeline * GraphiquePipelineManager::createPipeline(const std::string &frag, const std::string &vert, bool back, bool multiS, bool transparency, int cullmode)
	{
		ShaderPair * sp = new ShaderPair(frag, vert, back, multiS, transparency, cullmode);
		GraphiquePipeline * gp = new GraphiquePipeline(m_gdm, sp);
		m_graphiquePipeline.push_back(gp);
	
		return gp;
	}

	std::vector<GraphiquePipeline *> & GraphiquePipelineManager::GetPipelines()
	{
		return m_graphiquePipeline;
	}

	std::vector<CustomRenderer*> & GraphiquePipelineManager::getCustomRenderer()
	{
		return m_customRenderers;
	}

	void GraphiquePipelineManager::destroyPipeline(GraphiquePipeline * pipeline)
	{
		m_graphiquePipeline.erase(std::remove(m_graphiquePipeline.begin(), m_graphiquePipeline.end(), pipeline), m_graphiquePipeline.end());
		delete (pipeline);
	}
}