#ifndef __ENGINE_LIGHT_MANAGER__
#define __ENGINE_LIGHT_MANAGER__

#include "Initializer.hpp"
#include "Manager.hpp"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "MemoryManager.hpp"

namespace Ge
{
	class Lights;
	class PointLight;
	class SpotLight;
	class DirectionalLight;
}

namespace Ge
{
	class LightManager final : public InitializerAPI, public Manager
    {
	protected:
		friend class RenderingEngine;
		bool initialize(GraphicsDataMisc * gdm);
		void release();
		unsigned int getSsboShadow() const;
		unsigned int getTextureShadowArray() const;
		const std::vector<unsigned int>& getFrameShadowBuffer() const;
		void updateStorage();
	protected:
		friend class Lights;
		void updateStorageShadow();
	protected:
		friend class Camera;
		void updateShadowCascadeMatrix();
    public:
		SpotLight * createSpotLight(glm::vec3 position, glm::vec3 color, glm::vec3 euler, float angle, std::string name = "SpotLight");
		DirectionalLight *createDirectionalLight(glm::vec3 euler, glm::vec3 color, std::string name = "DirectionalLight");
		PointLight *createPointLight(glm::vec3 position, glm::vec3 color, std::string name = "PointLight");
		void destroyLight(Lights *light);
    private:
		MemoryPool<Lights> m_pool;
		GraphicsDataMisc * m_gdm;
		std::vector<Lights *> m_lights;		
		std::vector<unsigned int> m_frameBufferDepthShadow;
		unsigned int m_ssboShadow;
		unsigned int m_textureShadowArray;
    };
}

#endif //!__ENGINE_LIGHT_MANAGER__