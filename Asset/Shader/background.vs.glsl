#version 450
layout (location = 0) in vec3 aPos;

layout(std430, binding = 0) buffer UniformBufferCamera
{
	vec3 camPos;
	mat4 view;
	mat4 proj;
} ubc;

layout(location = 0) out vec3 WorldPos;
layout(location = 1) out vec3 ViewPos;

void main()
{
    WorldPos = aPos;

	mat4 rotView = mat4(mat3(ubc.view));
	vec4 clipPos = ubc.proj * rotView * vec4(WorldPos, 1.0);

	gl_Position = clipPos.xyww;
}