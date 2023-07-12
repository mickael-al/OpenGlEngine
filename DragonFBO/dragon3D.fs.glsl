// version 130 = OpenGL 3.0
// version 150 = OpenGL 3.2
// version 330 = OpenGL 3.3
#version 330

out vec4 o_FragColor;

in vec3 v_Normal;

// directional light simple 
const vec3 LightDirection = vec3(0.0, 0.0, -1.0);

float Lambert(vec3 N, vec3 L)
{
    return max(dot(N, L), 0.0);
}

void main(void)
{
    // v_Normal est interpolee LINEAIREMENT par le rasterizer
    // il faut renormaliser (re-spherifier) la direction
    vec3 N = normalize(v_Normal);
    // L est l'oppose du vecteur incident I
    vec3 L = -LightDirection;
    
    float diffuse = Lambert(N, L);

    // debug des normales
    o_FragColor = vec4(vec3(diffuse), 1.0);
}