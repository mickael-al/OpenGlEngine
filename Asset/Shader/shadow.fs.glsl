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

layout(binding = 0) uniform sampler2D albedoTexture;

uniform int offsetUbo;
uniform int offsetShadow;

layout(location = 0) in vec2 fragTexCoord;

void main()
{
	vec4 col = texture(albedoTexture, fragTexCoord);
	if (col.a < 0.5)
	{
		discard;
	}
}
