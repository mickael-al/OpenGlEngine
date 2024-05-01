#include "glcore.hpp"
#include "ShapeBufferBase.hpp"
#include <memory.h>
#include "Debug.hpp"

namespace Ge
{
	ShapeBufferBase::ShapeBufferBase(std::vector<Vertex> & vertices, std::vector<uint32_t> & indices, GraphicsDataMisc * gdm)
	{
		m_vertices = vertices;
		m_indices = indices;	

		glGenBuffers(1, &m_IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t)*indices.size(), indices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
	}

	ShapeBufferBase::~ShapeBufferBase()
	{
		m_vertices.clear();
		m_indices.clear();
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_IBO);		
	}

	void ShapeBufferBase::SetupVAO(unsigned int program)
	{
		uint32_t POSITION = glGetAttribLocation(program, "a_Position");
		uint32_t NORMAL = glGetAttribLocation(program, "a_Normal");
		uint32_t TANGENTS = glGetAttribLocation(program, "a_Tangents");
		uint32_t COLOR = glGetAttribLocation(program, "a_Color");
		uint32_t TEXCOORDS = glGetAttribLocation(program, "a_TexCoords");

		if (glIsVertexArray(m_VAO) == GL_TRUE)
		{
			glDeleteVertexArrays(1, &m_VAO);
			m_VAO = 0;
		}

		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glEnableVertexAttribArray(POSITION);
		glVertexAttribPointer(POSITION, 3, GL_FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, pos));
		glEnableVertexAttribArray(NORMAL);
		glVertexAttribPointer(NORMAL, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(TANGENTS);
		glVertexAttribPointer(TANGENTS, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, tangents));
		glEnableVertexAttribArray(COLOR);
		glVertexAttribPointer(COLOR, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(TEXCOORDS);
		glVertexAttribPointer(TEXCOORDS, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	}

	unsigned int ShapeBufferBase::getIndiceSize() const
	{
		return m_indices.size();
	}

	const std::vector<uint32_t> & ShapeBufferBase::getIndices() const
	{
		return m_indices;
	}

	const std::vector<Vertex> & ShapeBufferBase::getVertices() const
	{
		return m_vertices;
	}
}