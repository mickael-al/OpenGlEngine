#include "glcore.hpp"
#include "GraphiquePipeline.hpp"
#include "Debug.hpp"
#include "ShaderPair.hpp"
#include "GraphicsDataMisc.hpp"
#include <fstream>
#include <iostream>

namespace Ge
{	
	GraphiquePipeline::GraphiquePipeline(GraphicsDataMisc * gdm, ShaderPair * sp,std::string define)
	{
		m_gdm = gdm;
		m_shaderPair = sp;
		if (!LoadShader(m_shaderPair->Vert, ShaderType::VertexShader, &m_vertexShader, define))
		{
			Debug::Error("Init Vertex Shader : %s ", m_shaderPair->Vert.c_str());
		}
		if (!LoadShader(m_shaderPair->Frag, ShaderType::FragmentShader, &m_fragmentShader, define))
		{
			Debug::Error("Init Fragment Shader : %s ", m_shaderPair->Frag.c_str());
		}
		//LoadShader(m_shaderPair->Geom.c_str, ShaderType::GeometryShader, &m_geometryShader);
		if (!Create())
		{
			Debug::Error("Init Program Shader");
		}
	}

	GraphiquePipeline::~GraphiquePipeline()
	{
		glDetachShader(m_program, m_vertexShader);
		glDetachShader(m_program, m_fragmentShader);
		glDetachShader(m_program, m_geometryShader);
		glDeleteShader(m_geometryShader);
		glDeleteShader(m_vertexShader);
		glDeleteShader(m_fragmentShader);
		glDeleteProgram(m_program);
	}
	
	ShaderPair * GraphiquePipeline::getShaderPair() const
	{
		return m_shaderPair;
	}

	bool GraphiquePipeline::ValidateShader(unsigned int shader)
	{
		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

		if (!compiled)
		{
			GLint infoLen = 0;

			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

			if (infoLen > 1)
			{
				char* infoLog = new char[1 + infoLen];

				glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
				std::cout << "Error compiling shader:" << infoLog << std::endl;

				delete[] infoLog;
			}

			glDeleteShader(shader);

			return false;
		}

		return true;
	}

	bool GraphiquePipeline::Create()
	{
		m_program = glCreateProgram();

		glAttachShader(m_program, m_vertexShader);
		if (m_geometryShader)
		{
			glAttachShader(m_program, m_geometryShader);
		}
		glAttachShader(m_program, m_fragmentShader);
		glLinkProgram(m_program);
		int32_t linked = 0;
		int32_t infoLen = 0;
		glGetProgramiv(m_program, GL_LINK_STATUS, &linked);

		if (!linked)
		{
			glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &infoLen);

			if (infoLen > 1)
			{
				char* infoLog = new char[infoLen + 1];

				glGetProgramInfoLog(m_program, infoLen, NULL, infoLog);
				if (infoLen < BUFFER_SIZE)
				{
					Debug::Error("Erreur de lien du programme: %s", infoLog);
				}
				else
				{
					std::cerr << infoLog << std::endl;
				}

				delete(infoLog);
			}

			glDeleteProgram(m_program);

			return false;
		}

		return true;
	}

	bool GraphiquePipeline::LoadShader(std::string filename, ShaderType type, unsigned int* shader, std::string defines)
	{
		std::ifstream fin(filename, std::ios::in | std::ios::binary);
		if (!fin.good())
		{
			Debug::Error("Le fichier n'existe pas : %s", filename.c_str());
			return false;
		}

		fin.seekg(0, std::ios::end);
		uint32_t length = (uint32_t)fin.tellg();
		fin.seekg(0, std::ios::beg);

		char* buffer = new char[length + 1];
		buffer[length] = '\0';
		fin.read(buffer, length);
		fin.close();

		std::string shaderCode(buffer);
		delete[] buffer;

		size_t versionPos = shaderCode.find("#version");
		size_t nextLinePos = (versionPos != std::string::npos) ? shaderCode.find("\n", versionPos) : std::string::npos;

		if (nextLinePos != std::string::npos)
		{
			shaderCode.insert(nextLinePos + 1, defines + "\n");
		}
		else
		{
			shaderCode = "#version 330 core\n" + defines + "\n" + shaderCode;
		}

		const char* finalShaderSource = shaderCode.c_str();

		(*shader) = glCreateShader(
			type == ShaderType::VertexShader ? GL_VERTEX_SHADER :
			type == ShaderType::FragmentShader ? GL_FRAGMENT_SHADER :
			type == ShaderType::ComputeShaderType ? GL_COMPUTE_SHADER : GL_GEOMETRY_SHADER);

		glShaderSource((*shader), 1, &finalShaderSource, nullptr);
		glCompileShader((*shader));

		return ValidateShader((*shader));
	}


}