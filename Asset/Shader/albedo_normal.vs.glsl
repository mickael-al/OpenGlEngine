#version 450

in vec3 a_Position;
in vec3 a_Normal;
in vec3 a_Tangents;
in vec3 a_Color;
in vec2 a_TexCoords;

out vec2 v_UV;

void main(void)
{
	v_UV = a_TexCoords;
	gl_Position = vec4(a_Position, 1.0);
}