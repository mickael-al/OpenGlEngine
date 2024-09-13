#ifndef __ENGINE_API_DATA_MISC__
#define __ENGINE_API_DATA_MISC__

#include <glm/glm.hpp>
#include "GLFW/glfw3.h"
#include "DataMisc.hpp"
#include "SSBO.hpp"

namespace Ge
{
	class Textures;
	class Materials;
	class GraphiquePipeline;
	class Camera;
}
using namespace Ge;

struct GraphicsDataMisc
{
	GLFWwindow * str_window;
	unsigned int str_width;
	unsigned int str_height;
	Textures * str_default_texture;
	Textures* str_default_normal_texture;
	Textures* str_depth_texture;
	Materials * str_default_material;
	GraphiquePipeline * str_default_pipeline;
	GraphiquePipeline * str_default_pipeline_forward;
	GraphiquePipeline * str_default_pipeline_shadow;
	Camera* current_camera;
	DataMisc str_dataMisc;
	SSBO str_ssbo;
};

#endif //!__ENGINE_API_DATA_MISC__