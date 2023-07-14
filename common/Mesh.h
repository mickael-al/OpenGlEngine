#pragma once

#include "Vertex.h"
#include "Material.h"
#include "GObject.hpp"
#include <vector>

struct SubMesh
{
	uint32_t VAO;	// notez qu'il faut créer un VAO par SubMesh (VBO) quand bien meme on utilise le meme shader
	uint32_t VBO;	// ceci parceque l'identifiant du VBO est logiquement different a chaque fois
	uint32_t IBO;
	uint32_t verticesCount;
	uint32_t indicesCount;
	int32_t materialId;
};

class Mesh : public GObject
{	
public:
	Mesh();
	static bool ParseObj(Mesh* obj, const char* filepath);
	void Setup(uint32_t program);
	void SetupOnlyVertex(uint32_t program);
	void Destroy();
	void mapMemory();
	std::vector<SubMesh> meshes;
	std::vector<Material> materials;	
};


