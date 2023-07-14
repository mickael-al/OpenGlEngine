#version 430

in vec3 a_Position;

uniform mat4 u_WorldMatrix;

uniform MatrixCamera
{
	vec3 position;
	mat4 u_ViewMatrix;
	mat4 u_ProjectionMatrix;
}mc;

void main()
{
	gl_Position = mc.u_ProjectionMatrix * mc.u_ViewMatrix * u_WorldMatrix * vec4(a_Position, 1.0);
}