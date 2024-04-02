#ifndef __ENGINE_UNIFORM_BUFFER_MATERIAL__
#define __ENGINE_UNIFORM_BUFFER_MATERIAL__

#include "glm/glm.hpp"

struct UniformBufferMaterial
{
    glm::vec3 albedo;
	alignas(16) glm::vec2 offset;
    glm::vec2 tilling;
    float metallic;
    float roughness;
    float normal;
    float ao;    
    int albedoMap;
    int normalMap;
    int metallicMap;
    int roughnessMap;
    int aoMap;
    int castShadow;//ignore -> //0 nothing // 1= cast shadow //2 = receive the shadow // 3 == 1&&2
    int orientation;//0 = nothing// 1 = y only // 2 = xy
};

#endif //__ENGINE_UNIFORM_BUFFER_MATERIAL__