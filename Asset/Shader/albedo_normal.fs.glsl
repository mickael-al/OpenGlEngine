#version 450
out vec4 FragColor;

in vec2 v_UV;

uniform sampler2D albedoMap;
uniform float strength;

void main()
{
    vec2 texelSize = 1.0 / textureSize(albedoMap, 0);

    float heightL = texture(albedoMap, v_UV - vec2(texelSize.x, 0.0)).r;
    float heightR = texture(albedoMap, v_UV + vec2(texelSize.x, 0.0)).r;
    float heightD = texture(albedoMap, v_UV - vec2(0.0, texelSize.y)).r;
    float heightU = texture(albedoMap, v_UV + vec2(0.0, texelSize.y)).r;

    vec3 normal = vec3(heightL - heightR, heightD - heightU, 1.0 / strength);

    normal = normalize(normal);

    FragColor = vec4(normal * 0.5 + 0.5,1.0);
}