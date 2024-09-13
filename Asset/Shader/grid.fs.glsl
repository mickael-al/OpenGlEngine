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

layout(binding = 0) uniform sampler2D albedoTexture;
layout(binding = 1) uniform sampler2D normalTexture;
layout(binding = 2) uniform sampler2D metallicTexture;
layout(binding = 3) uniform sampler2D roughnessTexture;
layout(binding = 4) uniform sampler2D oclusionTexture;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 Color;
layout(location = 2) in vec3 ViewPos;
layout(location = 3) in mat3 TBN;
layout(location = 6) flat in int imaterial;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gColorSpec;
layout(location = 3) out vec3 gOther;

void main(void)
{
	/*float d = length(ubc.camPos - WorldPos) * (ubd.ortho ? 1.0f : (ubd.fov / 80.0f));
	float t = ubm.ubm[imaterial].roughness + (ubm.ubm[imaterial].normal > 0.5 ? clamp((d / 250.0)*0.4f, 0.0, 0.4) : clamp((d / 200.0)*0.5f, 0.0, 0.5));
	vec3 wp = vec3(WorldPos.x + (t / 2), WorldPos.y + (t / 2), WorldPos.z + (t / 2));

	float fvx = mod(wp.x, ubm.ubm[imaterial].metallic);
	float fvy = mod(wp.y, ubm.ubm[imaterial].metallic);
	float fvz = mod(wp.z, ubm.ubm[imaterial].metallic);
	
	if (fvx > t && fvy > t || fvx > t && fvz > t || fvz > t && fvy > t)
	{
		discard;
	}
	if ((wp.x >= 0 && wp.x < t) && (wp.y >= 0 && wp.y < t))
	{
		gPosition = vec4(0.25, 0.25, 0.7, clamp((1.0f-((d-25.0f) / 250.0))*0.6f, 0.0, 0.0));
	}
	else if((wp.x >= 0 && wp.x < t) && (wp.z >= 0 && wp.z < t))
	{		
		gPosition = vec4(0.25, 0.7, 0.25, clamp((1.0f-((d-25.0f) / 250.0))*0.6f, 0.0, 0.0));
	}
	else if((wp.y >= 0 && wp.y < t) && (wp.z >= 0 && wp.z < t))
	{
		gPosition = vec4(0.7, 0.25, 0.25, clamp((1.0f-((d-25.0f) / 250.0))*0.6f, 0.0, 0.0));
	}
	else
	{
		gPosition = vec4(0.4, 0.4, 0.45, clamp((1.0f - ((d - 25.0f) / 250.0))*0.4f, 0.0, 0.0));
	}*/
}