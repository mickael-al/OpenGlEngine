#version 430

out vec4 FragColor;
in vec3 V_TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, V_TexCoords);
}