#version 430

in vec3 a_Position;
in vec3 a_Normal;
in vec2 a_TexCoords;

out vec3 v_FragPosition;
out vec2 v_TexCoords;
out vec3 v_Normal;

uniform mat4 u_WorldMatrix;

uniform MatrixCamera
{
	vec3 position;
	mat4 u_ViewMatrix;
	mat4 u_ProjectionMatrix;
}mc;

uniform Materials
{
	vec3 diffuseColor;
	vec2 offset;
	vec2 tilling;
	float metallic;
	float roughness;
	float normal;
	float ao;
}mat;

uniform UniformBufferDiver
{
	uint maxLight;
	uint maxShadow;
	float u_time;
	float gamma;
}ubd;

void main(void)
{
	v_TexCoords = mat.offset + a_TexCoords * mat.tilling;
	//v_Normal = mat3(transpose(inverse(u_WorldMatrix))) * a_Normal;
	v_Normal = (u_WorldMatrix * vec4(a_Normal,0.0)).xyz;
	vec4 position_world = u_WorldMatrix * vec4(a_Position, 1.0);
	v_FragPosition = position_world.xyz;
	gl_Position = mc.u_ProjectionMatrix * mc.u_ViewMatrix * position_world;
}
