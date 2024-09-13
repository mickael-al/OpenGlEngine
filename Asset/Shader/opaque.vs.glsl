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
layout(location = 2) out vec3 ViewPos;
layout(location = 3) out mat3 TBN;
layout(location = 6) out flat int imaterial;

void main()
{
	imaterial = ubo.ubo[offsetUbo + gl_InstanceID].mat_index;
	fragTexCoord = ubm.ubm[imaterial].offset + a_TexCoords * ubm.ubm[imaterial].tilling;
	vec4 wp = ubc.view * ubo.ubo[offsetUbo + gl_InstanceID].ubo * vec4(a_Position, 1.0);
	ViewPos = vec3(wp);
	Color = a_Color;
	vec3 T = normalize(vec3(ubo.ubo[offsetUbo + gl_InstanceID].ubo * vec4(a_Tangents, 0.0)));
	vec3 N = normalize(vec3(ubo.ubo[offsetUbo + gl_InstanceID].ubo * vec4(a_Normal, 0.0)));
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	TBN = mat3(T, B, N);
	gl_Position = ubc.proj * wp;
}
