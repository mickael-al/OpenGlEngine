#version 450

vec2 gradientNoise_dir(vec2 p)
{
    p = mod(p, 289.0);  // % becomes mod for floats
    float x = mod((34.0 * p.x + 1.0) * p.x, 289.0) + p.y;
    x = mod((34.0 * x + 1.0) * x, 289.0);
    x = fract(x / 41.0) * 2.0 - 1.0;
    return normalize(vec2(x - floor(x + 0.5), abs(x) - 0.5));
}

float gradientNoise(vec2 p)//[0-1]
{
    vec2 ip = floor(p);
    vec2 fp = fract(p);
    float d00 = dot(gradientNoise_dir(ip), fp);
    float d01 = dot(gradientNoise_dir(ip + vec2(0.0, 1.0)), fp - vec2(0.0, 1.0));
    float d10 = dot(gradientNoise_dir(ip + vec2(1.0, 0.0)), fp - vec2(1.0, 0.0));
    float d11 = dot(gradientNoise_dir(ip + vec2(1.0, 1.0)), fp - vec2(1.0, 1.0));

    fp = fp * fp * fp * (fp * (fp * 6.0 - 15.0) + 10.0);
    return mix(mix(d00, d01, fp.y), mix(d10, d11, fp.y), fp.x) + 0.5;
}

uint permute(uint x)
{
    return ((34u * x + 1u) * x) % 289u;
}

vec2 gradient(uint hash)
{
    float angle = float(hash % 256u) * 6.2831853 / 256.0; // [0, 2pi[
    return vec2(cos(angle), sin(angle));
}

float fade(float t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float grad_dot(uint hash, vec2 pos)
{
    vec2 g = gradient(hash);
    return dot(g, pos);
}

float perlin(vec2 uv)
{
    ivec2 i = ivec2(floor(uv));
    vec2 f = fract(uv);

    uint aa = permute(uint(i.x) + permute(uint(i.y)));
    uint ab = permute(uint(i.x) + permute(uint(i.y + 1)));
    uint ba = permute(uint(i.x + 1) + permute(uint(i.y)));
    uint bb = permute(uint(i.x + 1) + permute(uint(i.y + 1)));

    float dot_aa = grad_dot(aa, f - vec2(0.0, 0.0));
    float dot_ba = grad_dot(ba, f - vec2(1.0, 0.0));
    float dot_ab = grad_dot(ab, f - vec2(0.0, 1.0));
    float dot_bb = grad_dot(bb, f - vec2(1.0, 1.0));

    vec2 u = vec2(fade(f.x), fade(f.y));

    float lerp_x1 = mix(dot_aa, dot_ba, u.x);
    float lerp_x2 = mix(dot_ab, dot_bb, u.x);
    float lerp_y = mix(lerp_x1, lerp_x2, u.y);

    return lerp_y;
}

float noise_randomValue(vec2 uv)
{
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise_interpolate(float a, float b, float t)
{
    return mix(a, b, t); // (1.0 - t)*a + t*b
}

float unity_valueNoise(vec2 uv)
{
    vec2 i = floor(uv);
    vec2 f = fract(uv);
    f = f * f * (3.0 - 2.0 * f);

    vec2 c0 = i + vec2(0.0, 0.0);
    vec2 c1 = i + vec2(1.0, 0.0);
    vec2 c2 = i + vec2(0.0, 1.0);
    vec2 c3 = i + vec2(1.0, 1.0);
    float r0 = noise_randomValue(c0);
    float r1 = noise_randomValue(c1);
    float r2 = noise_randomValue(c2);
    float r3 = noise_randomValue(c3);

    float bottomOfGrid = noise_interpolate(r0, r1, f.x);
    float topOfGrid = noise_interpolate(r2, r3, f.x);
    float t = noise_interpolate(bottomOfGrid, topOfGrid, f.y);
    return t;
}

float SimpleNoise(vec2 UV)
{
    float t = 0.0;

    float freq = pow(2.0, 0.0);
    float amp = pow(0.5, 3.0 - 0.0);
    t += unity_valueNoise(UV / freq) * amp;

    freq = pow(2.0, 1.0);
    amp = pow(0.5, 3.0 - 1.0);
    t += unity_valueNoise(UV / freq) * amp;

    freq = pow(2.0, 2.0);
    amp = pow(0.5, 3.0 - 2.0);
    t += unity_valueNoise(UV / freq) * amp;

    return t;
}

vec2 voronoi_noise_randomVector(vec2 UV, float offset)
{
    // Matrice 2x2 pour variation pseudo-aléatoire
    mat2 m = mat2(15.27, 47.63, 99.41, 89.98);
    UV = fract(sin(UV * m) * 46839.32);
    return vec2(sin(UV.y * offset) * 0.5 + 0.5, cos(UV.x * offset) * 0.5 + 0.5);
}

float Voronoi_float(vec2 UV, float AngleOffset, float CellDensity)
{
    vec2 g = floor(UV * CellDensity);
    vec2 f = fract(UV * CellDensity);
    float minDist = 8.0;
    //vec2 cellCoord = vec2(0.0);

    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            vec2 lattice = vec2(float(x), float(y));
            vec2 offset = voronoi_noise_randomVector(lattice + g, AngleOffset);
            float d = distance(lattice + offset, f);
            if (d < minDist)
            {
                minDist = d;
                // cellCoord = offset;
            }
        }
    }

    return minDist;
}

