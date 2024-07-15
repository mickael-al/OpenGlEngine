#version 450

layout(binding = 0) uniform sampler2D originalImage;
layout(binding = 1) uniform sampler2D bloomImage;
uniform float exposure;
uniform float gamma;
uniform float intensity;

in vec2 texCoord;
layout(location = 0) out vec3 upsample;

void main()
{
    vec3 original = texture(originalImage, texCoord).rgb;
    
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