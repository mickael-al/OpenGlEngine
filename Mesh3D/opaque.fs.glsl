#version 330

// il n'y a plus de variable predefinie en sortie: au revoir gl_FragColor
// on peut appeler la variable comme on veut mais doit etre marquee 'out vec4'

uniform sampler2D u_Sampler;

out vec4 o_FragColor;

in vec3 v_FragPosition; // dans le repere du monde
in vec3 v_Normal;
in vec2 v_TexCoords;

void main(void)
{
    o_FragColor = texture(u_Sampler, v_TexCoords);
    // debug des normales
    //o_FragColor = vec4(v_Normal * 0.5 + 0.5, 1.0);
}