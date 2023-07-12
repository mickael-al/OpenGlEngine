
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "../libs/tinyobjloader/tiny_obj_loader.h"

#include "OpenGLcore.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"

// materiau par defaut (couleur ambiante, couleur diffuse, couleur speculaire, shininess, tex ambient, tex diffuse, tex specular)
Material Material::defaultMaterial = { { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f }, { 0.f, 0.f, 0.f }, 256.f, 0, 1, 0 };

void Mesh::Destroy()
{
	// On n'oublie pas de d�truire les objets OpenGL
	for (auto & submesh : meshes)
	{
		// Comme les VBO ont ete "detruit" a l'initialisation
		// seul le VAO contient une reference vers ces VBO/IBO
		// detruire le VAO entraine donc la veritable destruction/deallocation des VBO/IBO
		//DeleteBufferObject(submesh.VBO);
		//DeleteBufferObject(submesh.IBO);
		glDeleteVertexArrays(1, &submesh.VAO);
		submesh.VAO = 0;
	}

	meshes.clear();
	// on supprime l'espace memoire alloue
	meshes.shrink_to_fit();
}

void Mesh::mapMemory()
{

}

void Mesh::Setup(uint32_t program)
{
	uint32_t POSITION = glGetAttribLocation(program, "a_Position");
	uint32_t NORMAL = glGetAttribLocation(program, "a_Normal");
	uint32_t TEXCOORDS = glGetAttribLocation(program, "a_TexCoords");

	for (auto& submesh : meshes)
	{
		glGenVertexArrays(1, &submesh.VAO);
		glBindVertexArray(submesh.VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, submesh.IBO);
		glBindBuffer(GL_ARRAY_BUFFER, submesh.VBO);
		glEnableVertexAttribArray(POSITION);
		glVertexAttribPointer(POSITION, 3, GL_FLOAT, false, sizeof(Vertex), (void *)offsetof(Vertex, position));
		glEnableVertexAttribArray(NORMAL);
		glVertexAttribPointer(NORMAL, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(TEXCOORDS);
		glVertexAttribPointer(TEXCOORDS, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, texcoords));
	}
}