float toSigned(float x) //0,1 -> -1,1
{
    return x * 2.0 - 1.0;
}

float toUnsigned(float x) //-1,1 -> 0,1
{
    return (x + 1.0) * 0.5;
}

float discretize(float x, int levels)
{
    return floor(x * float(levels)) / float(levels - 1);
}

float smoothDiscretize(float value, float levels, float transition)
{
    float scaled = value * levels;
    float base = floor(scaled);
    float frac = scaled - base;

    // Clamp transition pour éviter les erreurs (0.0 = step dur, 1.0 = très doux)
    transition = clamp(transition, 0.0, 1.0);

    // Détermine la plage d'interpolation en fonction du pourcentage
    float t0 = 0.5 * (1.0 - transition);
    float t1 = 0.5 * (1.0 + transition);

    // Interpolation douce entre les niveaux
    float smoothv = smoothstep(t0, t1, frac);

    return (base + smoothv) / levels;
}

float contrast(float value, float factor, float pivot)
{
    return (value - pivot) * factor + pivot;
}

float concave(float x) {
    return sqrt(x);
}
float concave2(float x) {
    return pow(x, 0.5); // même que sqrt(x)
}
float convex(float x) {
    return x * x;
}
float convex2(float x) {
    return pow(x, 2.0);
}
float smoothStepCurve(float x) {
    return smoothstep(0.0, 1.0, x);
}
float sineEase(float x) {
    return 0.5 - 0.5 * cos(x * 3.141592);
}

float hashFloat(uint x) {
    x ^= x >> 17;
    x *= 0xed5ad4bbu;
    x ^= x >> 11;
    x *= 0xac4c1b51u;
    x ^= x >> 15;
    x *= 0x31848babu;
    x ^= x >> 14u;
    return float(x) / 4294967295.0; // UINT32_MAX
}
vec3 hashVec3(uint seed) {
    float x = hashFloat(seed);
    float y = hashFloat(seed + 1u);
    float z = hashFloat(seed + 2u);
    return vec3(x, y, z) * 2.0 - 1.0;
}

//float gradientNoise(vec2 p)//[0-1]
//float perlin(vec2 uv)//
//float SimpleNoise(vec2 UV)//
//float Voronoi_float(vec2 UV, float AngleOffset, float CellDensity)//[0-1]
//float toSigned(float x) //0,1 -> -1,1
//float toUnsigned(float x) //-1,1 -> 0,1

#define HEIGHT_BASE 10000
#define HEIGHT_SMOOTH_BASE 32
#define HEIGHT_MIN_SMOOTH_BASE 16

#define HEIGHT_BASE_NOISE_SCALE 0.00001
#define SMOOTH_BASE_NOISE_SCALE 0.00001
//#define HEAT_BASE_NOISE_SCALE 0.0001
//#define HUMIDITY_BASE_NOISE_SCALE 0.0001
#define HEAT_BASE_NOISE_SCALE 0.00025
#define HUMIDITY_BASE_NOISE_SCALE 0.00025

#define HEIGHT_SEED_OFFSET 50000.0
#define HUMIDITY_SEED_OFFSET 25000.0
#define HEAT_SEED_OFFSET 95000.0
#define SMOOTH_SEED_OFFSET 10000.0

#define BIOME_SIZE 5.0
#define GROUP_SIZE 16

layout(std430, binding = 0) buffer FieldVHeight
{
    float height_data[];
};

layout(std430, binding = 1) buffer FieldVHeat
{
    float heat_data[];
};

layout(std430, binding = 2) buffer FieldVHumidity
{
    float humidity_data[];
};

layout(std430, binding = 3) buffer FieldVSmooth
{
    float smooth_data[];
};

layout(std430, binding = 4) buffer MinMaxBuffer
{
    int minValue;
    int maxValue;
};

uniform uint seed;
uniform uint length;

uniform float chunkScale;
uniform vec3 offset;


vec2 foretTempere(vec2 uv)//2-2
{
    //float scalefactor = abs(gradientNoise(uv * 0.005));
    float scalefactor = abs(Voronoi_float(uv * 0.0025,45,1.0f));

    float baseScale = 1.0f * scalefactor;
    float noise = mix(gradientNoise(uv * 0.008)*5.5, gradientNoise(uv * 0.0025), clamp(contrast(baseScale, 2.5f, 0.2f), 0, 1));
    float sm = gradientNoise(vec2(5000,-5500) + uv * 0.004);
    sm = clamp(contrast(sm, 1.5f, 0.5f),0.5,1.0);
    float terrainShape = smoothDiscretize(noise, 5.0* gradientNoise(uv * 0.00025), sm); // 5 niveaux discrets lissés
    return vec2(terrainShape * 35.0,200);
}

