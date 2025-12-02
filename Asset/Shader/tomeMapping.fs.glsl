#version 450

layout(binding = 0) uniform sampler2D originalImage;
layout(binding = 1) uniform sampler2D bloomImage;
uniform float exposure;
uniform float gamma;
uniform float intensity;

in vec2 texCoord;
layout(location = 0) out vec3 upsample;

vec3 adjustSaturation(vec3 color, float saturation)
{
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    return mix(vec3(luminance), color, saturation);
}
void main()
{
    vec3 original = texture(originalImage, texCoord).rgb;
    original = adjustSaturation(original,1.2);//TODO Remove and add pp 
    if (intensity == 0.0f)
    {
        upsample = original;
    }
    else
    {
        vec3 bloom = texture(bloomImage, texCoord).rgb;
        upsample = original + bloom * (intensity / 10.0f);
    }

    //Tome mapping + gamma fix
    upsample = vec3(1.0) - exp(-upsample * exposure);
    upsample = pow(upsample, vec3(1.0 / gamma));
}