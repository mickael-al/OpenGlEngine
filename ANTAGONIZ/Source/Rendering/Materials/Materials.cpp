#include "glcore.hpp"
#include "Materials.hpp"
#include "GraphicsDataMisc.hpp"
#include "imgui-cmake/Header/imgui.h"
#include "Textures.hpp"
#include "GraphiquePipeline.hpp"
#include "Debug.hpp"
#include <algorithm>
#include "Model.hpp"
#include "MaterialManager.hpp"
#include "Engine.hpp"
#include "PointeurClass.hpp"

namespace Ge
{
	Materials::Materials(unsigned int index, GraphicsDataMisc * gdm)
	{
		m_gdm = gdm;
		m_ubm.albedo = glm::vec4(1.0f, 1.0f, 1.0f,1.0f);
		m_ubm.metallic = 1.0f;		
		m_ubm.roughness = 1.0f;
		m_ubm.normal = 1.0f;
		m_ubm.ao = 1.0f;
		m_ubm.emit = 0.0f;
		m_ubm.tilling = glm::vec2(1.0f);
		m_ubm.offset = glm::vec2(0.0f);
		m_index = index;
		m_ssbo = m_gdm->str_ssbo.str_material;
		m_pipeline = m_gdm->str_default_pipeline;
		m_pc = Engine::getPtrClassAddr();

		m_albedoMap = gdm->str_default_texture;
		m_normalMap = gdm->str_default_normal_texture;
		m_metallicMap = gdm->str_default_texture;
		m_RoughnessMap = gdm->str_default_texture;
		m_aoMap = gdm->str_default_texture;
	}

	Materials::~Materials()
	{

	}

	void Materials::setColor(glm::vec4 color)
	{
		m_ubm.albedo = color;
		updateUniformBufferMaterial();
	}

	void Materials::setMetallic(float metal)
	{
		m_ubm.metallic = metal;
		updateUniformBufferMaterial();
	}

	void Materials::setRoughness(float roug)
	{
		m_ubm.roughness = roug;
		updateUniformBufferMaterial();
	}

	void Materials::setNormal(float normal)
	{
		m_ubm.normal = normal;
		updateUniformBufferMaterial();
	}

	void Materials::setOclusion(float ao)
	{
		m_ubm.ao = ao;
		updateUniformBufferMaterial();
	}

	void Materials::setEmission(float emit)
	{
		m_ubm.emit = emit;
		updateUniformBufferMaterial();
	}

	void Materials::setDepthTest(bool state)
	{
		m_depthTest = state;
		m_pc->materialManager->updateMaterialExecutionOrder();
	}

	bool Materials::getDepthTest() const
	{
		return m_depthTest;
	}

	void Materials::setDepthWrite(bool state)
	{
		m_depthWrite = state;
	}

	bool Materials::getDepthWrite() const
	{
		return m_depthWrite;
	}

	void Materials::setPipeline(GraphiquePipeline * p)
	{		
		m_pipeline = p;		
		m_pc->materialManager->updateMaterialExecutionOrder();
	}

	GraphiquePipeline * Materials::getPipeline() const
	{
		return m_pipeline;
	}

	std::vector<unsigned int> & Materials::getAditionalSSBO()
	{
		return m_aditionalSSBO;
	}

	unsigned int Materials::getAditionalInstanced() const
	{
		return m_aditionalInstanced;
	}

	void Materials::setAditionalInstanced(unsigned int instance)
	{
		m_aditionalInstanced = instance;
	}

	void Materials::setAlbedoTexture(Textures * albedo)
	{
		m_albedoMap = albedo == nullptr ? m_gdm->str_default_texture : albedo;
	}

	void Materials::setNormalTexture(Textures * normal)
	{
		m_normalMap = normal == nullptr ? m_gdm->str_default_normal_texture : normal;
	}

	void Materials::setMetallicTexture(Textures * metal)
	{
		m_metallicMap = metal == nullptr ? m_gdm->str_default_texture : metal;
	}

	void Materials::setRoughnessTexture(Textures * roug)
	{
		m_RoughnessMap = roug == nullptr ? m_gdm->str_default_texture : roug;
	}

	void Materials::setOclusionTexture(Textures * oclu)
	{
		m_aoMap = oclu == nullptr ? m_gdm->str_default_texture : oclu;
	}

