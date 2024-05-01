#ifndef __ENGINE_GRAPHIQUE_PIPELINE__
#define __ENGINE_GRAPHIQUE_PIPELINE__

#include <iostream>

struct GraphicsDataMisc;
namespace Ge
{
	class ShaderPair;
}

namespace Ge
{
	enum ShaderType
	{
		VertexShader,
		GeometryShader,
		FragmentShader,
		ComputeShaderType,
	};

	class GraphiquePipeline
	{
	public:
		GraphiquePipeline(GraphicsDataMisc * gdm, ShaderPair * sp);
		~GraphiquePipeline();		
		inline unsigned int getProgram() { return m_program; }
		ShaderPair * getShaderPair() const;
	private:
		friend class ComputeShader;
		static bool LoadShader(std::string filename, ShaderType type, unsigned int * shader);
		static bool ValidateShader(unsigned int shader);
		bool Create();
	private:
		ShaderPair * m_shaderPair;
		GraphicsDataMisc * m_gdm;
		unsigned int m_program;
		unsigned int m_vertexShader = 0;
		unsigned int m_geometryShader = 0;
		unsigned int m_fragmentShader = 0;
	};
}

#endif //!__ENGINE_GRAPHIQUE_PIPELINE__