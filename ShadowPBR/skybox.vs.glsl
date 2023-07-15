#version 430

in vec3 a_Position;
out vec3 V_TexCoords;

uniform MatrixCamera
{
	vec3 position;
	mat4 u_ViewMatrix;
	mat4 u_ProjectionMatrix;
}mc;

void main()
{
	V_TexCoords = a_Position;
    gl_Position = mc.u_ProjectionMatrix * mc.u_ViewMatrix * vec4(a_Position, 1.0);
}