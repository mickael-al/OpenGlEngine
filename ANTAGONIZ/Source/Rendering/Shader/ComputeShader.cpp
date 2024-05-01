#include "glcore.hpp"
#include "GraphiquePipeline.hpp"
#include "ComputeShader.hpp"
#include "Debug.hpp"

namespace Ge
{
	ComputeShader::ComputeShader(std::string filename)
	{
		GraphiquePipeline::LoadShader(filename, ShaderType::ComputeShaderType, &m_computeShader);
		m_computeProgram = glCreateProgram();

		glAttachShader(m_computeProgram, m_computeShader);
		glLinkProgram(m_computeProgram);
		int32_t linked = 0;
		int32_t infoLen = 0;
		glGetProgramiv(m_computeProgram, GL_LINK_STATUS, &linked);

		if (!linked)
		{
			glGetProgramiv(m_computeProgram, GL_INFO_LOG_LENGTH, &infoLen);

			if (infoLen > 1)
			{
				char* infoLog = new char[infoLen + 1];

				glGetProgramInfoLog(m_computeProgram, infoLen, NULL, infoLog);
				Debug::Error("Erreur de lien du programme: %s", infoLog);

				delete(infoLog);
			}

			glDeleteProgram(m_computeProgram);
		}
	}

	void ComputeShader::use()
	{
		glUseProgram(m_computeProgram);
	}

	void ComputeShader::dispatch(int num_groups_x, int num_groups_y, int num_groups_z)
	{
		glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
	}

	unsigned int ComputeShader::getProgram() const
	{
		return m_computeProgram;
	}

	ComputeShader::~ComputeShader()
	{
		glDeleteShader(m_computeShader);
		glDeleteProgram(m_computeProgram);
	}
}