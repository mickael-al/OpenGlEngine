#ifndef __ENGINE_SHADOW_MATRIX__
#define __ENGINE_SHADOW_MATRIX__

#include "glm/glm.hpp"

struct ShadowMatrix
{	
	glm::mat4 projview;
	glm::vec3 pos;
	float splitDepth;
};

#endif//!__ENGINE_SHADOW_MATRIX__

