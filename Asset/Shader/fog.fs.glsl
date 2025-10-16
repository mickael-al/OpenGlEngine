#version 450 core

in vec2 texCoord;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_colorBuffer;     // Texture couleur après lighting
layout(binding = 1) uniform sampler2D u_depthBuffer;     // Texture de profondeur (linear depth si possible)

uniform vec2 u_resolution;           // Résolution écran
uniform vec3 u_color;                // Couleur du fog
uniform float u_min;                 // Distance min pour fog (full transparent)
uniform float u_max;                 // Distance max pour fog (full opaque)
uniform int u_mode;                 // 0 = linéaire, 1 = exp, 2 = exp²
uniform float u_time;                // Pour effets animés éventuels (non utilisé ici)
uniform vec2 u_cameraData;                // near et far

float LinearizeDepth(float depth)
{
    // Reconvertit la profondeur non linéaire [0..1] en vraie distance
    float near = u_cameraData.x;     // À adapter selon ton moteur
    float far = u_cameraData.y;   // Idem
    float z = depth * 2.0 - 1.0;     // NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

float ComputeFogFactor(float distance)
{
    if (u_mode == 0) // Linéaire
    {
        return clamp((distance - u_min) / (u_max - u_min), 0.0, 1.0);
    }
    else if (u_mode == 1) // Exponentiel
    {
        float density = 1.0 / (u_max - u_min);
        return clamp(1.0 - exp(-distance * density), 0.0, 1.0);
    }
    else if (u_mode == 2) // Exponentiel²
    {
        float density = 1.0 / (u_max - u_min);
        return clamp(1.0 - exp(-pow(distance * density, 2.0)), 0.0, 1.0);
    }

    return 0.0;
}

void main()
{
    vec3 sceneColor = texture(u_colorBuffer, texCoord).rgb;
    float depth = texture(u_depthBuffer, texCoord).r;

    float linearDepth = LinearizeDepth(depth);
    float fogFactor = ComputeFogFactor(linearDepth);

    vec3 finalColor = mix(sceneColor, u_color, fogFactor);
    FragColor = vec4(finalColor, 1.0);
}