	glm::vec4 Materials::getColor() const
	{
		return m_ubm.albedo;
	}

	float Materials::getMetallic() const
	{
		return m_ubm.metallic;
	}

	float Materials::getRoughness() const
	{
		return m_ubm.roughness;
	}

	float Materials::getNormal() const
	{
		return m_ubm.normal;
	}

	float Materials::getOclusion() const
	{
		return m_ubm.ao;
	}

	float Materials::getEmission() const
	{
		return m_ubm.emit;
	}

	Textures * Materials::getAlbedoTexture() const
	{
		return m_albedoMap;
	}

	Textures * Materials::getNormalTexture() const
	{
		return m_normalMap;
	}

	Textures * Materials::getMetallicTexture() const
	{
		return m_metallicMap;
	}

	Textures * Materials::getRoughnessTexture() const
	{
		return m_RoughnessMap;
	}

	Textures * Materials::getOclusionTexture() const
	{
		return m_aoMap;
	}

	void Materials::updateUniformBufferMaterial()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, m_index * sizeof(UniformBufferMaterial), sizeof(UniformBufferMaterial), &m_ubm);
	}

	int Materials::getIndex() const
	{
		return m_index;
	}

	void Materials::setIndex(int i)
	{
		m_index = i;
		for (int j = 0; j < m_model.size(); j++)
		{
			m_model[j]->mapMemory();
		}
	}

	glm::vec2 Materials::getOffset() const
	{
		return m_ubm.offset;
	}

	glm::vec2 Materials::getTilling() const
	{
		return m_ubm.tilling;
	}

	void Materials::setOffset(glm::vec2 off)
	{
		m_ubm.offset = off;
		updateUniformBufferMaterial();
	}

	void Materials::setTilling(glm::vec2 tilling)
	{
		m_ubm.tilling = tilling;
		updateUniformBufferMaterial();
	}

	void Materials::addModel(Model * model)
	{
		auto it = std::find(m_model.begin(), m_model.end(), model);

		if (it == m_model.end())
		{
			m_model.push_back(model);
		}
	}

	void Materials::removeModel(Model * model)
	{
		m_model.erase(std::remove(m_model.begin(), m_model.end(), model), m_model.end());
	}

	bool Materials::getDraw() const
	{
		return m_draw;
	}

	bool * Materials::getDrawAddr()
	{
		return &m_draw;
	}

	void Materials::setDraw(bool draw)
	{
		m_draw = draw;
	}

	bool Materials::getCastShadow() const
	{
		return m_castShadow;
	}

	void Materials::setCastShadow(bool cs)
	{
		m_castShadow = cs;
	}

	void Materials::onGUI()
	{
		if (m_gdm->str_default_material == this)
		{
			ImGui::TextColored(ImVec4(1.0f, 0, 0.0f, 1), "Default Material Not Saved");
		}		
		ImGui::TextColored(ImVec4(0.2f, 1, 0.2f, 1), "Material\n");
		if (ImGui::ColorEdit4("Albedo", (float *)&m_ubm.albedo))
		{
			updateUniformBufferMaterial();
		}
		if (ImGui::DragFloat2("Tilling", (float*)&m_ubm.tilling, 0.2f))
		{
			updateUniformBufferMaterial();
		}
		if (ImGui::DragFloat2("Offset", (float *)&m_ubm.offset, 0.2f))
		{
			updateUniformBufferMaterial();
		}
		if (ImGui::DragFloat("Metallic", &m_ubm.metallic, 0.05f))
		{
			updateUniformBufferMaterial();
		}
		if (ImGui::DragFloat("Roughness", &m_ubm.roughness, 0.05f))
		{
			updateUniformBufferMaterial();
		}
		if (ImGui::DragFloat("Normal", &m_ubm.normal, 0.05f))
		{
			updateUniformBufferMaterial();
		}
		if (ImGui::DragFloat("AO", &m_ubm.ao, 0.05f))
		{
			updateUniformBufferMaterial();
		}
		if (ImGui::DragFloat("Emission", &m_ubm.emit, 0.05f))
		{
			updateUniformBufferMaterial();
		}
		if (ImGui::Checkbox("Show", &m_draw))
		{

		}
	}
}