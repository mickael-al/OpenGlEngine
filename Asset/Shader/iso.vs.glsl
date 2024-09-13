#version 450

layout(std430, binding = 0) buffer UniformBufferCamera
{
	vec3 camPos;
	mat4 view;
	mat4 proj;
} ubc;

struct StructUBO
{
	mat4 ubo;
	int mat_index;
};

layout(std430, binding = 1) buffer UniformBufferObject
{
	StructUBO ubo[];
}ubo;


struct StructUBM
{
	vec4 albedo;
	vec2 offset;
	vec2 tilling;
	float metallic;
	float roughness;
	float normal;
	float ao;
	float emit;
};

layout(std430, binding = 2) buffer UniformBufferMaterials
{
	StructUBM ubm[];
} ubm;

layout(std430, binding = 3) buffer UniformBufferDivers
{
	int maxLight;
	float u_time;
	float gamma;
	float ambiant;
	float fov;
	bool ortho;
} ubd;

uniform int offsetUbo;

in vec3 a_Position;
in vec3 a_Normal;
in vec3 a_Tangents;
in vec3 a_Color;
in vec2 a_TexCoords;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 Color;
layout(location = 2) out vec3 LocalPos;
layout(location = 3) out vec3 ViewPos;
layout(location = 4) out mat3 TBN;
layout(location = 7) out flat int imaterial;
layout(location = 8) out vec3 size;

vec3 extractScale(mat4 modelMatrix) 
{
	// Les vecteurs de base de la matrice de modèle
	vec3 scaleX = vec3(modelMatrix[0][0], modelMatrix[0][1], modelMatrix[0][2]);
	vec3 scaleY = vec3(modelMatrix[1][0], modelMatrix[1][1], modelMatrix[1][2]);
	vec3 scaleZ = vec3(modelMatrix[2][0], modelMatrix[2][1], modelMatrix[2][2]);

	// Calcul des facteurs d'échelle comme les longueurs des vecteurs de base
	float scaleXLength = length(scaleX);
	float scaleYLength = length(scaleY);
	float scaleZLength = length(scaleZ);

	// Retourne les facteurs d'échelle
	return vec3(scaleXLength, scaleYLength, scaleZLength);
}

void main()
{
	imaterial = ubo.ubo[offsetUbo + gl_InstanceID].mat_index;
	fragTexCoord = ubm.ubm[imaterial].offset + a_TexCoords * ubm.ubm[imaterial].tilling;
	vec4 wp = ubc.view * ubo.ubo[offsetUbo + gl_InstanceID].ubo * vec4(a_Position, 1.0);
	ViewPos = vec3(wp);
	size = extractScale(ubo.ubo[offsetUbo + gl_InstanceID].ubo);
	LocalPos = a_Position;
	Color = vec3(0.0, 0.9, 0.0);
	vec3 T = normalize(vec3(ubo.ubo[offsetUbo + gl_InstanceID].ubo * vec4(a_Tangents, 0.0)));
	vec3 N = normalize(vec3(ubo.ubo[offsetUbo + gl_InstanceID].ubo * vec4(a_Normal, 0.0)));
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	TBN = mat3(T, B, N);
	gl_Position = ubc.proj * wp;
}
