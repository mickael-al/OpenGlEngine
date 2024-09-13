#include "glcore.hpp"
#include "ModelManager.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"
#include <glm/gtx/normal.hpp>
#include "OpenFBX/src/ofbx.h"
#include "Model.hpp"
#include "ShapeBufferBase.hpp"
#include "Debug.hpp"
#include "Vertex.hpp"
#include <algorithm>
#include <unordered_map>
#include "GraphicsDataMisc.hpp"
#include "GraphiquePipeline.hpp"

namespace Ge
{
	bool ModelManager::initialize(GraphicsDataMisc *gdm)
	{
		m_gdm = gdm;
		glGenBuffers(1, &m_ssbo);
		m_gdm->str_ssbo.str_model = m_ssbo;
		float pos[] = { -1.0f, 1.0f,0.0f, -1.0f, -3.0f,0.0f , 3.0f, 1.0f,0.0f };
		float texCord[] = {0.0f,0.0f,0.0f,-2.0f,2.0f,0.0f};
		float normal[] = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f, 0.0f,0.0f,1.0f };
		unsigned int indice[] = { 0,1,2};

		m_fullScreenTriangle = allocateBuffer(pos, texCord, normal, indice, 9, 3);
		m_fullScreenTriangle->SetupVAO(gdm->str_default_pipeline_forward->getProgram());

