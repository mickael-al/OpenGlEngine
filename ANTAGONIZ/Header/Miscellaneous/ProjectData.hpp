#ifndef __PROJECT_DATA__
#define __PROJECT_DATA__

#include <string>
#include "json_struct.h"
#include <glm/glm.hpp>
#include "GObject.hpp"
#include "ModulePP.hpp"

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
	class SoundBuffer;
	class AudioSource;
	class Empty;
}
using namespace Ge;

struct EditorConfig
{
	bool gameMode;
	bool inspector;
	bool project;
	bool hiearchy;
	bool postProcess;
	bool autoLoadProject;
	std::string lastProjectpathOpen;	
	EditorConfig()
	{
		gameMode = false;
		inspector = false;
		project = false;
		hiearchy = false;
		postProcess = false;
		autoLoadProject = false;
		lastProjectpathOpen = "";		 
	}
	JS_OBJ(gameMode, inspector, project, hiearchy, postProcess, autoLoadProject, lastProjectpathOpen);
};

struct ScriptData
{
	std::string pathName;
	std::string data;	
	JS_OBJ(pathName, data);
};

struct ProjectData
{
	std::string projetName;
	std::string projetPath;//base path
	std::string assetPath;
	std::string ressourcePath;
	std::string lastSceneOpen;
	ProjectData()
	{
		projetName = "";
		projetPath = "";
		assetPath = "";
		ressourcePath = "";
		lastSceneOpen = "";
	}
	JS_OBJ(projetName, projetPath, assetPath, ressourcePath, lastSceneOpen);
};

JS_OBJ_EXT(glm::vec2, x, y);
#ifndef VEC3_JS
#define VEC3_JS
JS_OBJ_EXT(glm::vec3, x, y, z);
#endif // !VEC3_JS
JS_OBJ_EXT(glm::vec4, x, y, z,w);
JS_OBJ_EXT(glm::quat, x, y, z,w);
JS_OBJ_EXT(PPSetting, bloom, bloom_filter, bloom_threshold, bloom_intensity, exposure, fog, fog_min_distance, fog_max_distance, fog_mode, fog_color, oil_painting, oilp_radius);

struct CollisionData
{
	//CollisionType type;
	int type;//Box,Sphere,Capsule
	glm::vec3 position;
	glm::vec3 euler;
	glm::vec4 data;//3d extend radius capsule height and radius
	float mass;
	JS_OBJ(type,position, euler, data,mass);
};

struct PathFindingData
{
	bool has;
	glm::vec3 pathPosition;
	glm::vec3 zoneSize;
	glm::vec3 pointCount;
	std::string pathFolder;
	float pathLiasonPercent;
	JS_OBJ(has, pathPosition, zoneSize, pointCount, pathFolder, pathLiasonPercent);
};

struct ModelData
{
	std::string name;
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	int idMaterial;
	int idBuffer;
	int idParent;
	std::vector<ScriptData> scripts;
	std::vector<CollisionData> collisions;
	std::vector<std::string> tags;
	ModelData()
	{
		idMaterial = -1;
		idBuffer = -1;
		idParent = -1;
	}
	JS_OBJ(name,position, rotation, scale,idMaterial, idBuffer, idParent, scripts, collisions, tags);
};

struct BufferData
{
	std::string path;
	bool normalRecalculate;
	JS_OBJ(path, normalRecalculate);
};

struct ShaderData
{
	std::string name;
	std::string frag;
	std::string vert;
	bool back;
	bool multiS;
	bool transparency;
	int cullmode;
	JS_OBJ(name,frag, vert, back, multiS, transparency, cullmode);
};

struct MaterialData
{
	glm::vec4 albedo;
	glm::vec2 offset;
	glm::vec2 tilling;
	float metallic;
	float roughness;
	float normal;
	float ao;
	float emit;
	int albedoMap;
	int normalMap;
	int metallicMap;
	int RoughnessMap;
	int aoMap;
	int shader;
	bool active;
	bool depthTest;
	bool depthWrite;
	bool castShadow;	
	MaterialData()
	{
		albedoMap = -1;
		normalMap = -1;
		metallicMap = -1;
		RoughnessMap = -1;
		aoMap = -1;
		shader = -1;
		active = true;
		depthTest = true;
		depthWrite = true;
		castShadow = true;
	}
	JS_OBJ(albedo, offset, tilling, metallic, roughness, normal, ao, emit, albedoMap, normalMap, metallicMap, RoughnessMap, aoMap, shader, active, depthTest, depthWrite, castShadow);
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
	int idParent;
	std::vector<ScriptData> scripts;
	std::vector<CollisionData> collisions;
	std::vector<std::string> tags;
	LightData()
	{
		idParent = -1;
	}
	JS_OBJ(name, position, rotation, scale, color, range, spotAngle, shadow, status, idParent, scripts, collisions, tags);
};

struct SoundBufferData
{
	std::string path;
	JS_OBJ(path);
};

struct AudioSourceData
{
	std::string name;
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	float pitch;
	float gain;
	glm::vec3 velocity;
	bool loop;
	float rolloffFactor;   // Rolloff factor
	float maxDistance;   // Max distance for attenuation
	float refDistance;
	int idBuffer;
	int idParent;
	std::vector<ScriptData> scripts;
	std::vector<CollisionData> collisions;
	std::vector<std::string> tags;
	AudioSourceData()
	{
		idBuffer = -1;
		idParent = -1;
	}
	JS_OBJ(name, position, rotation, scale, pitch, gain, velocity, loop, rolloffFactor, maxDistance, refDistance, idBuffer, idParent, scripts, collisions, tags);
};

struct EmptyData
{
	std::string name;
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	int idParent;
	std::vector<ScriptData> scripts;
	std::vector<CollisionData> collisions;
	std::vector<std::string> tags;
	EmptyData()
	{
		idParent = -1;
	}
	JS_OBJ(name, position, rotation, scale, idParent, scripts, collisions, tags);
};

struct SceneData
{
	std::string name;
	std::string currentPath;
	std::string skyboxPath;
	glm::vec3 freeCamPos;
	glm::quat freeCamRot;
	std::vector<BufferData> bufferData;
	std::vector<ShaderData> shaderData;
	std::vector<MaterialData> materialData;	
	std::vector<TextureData> textureData;
	std::vector<ModelData> modelData;
	std::vector<LightData> lightData;
	std::vector<SoundBufferData> soundBufferData;
	std::vector<AudioSourceData> audioData;
	std::vector<EmptyData> emptyData;
	PathFindingData path;
	PPSetting pp;

	/*Engine Part*/
	std::vector<ShapeBuffer*> buffer;
	std::vector<GraphiquePipeline*> shader;
	std::vector<Materials*> materials;
	std::vector<Textures*> textures;
	std::vector<Model*> models;
	std::vector<DirectionalLight*> dlight;		
	std::vector<PointLight*> plight;
	std::vector<SpotLight*> slight;
	std::vector<SoundBuffer*> sound;
	std::vector<AudioSource*> audio;	
	std::vector<Empty*> empty;
	/*Engine Part*/
	SceneData()
	{
		name = "";
		currentPath = "";
		skyboxPath = "";
	}	
	JS_OBJ(freeCamPos, freeCamRot,bufferData, shaderData, materialData, textureData, modelData, lightData, soundBufferData, audioData, emptyData, path, skyboxPath, pp);
};

#endif //!__PROJECT_DATA__