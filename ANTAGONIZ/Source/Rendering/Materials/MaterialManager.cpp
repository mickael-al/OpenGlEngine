#include "glcore.hpp"
#include "MaterialManager.hpp"
#include "ModelManager.hpp"
#include "tinyobjloader/tiny_obj_loader.h"
#include "Debug.hpp"
#include "GraphicsDataMisc.hpp"
#include "Materials.hpp"
#include "UniformBufferMaterial.hpp"
#include "TextureManager.hpp"
#include "Engine.hpp"
#include "EngineHeader.hpp"
#include <cstring>
#include <algorithm>
#include "GraphiquePipeline.hpp"
#include "ShaderPair.hpp"

namespace Ge
{
    bool MaterialManager::initialize(GraphicsDataMisc * gdm)
    {
        m_gdm = gdm;
		glGenBuffers(1, &m_ssbo);
		m_gdm->str_ssbo.str_material = m_ssbo;
		m_gdm->str_default_material = m_defaultMaterial = createMaterial();				
        Debug::INITSUCCESS("MaterialManager");
        return true;
    }

	Materials * MaterialManager::getDefaultMaterial()
	{
		return m_defaultMaterial;
	}

    void MaterialManager::lowDrawPriority(Materials* mat)
    {
        auto it = std::find(m_materials.begin(), m_materials.end(), mat);

        if (it != m_materials.end() && it != m_materials.end() - 1)
        {
            m_materials.erase(it);
            m_materials.push_back(mat);
        }
        updateStorage();
    }

    Materials * MaterialManager::createMaterial()
    {             
		Materials * material = m_pool.newObject(0, m_gdm);
		m_materials.insert(m_materials.begin(),material);
		m_gdm->str_dataMisc.materialCount = m_materials.size();
        updateStorage();
		return material;
    }

	void MaterialManager::destroyMaterial(Materials * material)
	{				
		m_materials.erase(std::remove(m_materials.begin(), m_materials.end(), material), m_materials.end());
		for (unsigned int i = 0; i < m_materials.size(); i++)
		{
			m_materials[i]->setIndex(i);
		}
		Engine::getPtrClass().modelManager->clearInstancedMaterial(material);		
        m_pool.deleteObject(material);
		m_gdm->str_dataMisc.materialCount = m_materials.size();
		updateStorage();
	}

	void MaterialManager::updateStorage()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(UniformBufferMaterial)*m_materials.size(), nullptr, GL_DYNAMIC_DRAW);
		for (int i = 0; i < m_materials.size();i++)
		{
			m_materials[i]->setIndex(i);
			m_materials[i]->updateUniformBufferMaterial();
		}		
	}

	void MaterialManager::updateMaterialExecutionOrder()
	{
		std::sort(m_materials.begin(), m_materials.end(), [](const Materials* a, const Materials* b) 
		{
			return !a->getDepthTest() || a->getPipeline()->getShaderPair()->transparency < b->getPipeline()->getShaderPair()->transparency;
		});
		updateStorage();
	}

    std::vector<Materials*> & MaterialManager::loadMltMaterial(const char* path,bool filter,TextureManager* tm)
    {
        std::vector<Materials*> mats;
        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(path))
        {
            if (!reader.Error().empty())
            {
                Debug::Error("TinyObjLoader error: %s", reader.Error().c_str());
            }
            return mats;
        }
        const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

        // Get the shapes from the reader
        const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();

        const char* lastSlash = strrchr(path, '/');
        char* result = nullptr;
        if (lastSlash != nullptr)
        {
            ptrdiff_t position = lastSlash - path;
            result = new char[position + 2];
            strncpy(result, path, position + 1);
            result[position + 1] = '\0';
        }
        if (result == nullptr)
        {
            return mats;
        }

        std::map<std::string, Textures*> m_textures;

        for (size_t shapeIdx = 0; shapeIdx < shapes.size(); shapeIdx++)
        {
            const tinyobj::shape_t& shape = shapes[shapeIdx];

            // Assuming each shape uses a single material
            if (shape.mesh.material_ids.size() > 0)
            {
                int materialIndex = shape.mesh.material_ids[0];
                if (materialIndex >= 0 && materialIndex < materials.size())
                {
                    const tinyobj::material_t& material = materials[materialIndex];

                    if (m_textures.find(material.diffuse_texname) == m_textures.end())
                    {
                        std::string npath = result + material.diffuse_texname;
                        m_textures[material.diffuse_texname] = tm->createTexture(npath.c_str(), filter);
                    }

                    mats.push_back(createMaterial());
                    mats.back()->setAlbedoTexture(m_textures[material.diffuse_texname]);
                }
            }
        }

        delete[] result;
        m_gdm->str_dataMisc.materialCount = m_materials.size();
		updateStorage();
        return mats;
    }

	const std::vector<Materials *> & MaterialManager::getMaterials() const
	{
		return m_materials;
	}

    void MaterialManager::release()
    {
        for (int i = 0; i < m_materials.size();i++)
		{			
            m_pool.deleteObject(m_materials[i]);
		}
		m_materials.clear();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glDeleteBuffers(1, &m_ssbo);
        Debug::RELEASESUCCESS("MaterialManager");
    }
}