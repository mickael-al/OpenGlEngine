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
	vec3 albedo;
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
layout(location = 2) in vec3 LocalPos;
layout(location = 3) in vec3 WorldPos;
layout(location = 4) in mat3 TBN;
layout(location = 7) flat in int imaterial;
layout(location = 8) in float Depth;
layout(location = 9) in vec3 size;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gColorSpec;
layout(location = 3) out vec2 gOther;

void main(void)
{
	float t = ubm.ubm[imaterial].roughness;
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


	gPosition.rgb = WorldPos;
	vec3 normal = texture(normalTexture, fragTexCoord).rgb;
	normal = mix(vec3(0.5, 0.5, 1.0), normal, ubm.ubm[imaterial].normal);
	normal = normalize(normal * 2.0 - 1.0);
	gNormal.rgb = normalize(TBN * normal);

	gColorSpec.rgb = ubm.ubm[imaterial].albedo * Color * texture(albedoTexture, fragTexCoord).rgb;
	gColorSpec.a = ubm.ubm[imaterial].metallic * texture(metallicTexture, fragTexCoord).r;
	gNormal.a = ubm.ubm[imaterial].roughness * texture(roughnessTexture, fragTexCoord).r;
	gPosition.a = ubm.ubm[imaterial].ao * texture(oclusionTexture, fragTexCoord).r;
	gOther.r = Depth;
	gOther.g = 2.0;//emit
}