#version 450 core

in vec2 texCoord;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_colorBuffer;
layout(binding = 1) uniform sampler2D u_depthBuffer;

uniform vec3 color;
uniform float time;
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


vec3 gradientNoise_dir(vec3 p)
{
    p = mod(p, 289.0);
    float x = mod((34.0 * p.x + 1.0) * p.x, 289.0) + p.y;
    x = mod((34.0 * x + 1.0) * x, 289.0) + p.z;
    x = fract(x / 41.0) * 2.0 - 1.0;

    // Convert scalar x to a normalized 3D vector
    // Map to hemisphere using a simple hash to pseudo-random vec3
    return normalize(vec3(
        x - floor(x + 0.5),
        abs(x) - 0.5,
        cos(x * 3.1415)
    ));
}

float gradientNoise(vec3 p)
{
    vec3 ip = floor(p);
    vec3 fp = fract(p);

    // Get gradients at the 8 cube corners
    float d000 = dot(gradientNoise_dir(ip + vec3(0, 0, 0)), fp - vec3(0, 0, 0));
    float d100 = dot(gradientNoise_dir(ip + vec3(1, 0, 0)), fp - vec3(1, 0, 0));
    float d010 = dot(gradientNoise_dir(ip + vec3(0, 1, 0)), fp - vec3(0, 1, 0));
    float d110 = dot(gradientNoise_dir(ip + vec3(1, 1, 0)), fp - vec3(1, 1, 0));
    float d001 = dot(gradientNoise_dir(ip + vec3(0, 0, 1)), fp - vec3(0, 0, 1));
    float d101 = dot(gradientNoise_dir(ip + vec3(1, 0, 1)), fp - vec3(1, 0, 1));
    float d011 = dot(gradientNoise_dir(ip + vec3(0, 1, 1)), fp - vec3(0, 1, 1));
    float d111 = dot(gradientNoise_dir(ip + vec3(1, 1, 1)), fp - vec3(1, 1, 1));

    // Fade curve
    vec3 u = fp * fp * fp * (fp * (fp * 6.0 - 15.0) + 10.0);

    // Interpolate
    float x00 = mix(d000, d100, u.x);
    float x10 = mix(d010, d110, u.x);
    float x01 = mix(d001, d101, u.x);
    float x11 = mix(d011, d111, u.x);

    float y0 = mix(x00, x10, u.y);
    float y1 = mix(x01, x11, u.y);

    float result = mix(y0, y1, u.z);

    // Remap to [0, 1] if desired, or keep as [-1, 1]
    return result;
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
    float ld = LinearizeDepth(depth);
  
    float t = 0.0;                          // distance depuis la caméra
    float tMax = ld;                        // distance de coupure : profondeur de la scène
    float stepSize = 1.0;                   // pas d'échantillonnage (1m)
    int maxSteps = 256;                     // sécurité anti boucle infinie

    vec3 rayOrigin = cameraPos;
    vec3 accumColor = vec3(0.0);
    float accumAlpha = 0.0;

    for (int i = 0; i < maxSteps && t < tMax && accumAlpha < 0.99; ++i) 
    {
        vec3 samplePos = rayOrigin + rayDir * t;

        // Animation du volume avec le temps
        vec3 noisePos = samplePos * 0.25 + vec3(0.0, 0.0, time); // Échelle + mouvement
        float density = gradientNoise(noisePos*0.05); // [-1,1]
        density = clamp(density, 0.0, 1.0);

        float alpha = density * 0.05; // scale pour éviter saturation

        vec3 col = color;

        float weight = (1.0 - accumAlpha) * alpha;
        accumColor += col * weight;
        accumAlpha += weight;

        t += stepSize;
    }

    // Composition finale
    vec3 finalColor = mix(sceneColor.rgb, accumColor, accumAlpha);
    FragColor = vec4(finalColor, 1.0);
}
