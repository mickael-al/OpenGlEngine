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

struct StructUBL
{
	vec3 position;
	vec3 color;
	vec3 direction;
	float range;
	float spotAngle;
	int status;
};

layout(std430, binding = 2) buffer UniformBufferLight
{
	StructUBL ubl[];
} ubl;

struct StructUBM
{
	vec3 albedo;
	vec2 offset;
	vec2 tilling;
	float metallic;
	float roughness;
	float normal;
	float ao;
};

layout(std430, binding = 3) buffer UniformBufferMaterials
{
	StructUBM ubm[];
} ubm;

layout(std430, binding = 4) buffer UniformBufferDivers
{
	int maxLight;
	float u_time;
	float gamma;
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
layout(location = 3) out vec3 Normal;
layout(location = 4) out flat int imaterial;

void main()
{
	imaterial = ubo.ubo[offsetUbo + gl_InstanceID].mat_index;
	fragTexCoord = ubm.ubm[imaterial].offset + a_TexCoords * ubm.ubm[imaterial].tilling;
	vec4 wp = ubo.ubo[offsetUbo + gl_InstanceID].ubo * vec4(a_Position, 1.0);
	LocalPos = a_Position;
	Color = a_Color;
	Normal = normalize(vec3(ubo.ubo[offsetUbo + gl_InstanceID].ubo * vec4(a_Normal, 0.0)));
	gl_Position = ubc.proj * ubc.view * wp;
}
