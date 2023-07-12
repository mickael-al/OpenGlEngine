#version 120

// variable predefinie 
// en sortie: gl_FragColor
// elle est de type vec4

varying vec4 v_Color;

void main(void)
{
    gl_FragColor = v_Color;
}