#ifndef __ENGINE_VERTEX__
#define __ENGINE_VERTEX__

#include <glm/glm.hpp>
#include <array>
#include <functional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

using namespace std;

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 tangents;
	glm::vec3 color;
	glm::vec2 texCoord;	

	bool operator==(const Vertex &other) const
	{
		return pos == other.pos && normal == other.normal && color == other.color && texCoord == other.texCoord;
	}

	static void hashCombine(std::size_t& hash, std::size_t v)
	{
		hash ^= v + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	}
};
namespace std
{

	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			auto hash = std::size_t{ 0 };
			Vertex::hashCombine(hash, std::hash<glm::vec3>()(vertex.pos));
			Vertex::hashCombine(hash, std::hash<glm::vec3>()(vertex.normal));
			//Vertex::hashCombine(hash, std::hash<glm::vec3>()(vertex.color));
			Vertex::hashCombine(hash, std::hash<glm::vec2>()(vertex.texCoord));
			//Vertex::hashCombine(hash, std::hash<glm::vec3>()(vertex.tangents));
			return hash;
		}
	};
}

#endif //!__ENGINE_VERTEX__