		float posQ[] = { -1.0f, 1.0f,0.0f, -1.0f, -1.0f,0.0f , 1.0f, 1.0f,0.0f, 1.0f, -1.0f,0.0f };
		float texCordQ[] = { 0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f,0.0f };
		float normalQ[] = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f, 0.0f,0.0f,1.0f, 0.0f,0.0f,1.0f };
		unsigned int indiceQ[] = { 0,1,2,3,2,1 };

		m_defferedQuad = allocateBuffer(posQ, texCordQ, normalQ, indiceQ, 12, 6);
		m_defferedQuad->SetupVAO(gdm->str_default_pipeline_forward->getProgram());
		Debug::INITSUCCESS("ModelManager");
		return true;
	}

	void ModelManager::release()
	{
		for (int i = 0; i < m_shapeBuffers.size(); i++)
		{			
			m_poolBuffer.deleteObject((ShapeBufferBase*)m_shapeBuffers[i]);
		}
		m_shapeBuffers.clear();
		for (int i = 0; i < m_models.size(); i++)
		{			
			m_pool.deleteObject(m_models[i]);
		}
		m_models.clear();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glDeleteBuffers(1, &m_ssbo);
		Debug::RELEASESUCCESS("ModelManager");
	}

	Model * ModelManager::createModel(ShapeBuffer *buffer, std::string nom)
	{
		if (buffer == nullptr)
		{
			Debug::Warn("Le buffer n'existe pas");
			return nullptr;
		}
		Model * Mesh = m_pool.newObject(buffer, m_models.size(), m_gdm);
		m_models.push_back(Mesh);
		m_gdm->str_dataMisc.modelCount = m_models.size();
		updateStorage();
		Mesh->setName(nom);
		Mesh->setMaterial(m_gdm->str_default_material);		
		m_gdm->str_dataMisc.recreateCommandBuffer = true;
		return Mesh;
	}

	void ModelManager::destroyModel(Model *model)
	{
		model->setParent(nullptr);
		m_models.erase(std::remove(m_models.begin(), m_models.end(), model), m_models.end());
		Materials * mat = model->getMaterial();
		ShapeBuffer * sb = model->getShapeBuffer();
		std::vector<Model*> &vecModel = m_instanced[mat][sb];
		vecModel.erase(std::remove(vecModel.begin(), vecModel.end(), model), vecModel.end());
		if (vecModel.size() == 0)
		{
			auto bufferIterator = std::find_if(
				m_instanced[mat].begin(), m_instanced[mat].end(),
				[sb](const std::pair<ShapeBuffer * , std::vector<Model*>>& shapeBufferPair) {
				return shapeBufferPair.first == sb;
			}
			);

			if (bufferIterator != m_instanced[mat].end())
			{
				m_instanced[mat].erase(bufferIterator);
				if (m_instanced[mat].empty())
				{
					m_instanced.erase(mat);
				}
			}
		}		
		m_pool.deleteObject(model);
		m_gdm->str_dataMisc.modelCount = m_models.size();
		m_gdm->str_dataMisc.recreateCommandBuffer = true;
		updateStorage();
	}

	void ModelManager::destroyBuffer(ShapeBuffer *buffer)
	{
		m_shapeBuffers.erase(std::remove(m_shapeBuffers.begin(), m_shapeBuffers.end(), buffer), m_shapeBuffers.end());
		for (auto& materialPair : m_instanced) 
		{
			Materials* currentMaterials = materialPair.first;

			auto bufferIterator = std::find_if(
				materialPair.second.begin(), materialPair.second.end(),
				[buffer](const std::pair<ShapeBuffer *, std::vector<Model*>>& shapeBufferPair) {
				return shapeBufferPair.first == buffer;
			}
			);

			if (bufferIterator != materialPair.second.end()) 
			{
				materialPair.second.erase(bufferIterator);			
				if (materialPair.second.empty())
				{
					m_instanced.erase(materialPair.first);
					break;
				}
			}
		}		
		m_poolBuffer.deleteObject((ShapeBufferBase*)buffer);
		m_gdm->str_dataMisc.recreateCommandBuffer = true;	
	}

	std::vector<Model*> & ModelManager::getModels()
	{
		return m_models;
	}

	std::unordered_map<Materials*, std::unordered_map<ShapeBuffer*, std::vector<Model*>>> & ModelManager::getInstancedModels()
	{
		return m_instanced;
	}

	void ModelManager::buildInstancedModels(Model * target,Materials * switchMat)
	{
		Materials * mat = target->getMaterial();
		ShapeBuffer * sb = target->getShapeBuffer();
		std::vector<Model*> &vecModel = m_instanced[mat][sb];
		vecModel.erase(std::remove(vecModel.begin(), vecModel.end(), target), vecModel.end());
		m_instanced[switchMat][sb].push_back(target);
		int count = 0;
		for (auto& material : m_instanced)
		{
			for (auto& buffer : material.second)
			{
				for (auto& model : buffer.second)
				{
					if (model->getIndex() != count)
					{
						model->setIndex(count);
					}
					count++;
				}
			}
		}
	}

	void ModelManager::clearInstancedMaterial(Materials * mat)
	{
		std::unordered_map<Materials*, std::unordered_map<ShapeBuffer*, std::vector<Model*>>>::iterator materialIterator = std::find_if(
			m_instanced.begin(), m_instanced.end(), 
			[mat](const std::pair<Materials*, std::unordered_map<ShapeBuffer*, std::vector<Model*>>>& pair) { return pair.first == mat; }
		);
		if (materialIterator != m_instanced.end()) 
		{
			m_instanced.erase(materialIterator);
		}
	}

	void ModelManager::printModelInfo(const char *path)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		glm::vec3 normalResult;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path))
		{
			Debug::Warn("%s  %s", nullptr, warn.c_str(), err.c_str());
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto &shape : shapes)
		{
			Debug::Log("%s", path);
			for (const auto &index : shape.mesh.indices)
			{
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2] };

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2] };

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
		Debug::Info("POS: %d", vertices.size());
		for (int i = 0; i < vertices.size(); i++)
		{
			std::cout << vertices[i].pos.x << "," << vertices[i].pos.y << "," << vertices[i].pos.z << ",";
		}
		std::cout << std::endl;
		Debug::Info("TEXCORD: %d", vertices.size());
		for (int i = 0; i < vertices.size(); i++)
		{
			std::cout << vertices[i].texCoord.x << "," << vertices[i].texCoord.y << ",";
		}
		std::cout << std::endl;
		Debug::Info("NORMAL: %d", vertices.size());
		for (int i = 0; i < vertices.size(); i++)
		{
			std::cout << vertices[i].normal.x << "," << vertices[i].normal.y << "," << vertices[i].normal.z << ",";
		}
		std::cout << std::endl;
		Debug::Info("INDICE: %d", indices.size());
		for (int i = 0; i < indices.size(); i++)
		{
			std::cout << indices[i] << ",";
		}
		std::cout << std::endl;
	}

	ShapeBuffer * ModelManager::allocateBuffer(float * pos, float * texCord, float * normal, unsigned int * indice, unsigned vertexSize, unsigned indiceSize)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices(indice, indice + indiceSize);

		vertices.reserve(vertexSize / 3);
		for (int i = 0; i < vertexSize; i++)
		{
			Vertex vertex{};

			vertex.pos = {
				pos[3 * i + 0],
				pos[3 * i + 1],
				pos[3 * i + 2] };

			vertex.texCoord = {
				texCord[2 * i + 0],
				texCord[2 * i + 1] };

			vertex.normal = {
				normal[3 * i + 0],
				normal[3 * i + 1],
				normal[3 * i + 2] };
			vertices.push_back(vertex);
		}

		uint32_t index0, index1, index2;
		glm::vec3 edge1, edge2, tangent;
		glm::vec2 deltaUV1, deltaUV2;
		float r;
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			index0 = indices[i];
			index1 = indices[i + 1];
			index2 = indices[i + 2];

			Vertex& vertex0 = vertices[index0];
			Vertex& vertex1 = vertices[index1];
			Vertex& vertex2 = vertices[index2];

			edge1 = vertex1.pos - vertex0.pos;
			edge2 = vertex2.pos - vertex0.pos;

			deltaUV1 = vertex1.texCoord - vertex0.texCoord;
			deltaUV2 = vertex2.texCoord - vertex0.texCoord;

			r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			tangent = glm::normalize(r * (deltaUV2.y * edge1 - deltaUV1.y * edge2));
	
			vertex0.tangents = tangent;
			vertex1.tangents = tangent;
			vertex2.tangents = tangent;
		}				
		ShapeBuffer* buffer = (ShapeBuffer*)m_poolBuffer.newObject(vertices, indices, m_gdm);
		m_shapeBuffers.push_back(buffer);
		return buffer;
	}

	ShapeBuffer* ModelManager::getDefferedQuad() const
	{
		return m_defferedQuad;
	}

	ShapeBuffer* ModelManager::getFullScreenTriangle() const
	{
		return m_fullScreenTriangle;
	}

	void ModelManager::ComputationTangent(std::vector<Vertex> &vertices)
	{
		glm::vec3 edge1;
		glm::vec3 edge2;
		glm::vec2 deltaUV1;
		glm::vec2 deltaUV2;
		glm::vec3 tangents;

		float r;
		for (int i = 0; i + 2 < vertices.size(); i += 3)
		{
			edge1 = vertices[i + 1].pos - vertices[i].pos;
			edge2 = vertices[i + 2].pos - vertices[i].pos;

			deltaUV1 = vertices[i + 1].texCoord - vertices[i].texCoord;
			deltaUV2 = vertices[i + 2].texCoord - vertices[i].texCoord;

			r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			tangents.x = r * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangents.y = r * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangents.z = r * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			tangents = glm::normalize(tangents);

			vertices[i + 0].tangents = tangents;
			vertices[i + 1].tangents = tangents;
			vertices[i + 2].tangents = tangents;
		}
	}

	std::vector<ShapeBuffer*> ModelManager::allocateBuffers(const char* path, bool normal_recalculate)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::vector<ShapeBuffer*> sbvec;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path))
		{
			Debug::Warn("%s  %s", warn.c_str(), err.c_str());
			return sbvec;
		}

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		vertices.reserve(attrib.vertices.size() / 3);
		indices.reserve(attrib.vertices.size());

		for (const auto& shape : shapes)
		{
			vertices.clear();
			indices.clear();
			uniqueVertices.clear();
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				vertex.color = { 1, 1, 1 };
				vertex.tangents = { 0, 0, 0 };

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.emplace_back(vertex);
				}

				indices.emplace_back(uniqueVertices[vertex]);
			}

			uint32_t index0, index1, index2;
			glm::vec3 edge1, edge2, tangent;
			glm::vec2 deltaUV1, deltaUV2;
			float r;
			for (size_t i = 0; i < indices.size(); i += 3)
			{
				index0 = indices[i];
				index1 = indices[i + 1];
				index2 = indices[i + 2];

				Vertex& vertex0 = vertices[index0];
				Vertex& vertex1 = vertices[index1];
				Vertex& vertex2 = vertices[index2];

				edge1 = vertex1.pos - vertex0.pos;
				edge2 = vertex2.pos - vertex0.pos;

				deltaUV1 = vertex1.texCoord - vertex0.texCoord;
				deltaUV2 = vertex2.texCoord - vertex0.texCoord;

				r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

				tangent = glm::normalize(r * (deltaUV2.y * edge1 - deltaUV1.y * edge2));

				vertex0.tangents = tangent;
				vertex1.tangents = tangent;
				vertex2.tangents = tangent;
				if (normal_recalculate)
				{
					glm::vec3 normal = glm::normalize(glm::cross(vertex1.pos - vertex0.pos, vertex2.pos - vertex0.pos));
					vertex0.normal = normal;
					vertex1.normal = normal;
					vertex2.normal = normal;
				}
			}
			ShapeBuffer* buffer = (ShapeBuffer*)m_poolBuffer.newObject(vertices, indices, m_gdm);
			m_shapeBuffers.push_back(buffer);
			sbvec.push_back(buffer);
		}

		return sbvec;
	}

	void ModelManager::updateStorage()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(UniformBufferObject)*m_models.size(), nullptr, GL_DYNAMIC_DRAW);
		int count = 0;
		for (auto& material : m_instanced)
		{
			for (auto& buffer : material.second)
			{
				for (auto& model : buffer.second)
				{
					model->setIndex(count++);
				}
			}
		}
	}

	std::vector<ShapeBuffer*> ModelManager::allocateFBXBufferNoOptimize(const char* path, bool normal_recalculate, std::vector<int> m_loadIdMesh)
	{
		std::vector<ShapeBuffer*> sbvec;
		FILE* fp = fopen(path, "rb");

		if (!fp)
		{
			Debug::Warn("cannot load : %s ", path);
			return sbvec;
		}
		fseek(fp, 0, SEEK_END);
		long file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		auto* content = new ofbx::u8[file_size];
		fread(content, 1, file_size, fp);

		ofbx::LoadFlags flags =
			ofbx::LoadFlags::TRIANGULATE |
			//		ofbx::LoadFlags::IGNORE_MODELS |
			ofbx::LoadFlags::IGNORE_BLEND_SHAPES |
			ofbx::LoadFlags::IGNORE_CAMERAS |
			ofbx::LoadFlags::IGNORE_LIGHTS |
			//		ofbx::LoadFlags::IGNORE_TEXTURES |
			ofbx::LoadFlags::IGNORE_SKIN |
			ofbx::LoadFlags::IGNORE_BONES |
			ofbx::LoadFlags::IGNORE_PIVOTS |
			//		ofbx::LoadFlags::IGNORE_MATERIALS |
			ofbx::LoadFlags::IGNORE_POSES |
			ofbx::LoadFlags::IGNORE_VIDEOS |
			ofbx::LoadFlags::IGNORE_LIMBS |
			//		ofbx::LoadFlags::IGNORE_MESHES |
			ofbx::LoadFlags::IGNORE_ANIMATIONS;

		ofbx::IScene* scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u16)flags);

		int size = (m_loadIdMesh.size() > 0) ? m_loadIdMesh.size() : scene->getMeshCount();

		for (int i = 0; i < size; ++i)
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			const ofbx::Geometry* geom = nullptr;
			if (m_loadIdMesh.size() > 0)
			{
				geom = scene->getMesh(m_loadIdMesh[i] % scene->getMeshCount())->getGeometry();
			}
			else
			{
				geom = scene->getMesh(i)->getGeometry();
			}

			vertices.reserve(geom->getVertexCount());
			indices.reserve(geom->getIndexCount());
			const ofbx::Vec3* position = geom->getVertices();
			const ofbx::Vec2* uv = geom->getUVs();
			const ofbx::Vec3* normals = geom->getNormals();
			const ofbx::Vec3* tangents = geom->getTangents();
			const ofbx::Vec4* colors = geom->getColors();

			for (int j = 0; j < geom->getVertexCount(); ++j)
			{
				Vertex vertex{};
				vertex.pos.x = position[j].x;
				vertex.pos.y = position[j].y;
				vertex.pos.z = position[j].z;

				vertex.texCoord.x = uv[j].x;
				vertex.texCoord.y = 1.0f - uv[j].y;

				if (normals != nullptr && !normal_recalculate)
				{
					vertex.normal.x = normals[j].x;
					vertex.normal.y = normals[j].y;
					vertex.normal.z = normals[j].z;
				}

				if (tangents != nullptr)
				{
					vertex.tangents.x = tangents[j].x;
					vertex.tangents.y = tangents[j].y;
					vertex.tangents.z = tangents[j].z;
				}
				else
				{
					vertex.tangents = { 0, 0, 0 };
				}

				if (colors != nullptr)
				{
					vertex.color.x = colors[j].x;
					vertex.color.y = colors[j].y;
					vertex.color.z = colors[j].z;
				}
				else
				{
					vertex.color = { 1, 1, 1 };
				}

				vertices.push_back(vertex);
				indices.push_back(static_cast<uint32_t>(vertices.size()-1));
			}

			if (tangents == nullptr || normal_recalculate || normals == nullptr)
			{
				uint32_t index0, index1, index2;
				glm::vec3 edge1, edge2, tangent;
				glm::vec2 deltaUV1, deltaUV2;
				float r;
				for (size_t i = 0; i < indices.size(); i += 3)
				{
					index0 = indices[i];
					index1 = indices[i + 1];
					index2 = indices[i + 2];

					Vertex& vertex0 = vertices[index0];
					Vertex& vertex1 = vertices[index1];
					Vertex& vertex2 = vertices[index2];

					if (tangents == nullptr)
					{
						edge1 = vertex1.pos - vertex0.pos;
						edge2 = vertex2.pos - vertex0.pos;

						deltaUV1 = vertex1.texCoord - vertex0.texCoord;
						deltaUV2 = vertex2.texCoord - vertex0.texCoord;

						r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

						tangent = glm::normalize(r * (deltaUV2.y * edge1 - deltaUV1.y * edge2));

						vertex0.tangents = tangent;
						vertex1.tangents = tangent;
						vertex2.tangents = tangent;
					}
					if (normal_recalculate || normals == nullptr)
					{
						glm::vec3 normal = glm::normalize(glm::cross(vertex1.pos - vertex0.pos, vertex2.pos - vertex0.pos));
						vertex0.normal = normal;
						vertex1.normal = normal;
						vertex2.normal = normal;
					}
				}
			}
			ShapeBuffer* buffer = (ShapeBuffer*)m_poolBuffer.newObject(vertices, indices, m_gdm);
			m_shapeBuffers.push_back(buffer);
			sbvec.push_back(buffer);
		}

		delete[] content;
		fclose(fp);

		return sbvec;
	}

	std::vector<ShapeBuffer*> ModelManager::allocateFBXBuffer(const char* path, bool normal_recalculate, std::vector<int> m_loadIdMesh)
	{
		std::vector<ShapeBuffer*> sbvec;
		FILE* fp = fopen(path, "rb");

		if (!fp)
		{
			Debug::Warn("cannot load : %s ", path);
			return sbvec;
		}
		fseek(fp, 0, SEEK_END);
		long file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		auto* content = new ofbx::u8[file_size];
		fread(content, 1, file_size, fp);

		ofbx::LoadFlags flags =
			ofbx::LoadFlags::TRIANGULATE |
			//		ofbx::LoadFlags::IGNORE_MODELS |
			ofbx::LoadFlags::IGNORE_BLEND_SHAPES |
			ofbx::LoadFlags::IGNORE_CAMERAS |
			ofbx::LoadFlags::IGNORE_LIGHTS |
			//		ofbx::LoadFlags::IGNORE_TEXTURES |
			ofbx::LoadFlags::IGNORE_SKIN |
			ofbx::LoadFlags::IGNORE_BONES |
			ofbx::LoadFlags::IGNORE_PIVOTS |
			//		ofbx::LoadFlags::IGNORE_MATERIALS |
			ofbx::LoadFlags::IGNORE_POSES |
			ofbx::LoadFlags::IGNORE_VIDEOS |
			ofbx::LoadFlags::IGNORE_LIMBS |
			//		ofbx::LoadFlags::IGNORE_MESHES |
			ofbx::LoadFlags::IGNORE_ANIMATIONS;

		ofbx::IScene* scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u16)flags);

		int size = (m_loadIdMesh.size() > 0) ? m_loadIdMesh.size() : scene->getMeshCount();

		for (int i = 0; i < size; ++i)
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::unordered_map<Vertex, uint32_t> uniqueVertices;

			const ofbx::Geometry* geom = nullptr;
			if (m_loadIdMesh.size() > 0)
			{
				geom = scene->getMesh(m_loadIdMesh[i] % scene->getMeshCount())->getGeometry();
			}
			else
			{
				geom = scene->getMesh(i)->getGeometry();
			}

			vertices.reserve(geom->getVertexCount());
			indices.reserve(geom->getIndexCount());
			const ofbx::Vec3* position = geom->getVertices();
			const ofbx::Vec2* uv = geom->getUVs();
			const ofbx::Vec3* normals = geom->getNormals();
			const ofbx::Vec3* tangents = geom->getTangents();
			const ofbx::Vec4* colors = geom->getColors();

			for (int j = 0; j < geom->getVertexCount(); ++j)
			{
				Vertex vertex{};
				vertex.pos.x = position[j].x;
				vertex.pos.y = position[j].y;
				vertex.pos.z = position[j].z;

				vertex.texCoord.x = uv[j].x;
				vertex.texCoord.y = 1.0f - uv[j].y;

				if (normals != nullptr && !normal_recalculate)
				{
					vertex.normal.x = normals[j].x;
					vertex.normal.y = normals[j].y;
					vertex.normal.z = normals[j].z;
				}

				if (tangents != nullptr)
				{
					vertex.tangents.x = tangents[j].x;
					vertex.tangents.y = tangents[j].y;
					vertex.tangents.z = tangents[j].z;
				}
				else
				{
					vertex.tangents = { 0, 0, 0 };
				}

				if (colors != nullptr)
				{
					vertex.color.x = colors[j].x;
					vertex.color.y = colors[j].y;
					vertex.color.z = colors[j].z;
				}
				else
				{
					vertex.color = { 1, 1, 1 };
				}

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.emplace_back(vertex);
				}

				indices.emplace_back(uniqueVertices[vertex]);
			}

			if (tangents == nullptr || normal_recalculate || normals == nullptr)
			{
				uint32_t index0, index1, index2;
				glm::vec3 edge1, edge2, tangent;
				glm::vec2 deltaUV1, deltaUV2;
				float r;
				for (size_t i = 0; i < indices.size(); i += 3)
				{
					index0 = indices[i];
					index1 = indices[i + 1];
					index2 = indices[i + 2];

					Vertex& vertex0 = vertices[index0];
					Vertex& vertex1 = vertices[index1];
					Vertex& vertex2 = vertices[index2];

					if (tangents == nullptr)
					{
						edge1 = vertex1.pos - vertex0.pos;
						edge2 = vertex2.pos - vertex0.pos;

						deltaUV1 = vertex1.texCoord - vertex0.texCoord;
						deltaUV2 = vertex2.texCoord - vertex0.texCoord;

						r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

						tangent = glm::normalize(r * (deltaUV2.y * edge1 - deltaUV1.y * edge2));

						vertex0.tangents = tangent;
						vertex1.tangents = tangent;
						vertex2.tangents = tangent;
					}
					if (normal_recalculate || normals == nullptr)
					{
						glm::vec3 normal = glm::normalize(glm::cross(vertex1.pos - vertex0.pos, vertex2.pos - vertex0.pos));
						vertex0.normal = normal;
						vertex1.normal = normal;
						vertex2.normal = normal;
					}
				}
			}
			ShapeBuffer* buffer = (ShapeBuffer*)m_poolBuffer.newObject(vertices, indices, m_gdm);
			m_shapeBuffers.push_back(buffer);
			sbvec.push_back(buffer);
		}

		delete[] content;
		fclose(fp);

		return sbvec;
	}

	ShapeBuffer* ModelManager::allocateBuffer(const char* path, bool normal_recalculate)
	{		
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path))
		{
			Debug::Warn("%s  %s", warn.c_str(), err.c_str());
			return nullptr;
		}

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		vertices.reserve(attrib.vertices.size() / 3);
		indices.reserve(attrib.vertices.size());

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				vertex.color = { 1, 1, 1 };
				vertex.tangents = { 0, 0, 0 };

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.emplace_back(vertex);
				}

				indices.emplace_back(uniqueVertices[vertex]);
			}
		}

		uint32_t index0, index1, index2;
		glm::vec3 edge1, edge2, tangent;
		glm::vec2 deltaUV1, deltaUV2;
		float r;
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			index0 = indices[i];
			index1 = indices[i + 1];
			index2 = indices[i + 2];

			Vertex& vertex0 = vertices[index0];
			Vertex& vertex1 = vertices[index1];
			Vertex& vertex2 = vertices[index2];

			edge1 = vertex1.pos - vertex0.pos;
			edge2 = vertex2.pos - vertex0.pos;

			deltaUV1 = vertex1.texCoord - vertex0.texCoord;
			deltaUV2 = vertex2.texCoord - vertex0.texCoord;

			r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			tangent = glm::normalize(r * (deltaUV2.y * edge1 - deltaUV1.y * edge2));

			vertex0.tangents = tangent;
			vertex1.tangents = tangent;
			vertex2.tangents = tangent;
			if (normal_recalculate)
			{
				glm::vec3 normal = glm::normalize(glm::cross(vertex1.pos - vertex0.pos, vertex2.pos - vertex0.pos));
				vertex0.normal = normal;
				vertex1.normal = normal;
				vertex2.normal = normal;
			}
		}
		ShapeBuffer* buffer = (ShapeBuffer*)m_poolBuffer.newObject(vertices, indices, m_gdm);
		m_shapeBuffers.push_back(buffer);
		return buffer;
	}
}