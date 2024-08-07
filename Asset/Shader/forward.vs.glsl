#version 450

layout(std430, binding = 0) buffer UniformBufferCamera
{
	vec3 camPos;
	mat4 view;
	mat4 proj;
} ubc;

struct StructUBL
{
	vec3 position;
	vec3 color;
	vec3 direction;
	float range;
	float spotAngle;
	int status;
	int shadowID;
};

layout(std430, binding = 1) buffer UniformBufferLight
{
	StructUBL ubl[];
} ubl;

layout(std430, binding = 2) buffer UniformBufferDivers
{
	int maxLight;
	float u_time;
	float gamma;
	float ambiant;
	float fov;
	bool ortho;
} ubd;

in vec3 a_Position;
in vec3 a_Normal;
in vec3 a_Tangents;
in vec3 a_Color;
in vec2 a_TexCoords;

out vec2 v_UV;

void main(void)
{
	v_UV = a_TexCoords;
	gl_Position = vec4(a_Position,1.0);
}