bool Mesh::ParseObj(Mesh* obj, const char* filepath)
{
	std::string warning, error;

	memset(obj, 0, sizeof(Mesh));

	std::map<std::string, int> material_map;
	std::vector<tinyobj::material_t> materials;
	std::string mtlPath = filepath;

	{
		size_t off = mtlPath.rfind("/");
		mtlPath.resize(off);
		std::vector<tinyobj::shape_t> shapes;
		tinyobj::attrib_t attrib;

		bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, filepath, mtlPath.c_str());
		if (warning.length())
			std::cout << "[warning]: " << warning << std::endl;
		if (error.length())
			std::cout << "[error]: " << error << std::endl;

		obj->materials.resize(materials.size());
		// voir le warning plus bas 
		memset(obj->materials.data(), 0, sizeof(Material) * materials.size());

		uint32_t materialCount = 0;
		for (tinyobj::material_t& material : materials)
		{
			Material& mat = obj->materials[materialCount];
			memcpy(&mat.ambientColor, material.ambient, sizeof(vec3));
			memcpy(&mat.diffuseColor, material.diffuse, sizeof(vec3));
			memcpy(&mat.specularColor, material.specular, sizeof(vec3));
			mat.shininess = material.shininess;
			mat.diffuseTexture = Texture::LoadTexture((mtlPath + "/" + material.diffuse_texname).c_str());
			++materialCount;
		}

		// On va g�rer plusieurs objets / groupes OBJ - ce que tinyobj appelle des shapes
		// chaque shape est un mesh, plus precisement ici un submesh
		// le format OBJ ne d�fini pas de hierarchie claire, il n'est pas toujours evident de savoir
		// si une "shape" est un objet � part ou une sous partie d'un autre...j'ai fait le choix de la sous partie
		obj->meshes.resize(shapes.size());
		// note: attention � ne pas utiliser memset avec des classes polymorphiques (virtual) 
		// vous risquez d'�craser les pointeurs vers la table virtuelle (vtable)
		memset(obj->meshes.data(), 0, sizeof(SubMesh) * shapes.size());

		uint32_t meshCount = 0;
		for (tinyobj::shape_t& shape : shapes)
		{
			SubMesh& submesh = obj->meshes[meshCount];
			++meshCount;

			// buffer temporaire, on va tout stocker c�t� GPU
			Vertex* vertices = new Vertex[shape.mesh.indices.size()];
			submesh.verticesCount = 0;
			uint32_t* indices = new uint32_t[shape.mesh.indices.size()];
			submesh.indicesCount = 0;

			int materialId = -1;
			int faceId = 0;
			for (tinyobj::index_t& index : shape.mesh.indices)
			{
				Vertex v;

				// tinyobj ne stocke pas l'identifiant du materiau globalement dans la shape
				// mais dans les faces..
				int currentMaterialId = shape.mesh.material_ids[faceId / 3];
				// techniquement si des faces d'une m�me shape ont des
				// materiaux differents il faudrait cr�er un SubMesh � chaque mat�riaux dans une shape.
				// ici, je me contente de selectionner le dernier material_id pour le SubMesh actuel
				if (currentMaterialId != materialId)
					materialId = currentMaterialId;

				v.position.x = attrib.vertices[3 * index.vertex_index + 0];
				v.position.y = attrib.vertices[3 * index.vertex_index + 1];
				v.position.z = attrib.vertices[3 * index.vertex_index + 2];
				
				if (index.normal_index > -1) {
					v.normal.x = attrib.normals[3 * index.normal_index + 0];
					v.normal.y = attrib.normals[3 * index.normal_index + 1];
					v.normal.z = attrib.normals[3 * index.normal_index + 2];
				}
				else
				{
					// todo : g�n�rer des normales
					bool useSmoothingGroup = false;
					if (shape.mesh.smoothing_group_ids.size() != 0)
						useSmoothingGroup = true;
				}

				v.texcoords = { 0.f, 0.f };
				if (index.texcoord_index > -1) {
					v.texcoords.x = attrib.texcoords[2 * index.texcoord_index + 0];
					v.texcoords.y = attrib.texcoords[2 * index.texcoord_index + 1];
					// Important � savoir
					// contrairement � OpenGL, les textures dans les logiciels 2D et 3D
					// ont pour origine le coin haut-gauche de l'�cran
					// Il est donc souvent n�cessaire de convertir la cordonn�es v (y)
					// en C++ ou dans le shader, par exemple ici 
					v.texcoords.y = 1.f - v.texcoords.y;
				}

				// tinyobj loader affecte du blanc par defaut lorsqu'il ne trouve pas de couleur
				// c'est g�n�ralement le cas car les couleurs sont une extension non standard du format OBJ
				// ce qui rend cet attribut purement optionnel
				// notez que les couleurs sont volontairement converties en RGBA8 pour gagner de la place en memoire
				//v.color[0] = uint8_t(attrib.colors[3 * index.vertex_index + 0] * 255.99f);
				//v.color[1] = uint8_t(attrib.colors[3 * index.vertex_index + 1] * 255.99f);
				//v.color[2] = uint8_t(attrib.colors[3 * index.vertex_index + 2] * 255.99f);
				//v.color[3] = 255;

				uint32_t vertexIndex = 0;
				// recherche lineaire (lente) afin de tester si le vertex existe deja
				for (; vertexIndex < submesh.verticesCount; ++vertexIndex)
				{
					const Vertex &vi = vertices[vertexIndex];
					if (v.IsSame(vi)) {
						break;
					}
				}

				if (vertexIndex == submesh.verticesCount)
				{
					vertices[vertexIndex] = v;
					++submesh.verticesCount;
				}
				indices[submesh.indicesCount] = vertexIndex;
				++submesh.indicesCount;

				faceId++;
			}

			submesh.materialId = materialId;

			// notez que je ne cree pas le VAO ici
			// Un VAO fait le lien entre le VBO (+ EBO/IBO) et les attributs d'un vertex shader
			submesh.VBO = CreateBufferObject(BufferType::VBO, sizeof(Vertex) * submesh.verticesCount, vertices);
			submesh.IBO = CreateBufferObject(BufferType::IBO, sizeof(uint32_t) * submesh.indicesCount, indices);

			// important, bien lib�rer la m�moire des buffers temporaires
			delete[] indices;
			delete[] vertices;
		}
	}

	return true;
}
