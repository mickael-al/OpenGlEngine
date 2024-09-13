#version 450

struct StructShadow
{
	mat4 projview;
	vec3 pos;
	float splitDepth;
};

layout(std430, binding = 0) buffer UniformBufferShadow
{
	StructShadow s[];
} ubs;

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

uniform int offsetUbo;
uniform int offsetShadow;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangents;
layout(location = 3) in vec3 a_Color;
layout(location = 4) in vec2 a_TexCoords;

layout(location = 0) out vec2 fragTexCoord;

void main()
{
	int imaterial = ubo.ubo[offsetUbo + gl_InstanceID].mat_index;
	fragTexCoord = ubm.ubm[imaterial].offset + a_TexCoords * ubm.ubm[imaterial].tilling;
	vec4 wp = ubo.ubo[offsetUbo + gl_InstanceID].ubo * vec4(a_Position, 1.0);
	gl_Position = ubs.s[offsetShadow].projview * wp;
}
