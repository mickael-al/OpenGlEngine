#ifndef __ENGINE_LIGHT_MANAGER__
#define __ENGINE_LIGHT_MANAGER__

#include <vector>
#include "Initializer.hpp"

namespace Ge
{
    class Lights;
    class LightManager final : public Initializer
    {
    public:
        bool initialize();
        void release();
        /*SpotLight* createSpotLight(glm::vec3 position, glm::vec3 color, glm::vec3 euler, float angle, std::string name = "SpotLight");
        DirectionalLight *createDirectionalLight(glm::vec3 euler, glm::vec3 color, std::string name = "DirectionalLight");
        PointLight *createPointLight(glm::vec3 position, glm::vec3 color, std::string name = "PointLight");
        void destroyLight(Lights *light);*/
    private:
        std::vector<Lights *> m_lights;
		std::vector<Lights *> m_destroy_lights;
    };
}

#endif //__ENGINE_LIGHT_MANAGER__