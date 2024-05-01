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

uniform int offsetUbo;
uniform int offsetShadow;

in vec3 a_Position;
in vec3 a_Normal;
in vec3 a_Tangents;
in vec3 a_Color;
in vec2 a_TexCoords;

void main()
{
	vec4 wp = ubo.ubo[offsetUbo + gl_InstanceID].ubo * vec4(a_Position, 1.0);
	gl_Position = ubs.s[offsetShadow].projview * wp;
}
