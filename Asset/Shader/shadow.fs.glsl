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

void main()
{

}
