#version 430

in vec3 a_Position;
in vec3 a_Normal;
in vec3 a_Tangents;
in vec2 a_TexCoords;

out vec3 v_FragPosition;
out vec2 v_TexCoords;
out vec4 v_ShadowCoord;
out mat3 v_TBN;

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

uniform MatrixShadow
{
    vec3 position;
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
}ms;

const mat4 biasMatrix = mat4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
);
void main(void)
{
	v_TexCoords = mat.offset + a_TexCoords * mat.tilling;
	//v_Normal = mat3(transpose(inverse(u_WorldMatrix))) * a_Normal;
	vec3 T = normalize(vec3(u_WorldMatrix * vec4(a_Tangents, 0.0)));
	vec3 N = normalize(vec3(u_WorldMatrix * vec4(a_Normal, 0.0)));	
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	v_TBN = mat3(T, B, N);
	vec4 position_world = u_WorldMatrix * vec4(a_Position, 1.0);
	v_FragPosition = position_world.xyz;
	v_ShadowCoord = ((ms.u_ProjectionMatrix*ms.u_ViewMatrix*u_WorldMatrix))*vec4(a_Position, 1.0);

	gl_Position = mc.u_ProjectionMatrix * mc.u_ViewMatrix * position_world;
}
