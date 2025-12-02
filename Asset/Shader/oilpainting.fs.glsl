#version 450

layout(binding = 0) uniform sampler2D u_image;
uniform float u_radius;  // Taille du pinceau
uniform vec2 u_resolution;
uniform float u_time;

vec3 oilPaintEffect(vec2 uv)
{
    float radius = u_radius;
    int sampleCount = int(radius) * 2 + 1;

    const int bins = 24;
    int histogram[bins];
    vec3 averageColor[bins];

    for (int i = 0; i < bins; ++i) 
    {
        histogram[i] = 0;
        averageColor[i] = vec3(0.0);
    }

    for (int y = -int(radius); y <= int(radius); ++y) 
    {
        for (int x = -int(radius); x <= int(radius); ++x) 
        {
            vec2 offset = vec2(float(x), float(y)) / u_resolution;
            vec3 col = texture2D(u_image, uv + offset).rgb;
            float intensity = dot(col, vec3(0.299, 0.587, 0.114));  // Luminance
            int bin = int(floor(intensity * float(bins - 1)));

            histogram[bin]++;
            averageColor[bin] += col;
        }
    }

    int maxBin = 0;
    for (int i = 1; i < bins; ++i) 
    {
        if (histogram[i] > histogram[maxBin]) 
        {
            maxBin = i;
        }
    }

    return averageColor[maxBin] / float(histogram[maxBin]);
}

/*float edgeDetect(vec2 uv) {
    float dx = 1.0 / u_resolution.x;
    float dy = 1.0 / u_resolution.y;

    mat3 Gx = mat3(-1, 0, 1,
        -2, 0, 2,
        -1, 0, 1);

    mat3 Gy = mat3(-1, -2, -1,
        0, 0, 0,
        1, 2, 1);

    float sumX = 0.0;
    float sumY = 0.0;

    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            vec2 offset = vec2(i * dx, j * dy);
            float intensity = dot(texture(u_image, uv + offset).rgb, vec3(0.299, 0.587, 0.114));
            sumX += intensity * Gx[j + 1][i + 1];
            sumY += intensity * Gy[j + 1][i + 1];
        }
    }

    return length(vec2(sumX, sumY));
}

// Ajout de hachures avec bruit pseudo-aléatoire pour l'effet crayonné
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}*/

in vec2 texCoord;
layout(location = 0) out vec3 fragColor;
/*void main()
{
float edge = edgeDetect(texCoord);

    // Amplifie les bords
    edge = smoothstep(0.1, 0.3, edge);

    // Ajoute un motif "hachuré" avec du bruit
    float noise = hash(floor(texCoord * u_resolution * 1.5));
    float sketch = step(noise, edge);

    // Inverse le résultat pour obtenir un fond clair et lignes sombres
    fragColor = vec3(sketch);
}
*/

void main()
{
   fragColor = oilPaintEffect(texCoord);
}


float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// --- Bruit doux ---
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

// --- Flou directionnel (simule diffusion d'encre) ---
vec3 blurredColor(vec2 uv, float radius) {
    vec3 color = vec3(0.0);
    float total = 0.0;

    for (float x = -radius; x <= radius; x++) {
        for (float y = -radius; y <= radius; y++) {
            vec2 offset = vec2(x, y) / u_resolution;
            float weight = exp(-(x * x + y * y) / (2.0 * radius));
            color += texture(u_image, uv + offset).rgb * weight;
            total += weight;
        }
    }
    return color / total;
}

// --- Détection douce des bords ---
float softEdges(vec2 uv) {
    float dx = 1.0 / u_resolution.x;
    float dy = 1.0 / u_resolution.y;

    vec3 center = texture(u_image, uv).rgb;
    float edge = 0.0;

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 offset = vec2(x * dx, y * dy);
            vec3 sa = texture(u_image, uv + offset).rgb;
            edge += distance(center, sa);
        }
    }
    return clamp(edge * 2.0, 0.0, 1.0); // amplification du contraste
}
/*
void main() {
    // Appliquer un flou doux
    vec3 blurred = blurredColor(texCoord, 2.5);

    // Mélanger avec la couleur originale
    vec3 original = texture(u_image, texCoord).rgb;
    vec3 mixColor = mix(original, blurred, 0.6);

    // Ajouter du bruit pour simuler le papier
    float grain = noise(texCoord * u_resolution * 0.8 + u_time * 10.1) * 0.07;
    mixColor += grain;

    // Atténuer les bords pour un effet aquarelle
    float edge = softEdges(texCoord);
    mixColor *= 1.0 - edge * 0.5;

    fragColor = mixColor;
}*/