#ifndef __ENGINE_MODEL_BUFFER__
#define __ENGINE_MODEL_BUFFER__

#include <vector>
struct GraphicsDataMisc;
#include "Vertex.hpp"
#include "ShapeBuffer.hpp"

namespace Ge
{
	class ShapeBufferBase : public ShapeBuffer
	{
	public:
		ShapeBufferBase(std::vector<Vertex> & vertices, std::vector<uint32_t> & indices, GraphicsDataMisc * gdm);
		~ShapeBufferBase();		
		void SetupVAO(unsigned int program);
		unsigned int getIndiceSize() const;
		const std::vector<Vertex> & getVertices() const;
		const std::vector<uint32_t> & getIndices() const;
	private:
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;
	};
}

#endif //!__ENGINE_MODEL_BUFFER__
