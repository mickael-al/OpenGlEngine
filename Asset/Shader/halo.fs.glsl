#version 450 core

in vec2 texCoord;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_colorBuffer;
layout(binding = 1) uniform sampler2D u_depthBuffer;

uniform vec3 planetPos;
uniform float atmosphereRadius;
uniform mat4 invProjection;
uniform mat4 invView;
uniform vec3 cameraPos;
uniform vec2 cameraData; // x = near, y = far

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    float near = cameraData.x;
    float far = cameraData.y;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

float RaySphereIntersection(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float radius, out float t0, out float t1)
{
    vec3 oc = rayOrigin - sphereCenter;
    float b = dot(rayDir, oc);
    float c = dot(oc, oc) - radius * radius;
    float h = b * b - c;
    if (h < 0.0)
    {
        return -1.0;
    }
    h = sqrt(h);
    t0 = -b - h;
    t1 = -b + h;
    return t1 - t0;
}


vec3 AtmosphereColor(float factor)
{
    vec3 inner = vec3(0.3, 0.05, 0.02);   // rouge foncé/brun (base rocheuse)
    vec3 mid = vec3(0.8, 0.2, 0.05);    // orange-rouge chaud
    vec3 outer = vec3(1.0, 0.7, 0.4);     // lueur rose-dorée

    if (factor < 0.5)
    {
        return mix(inner, mid, factor * 2.0);
    }
    else
    {
        return mix(mid, outer, (factor - 0.5) * 2.0);
    }
}

void main()
{
    vec4 sceneColor = texture(u_colorBuffer, texCoord);
    float depth = texture(u_depthBuffer, texCoord).r;

    // Reconstruct view ray
    vec2 ndc = texCoord * 2.0 - 1.0; // [-1,1]
    vec4 clipPos = vec4(ndc, 1.0, 1.0);
    vec4 viewPos = invProjection * clipPos;
    viewPos /= viewPos.w;
    vec3 rayDir = normalize((invView * vec4(viewPos.xyz, 0.0)).xyz);

    // Compute intersection with atmospheric shell
    float t0, t1;
    float segmentLength = RaySphereIntersection(cameraPos, rayDir, planetPos, atmosphereRadius, t0, t1);
    t0 = max(t0, 0.0);
    if (segmentLength < 0.0) 
    {
        FragColor = sceneColor;
        return;
    }

    float ld = LinearizeDepth(depth);
    
    vec3 lightDir = vec3(1, 0, 0.3);
    vec3 hitPos = cameraPos + rayDir * t0;
    vec3 normal = normalize(hitPos - planetPos);
    float lightIntensity = clamp(dot(normal, normalize(lightDir)), 0.0, 1.0);
    lightIntensity = pow(lightIntensity, 1.5); // pour un dégradé plus doux
   
    /*
    ld = distance entre la camera et l'objet
    t0 = distance du points d'entres de la sphere
    t1 = distance du points de sortie
    segmentLength = distance entre le point d'entrer et de sortie
    */

    t1 = min(t1,ld);
    segmentLength = t1 - t0;
    float densityFactor = clamp(segmentLength / (atmosphereRadius*2), 0.0, 1.0);
    vec3 haloColor = AtmosphereColor(densityFactor);
    

    vec3 litHalo = haloColor * pow(densityFactor, 4.0) * lightIntensity;
    vec3 finalColor = sceneColor.rgb + litHalo;

    FragColor = vec4(finalColor, 1.0);

}
