#ifndef __PROJECT_DATA__
#define __PROJECT_DATA__

#include <string>
#include "json_struct.h"
#include <glm/glm.hpp>
#include "GObject.hpp"
namespace Ge
{
	class Textures;
	class Model;
	class ShapeBuffer;
	class Materials;
	class GraphiquePipeline;
	class DirectionalLight;
	class PointLight;
	class SpotLight;
}
using namespace Ge;

struct EditorConfig
{
	bool gameMode;
	bool inspector;
	bool project;
	bool hiearchy;
	std::string lastProjectpathOpen;
	EditorConfig()
	{
		gameMode = false;
		inspector = false;
		project = false;
		hiearchy = false;
		lastProjectpathOpen = "";
		 
	}
	JS_OBJ(gameMode, inspector, project, hiearchy, lastProjectpathOpen);
};

struct ProjectData
{
	char projetName[64];
	char projetPath[256];//base path
	std::string assetPath;
	std::string ressourcePath;
	std::string lastSceneOpen;
	ProjectData()
	{
		projetName[0] = '\0';
		projetPath[0] = '\0';
		assetPath = "";
		ressourcePath = "";
		lastSceneOpen = "";
	}
	JS_OBJ(projetName, projetPath, assetPath, ressourcePath, lastSceneOpen);
};

JS_OBJ_EXT(glm::vec2, x, y);
JS_OBJ_EXT(glm::vec3, x, y, z);
JS_OBJ_EXT(glm::quat, x, y, z,w);

struct ModelData
{
	std::string name;
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	int idMaterial;
	int idBuffer;
	ModelData()
	{
		idMaterial = -1;
		idBuffer = -1;
	}
	JS_OBJ(name,position, rotation, scale,idMaterial, idBuffer);
};

struct BufferData
{
	std::string path;
	bool normalRecalculate;
	JS_OBJ(path, normalRecalculate);
};

struct ShaderData
{
	std::string frag;
	std::string vert;
	bool back;
	bool multiS;
	bool transparency;
	int cullmode;
	JS_OBJ(frag, vert, back, multiS, transparency, cullmode);
};

struct MaterialData
{
	glm::vec3 albedo;
	glm::vec2 offset;
	glm::vec2 tilling;
	float metallic;
	float roughness;
	float normal;
	float ao;
	int albedoMap;
	int normalMap;
	int metallicMap;
	int RoughnessMap;
	int aoMap;
	int shader;
	MaterialData()
	{
		albedoMap = -1;
		normalMap = -1;
		metallicMap = -1;
		RoughnessMap = -1;
		aoMap = -1;
		shader = -1;
	}
	JS_OBJ(albedo, offset, tilling, metallic, roughness, normal, ao, albedoMap, normalMap, metallicMap, RoughnessMap, aoMap, shader);
};

struct TextureData
{
	std::string path;
	bool filter;
	JS_OBJ(path, filter);
};

struct LightData
{
	std::string name;
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	glm::vec3 color;
	float range;
	float spotAngle;
	bool shadow;
	int status;
	JS_OBJ(name, position, rotation, scale, color, range, spotAngle, shadow, status);
};

struct SceneData
{
	std::string name;
	std::string currentPath;
	glm::vec3 freeCamPos;
	glm::quat freeCamRot;
	std::vector<BufferData> bufferData;
	std::vector<ShaderData> shaderData;
	std::vector<MaterialData> materialData;	
	std::vector<TextureData> textureData;
	std::vector<ModelData> modelData;
	std::vector<LightData> lightData;

	/*Engine Part*/
	std::vector<ShapeBuffer*> buffer;
	std::vector<GraphiquePipeline*> shader;
	std::vector<Materials*> materials;
	std::vector<Textures*> textures;
	std::vector<Model*> models;
	std::vector<DirectionalLight*> dlight;		
	std::vector<PointLight*> plight;
	std::vector<SpotLight*> slight;
	/*Engine Part*/
	SceneData()
	{
		name = "";
		currentPath = "";
	}	
	JS_OBJ(freeCamPos, freeCamRot,bufferData, shaderData, materialData, textureData, modelData, lightData);
};

#endif //!__PROJECT_DATA__