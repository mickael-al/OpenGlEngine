#include "SkyboxManager.hpp"

/*
namespace Ge
{
	Skybox * SkyboxManager::currentSkybox = nullptr;
	bool SkyboxManager::initialize(ModelManager * mM, GraphiquePipelineManager * gPM)
	{
		
		Debug::INITSUCCESS("SkyboxManager");
		return true;
	}

	Skybox * SkyboxManager::GetCurrentSkybox()
	{
		return currentSkybox;
	}

	void SkyboxManager::initDescriptor(VulkanMisc * vM)
	{
		if (m_descriptor.size() == 0)
		{
			m_descriptor.push_back(new Descriptor(vM, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1));
		}
	}

	void SkyboxManager::updateDescriptor()
	{
		std::vector<VkDescriptorImageInfo> imageInfo{};
		VkDescriptorImageInfo imageI{};
		for (int i = 0; i < 1; i++)
		{
			imageI.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageI.imageView = currentSkybox->getTextureCubeMap()->getVkImageView();
			imageI.sampler = currentSkybox->getTextureCubeMap()->getVkSampler();
			imageInfo.push_back(imageI);
		}
		m_descriptor[0]->updateCount(vulkanM, 1, imageInfo);
	}

	Skybox * SkyboxManager::loadSkybox(TextureCubeMap * tCM)
	{		
		if (currentSkybox != nullptr)
		{
			m_skybox.erase(std::find(m_skybox.begin(), m_skybox.end(), currentSkybox), m_skybox.end());
			delete(currentSkybox);
		}
		currentSkybox = new Skybox(tCM, cubeMapBaseModel, m_skyboxPipeline->getIndex());
		m_skybox.push_back(currentSkybox);
		updateDescriptor();
		return currentSkybox;
	}

	Skybox* SkyboxManager::createSkybox(TextureCubeMap* tCM)
	{
		return new Skybox(tCM, cubeMapBaseModel, m_skyboxPipeline->getIndex());
	}
	
	void SkyboxManager::changeSkybox(Skybox* sky)
	{
		if (sky == nullptr)
		{
			currentSkybox = m_skybox[0];
		}
		else
		{
			currentSkybox = sky;
		}
		updateDescriptor();
	}

	void SkyboxManager::release()
	{
		currentSkybox = nullptr;
		for (int i = 0; i < m_skybox.size(); i++)
		{
			delete(m_skybox[i]);
		}
		m_skybox.clear();
		for (int i = 0; i < m_descriptor.size(); i++)
		{
			delete m_descriptor[i];
		}
		m_descriptor.clear();
		Debug::RELEASESUCCESS("SkyboxManager");
	}
}*/