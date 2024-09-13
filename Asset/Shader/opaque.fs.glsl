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
layout(location = 2) out vec4 gColor;
layout(location = 3) out vec3 gOther;

void main(void)
{   
	vec4 col = texture(albedoTexture, fragTexCoord);
	if (col.a < 0.5)
	{
		discard;
	}
	gPosition = ViewPos;
	vec3 normal = texture(normalTexture, fragTexCoord).rgb;
	normal = mix(vec3(0.5, 0.5, 1.0), normal, ubm.ubm[imaterial].normal);
	normal = normalize(normal * 2.0 - 1.0);
	gNormal.rgb = normalize(TBN * normal);
	
	gColor = ubm.ubm[imaterial].albedo * col * vec4(Color, 1.0);
	gOther.r = ubm.ubm[imaterial].metallic * texture(metallicTexture, fragTexCoord).r;
	gOther.g = ubm.ubm[imaterial].roughness * texture(roughnessTexture, fragTexCoord).r;
	vec2 aoe = texture(oclusionTexture, fragTexCoord).rg;
	gOther.b = ubm.ubm[imaterial].ao * aoe.x;
	gNormal.a = ubm.ubm[imaterial].emit * aoe.y;
}