vec2 desertSec(vec2 uv)//rouge 1-4
{
    float noise = Voronoi_float(uv * 0.008,45,1.0f);
    noise += gradientNoise(uv * 0.016) * 0.25;
    noise += gradientNoise(uv * 0.003) * 0.5;
    return vec2(noise*35.0, 0);
}

vec2 desertRoche(vec2 uv)//tres Sec 0-4
{
    float maxn = 20.0f;
    float noise = clamp(contrast(gradientNoise(uv * 0.01),5.0f,0.5),0.0f,1.0f) * maxn;
    noise += gradientNoise(uv * 0.0016) * 1;
    noise += gradientNoise(uv * 0.0003) * 2;
    noise /= maxn;
    return vec2(noise * 1000.0, 0);
}

const uint MATRIX_SIZE = 5u;

vec2 getMatrix(vec2 uv, uvec2 p)
{
    if (p.x == 0)//tres sec
    {
        return desertRoche(uv);
    }
    else if (p.x == 1)//sec
    {
        return desertSec(uv);
    }
    else if (p.x == 2)//Modere
    {
        return foretTempere(uv);
    }
    else if (p.x == 3)//Humide
    {

    }
    else if (p.x == 4)//Tres Humide
    {

    }
    return vec2(0.0, 0.0);
}

vec2 bilinearMatrixSample(vec2 buv, float humidity, float heat)
{
    vec2 uv = clamp(vec2(humidity, heat), 0.0, 1.0);
    vec2 gridPos = uv * float(MATRIX_SIZE - 1);

    uvec2 p0 = uvec2(clamp(floor(gridPos), 0.0, float(MATRIX_SIZE - 1)));
    uvec2 p1 = uvec2(clamp(p0 + uvec2(1), uvec2(0), uvec2(MATRIX_SIZE - 1)));

    vec2 f = fract(gridPos);

    vec2 v00 = getMatrix(buv, p0);
    vec2 v10 = getMatrix(buv, uvec2(p1.x, p0.y));
    vec2 v01 = getMatrix(buv, uvec2(p0.x, p1.y));
    vec2 v11 = getMatrix(buv, p1);

    vec2 v0 = mix(v00, v10, f.x);
    vec2 v1 = mix(v01, v11, f.x);
    return mix(v0, v1, f.y);
}

float snapToNearestInt(float v, float strength)
{
    float nearest = round(v);
    return mix(v, nearest, strength);
}

layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE, local_size_z = 1) in;
void main()
{
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;

    if (x >= length || y >= length)
    {
        return;
    }

    uint index = x + y * length;
    vec2 uv = (vec2(x - 1.0, y - 1.0) * chunkScale + offset.xz);

    vec3 heightStepOffset = hashVec3(seed) * HEIGHT_SEED_OFFSET;
    vec3 smoothStepOffset = hashVec3(seed) * SMOOTH_SEED_OFFSET;
    vec3 humidityStepOffset = hashVec3(seed) * HUMIDITY_SEED_OFFSET;
    vec3 heatStepOffset = hashVec3(seed) * HEAT_SEED_OFFSET;

    float height = perlin((uv + heightStepOffset.xz) * HEIGHT_BASE_NOISE_SCALE) * HEIGHT_BASE;
    float smooths = perlin((uv + smoothStepOffset.xz) * SMOOTH_BASE_NOISE_SCALE) * (HEIGHT_SMOOTH_BASE - HEIGHT_MIN_SMOOTH_BASE);
    smooths += HEIGHT_MIN_SMOOTH_BASE;
    float humidity = gradientNoise((uv + humidityStepOffset.xz) * HUMIDITY_BASE_NOISE_SCALE);
    float heat = perlin((uv + heatStepOffset.xz) * HEAT_BASE_NOISE_SCALE);

    float steps = 5.0;
    float percent = 1.0;

    float h = humidity * steps;
    float base = floor(h);
    float frac = h - base;
    float smoothed = base + smoothstep(0.5 - percent, 0.5 + percent, frac);
    humidity = smoothed / steps;

    h = heat * steps;
    base = floor(h);
    frac = h - base;
    smoothed = base + smoothstep(0.5 - percent, 0.5 + percent, frac);
    heat = smoothed / steps;

    vec2 bms = bilinearMatrixSample(uv, humidity, heat);
    height += bms.x;
    smooths += bms.y;

    height_data[index] = height;
    smooth_data[index] = smooths;
    humidity_data[index] = humidity;
    heat_data[index] = heat;

    int orderedMax = int(((height + smooths * 0.5) / HEIGHT_BASE) * 2147483647.0f);
    int orderedMin = int(((height - smooths * 0.5) / HEIGHT_BASE) * 2147483647.0f);

    atomicMin(minValue, orderedMin);
    atomicMax(maxValue, orderedMax);
}