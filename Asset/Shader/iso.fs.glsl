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

const float PI = 3.14159265359;

out vec4 o_FragColor;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 Color;
layout(location = 2) in vec3 LocalPos;
layout(location = 3) in vec3 Normal;
layout(location = 4) flat in int imaterial;

void main(void)
{
	float t = ubm.ubm[imaterial].roughness;
	vec3 size = ubm.ubm[imaterial].albedo;
	vec3 fragPos = LocalPos * size + size*0.5;
	if ((fragPos.x < t || fragPos.x > size.x - t) && fragPos.y > t && fragPos.z > t && fragPos.y < size.y-t && fragPos.z < size.z - t)
	{ 
		discard;
	}
	
	if ((fragPos.y < t || fragPos.y > size.y - t) && fragPos.x > t && fragPos.z > t && fragPos.x < size.x - t && fragPos.z < size.z - t)
	{
		discard;
	}

	if ((fragPos.z < t || fragPos.z > size.z - t) && fragPos.x > t && fragPos.y > t && fragPos.x < size.x - t && fragPos.y < size.y - t)
	{
		discard;
	}

	o_FragColor = vec4(0.9, 0.9, 0.0, 0.9);	
}