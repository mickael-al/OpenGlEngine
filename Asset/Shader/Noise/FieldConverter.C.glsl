#version 450

#define GROUP_SIZE 8



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

/// <PERLIN3D>

float fade(float t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

uint permute(uint x)
{
    return ((34u * x + 1u) * x) % 289u;
}

vec3 gradient3D(uint hash)
{
    hash = hash % 12u;
    const vec3[12] grad3 = vec3[12](
        vec3(1, 1, 0), vec3(-1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0),
        vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
        vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, 1, -1), vec3(0, -1, -1)
        );
    return grad3[int(hash)];
}

float grad_dot(uint hash, vec3 p)
{
    vec3 g = gradient3D(hash);
    return dot(g, p);
}

// Fonction de Perlin 3D
float perlin3D(vec3 p)
{
    ivec3 i = ivec3(floor(p));
    vec3 f = fract(p);

    uint a = permute(uint(i.x));
    uint aa = permute(a + uint(i.y));
    uint ab = permute(a + uint(i.y + 1));
    uint b = permute(uint(i.x + 1));
    uint ba = permute(b + uint(i.y));
    uint bb = permute(b + uint(i.y + 1));

    uint aaa = permute(aa + uint(i.z));
    uint aba = permute(ab + uint(i.z));
    uint baa = permute(ba + uint(i.z));
    uint bba = permute(bb + uint(i.z));

    uint aab = permute(aa + uint(i.z + 1));
    uint abb = permute(ab + uint(i.z + 1));
    uint bab = permute(ba + uint(i.z + 1));
    uint bbb = permute(bb + uint(i.z + 1));

    vec3 u = vec3(fade(f.x), fade(f.y), fade(f.z));

    float x1 = mix(grad_dot(aaa, f - vec3(0, 0, 0)), grad_dot(baa, f - vec3(1, 0, 0)), u.x);
    float x2 = mix(grad_dot(aba, f - vec3(0, 1, 0)), grad_dot(bba, f - vec3(1, 1, 0)), u.x);
    float y1 = mix(x1, x2, u.y);

    float x3 = mix(grad_dot(aab, f - vec3(0, 0, 1)), grad_dot(bab, f - vec3(1, 0, 1)), u.x);
    float x4 = mix(grad_dot(abb, f - vec3(0, 1, 1)), grad_dot(bbb, f - vec3(1, 1, 1)), u.x);
    float y2 = mix(x3, x4, u.y);

    return mix(y1, y2, u.z);
}
/// </PERLIN3D>
/// <GRADIENT3D>
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
/// </GRADIENT3D>
/// <SIMPLEX3D>
float noise_randomValue(vec3 p) {
    return fract(sin(dot(p, vec3(12.9898, 78.233, 37.719))) * 43758.5453);
}

float noise_interpolate(float a, float b, float t) {
    return mix(a, b, t);
}

float unity_valueNoise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f); // smoothstep

    // 8 corners of the cube
    float r000 = noise_randomValue(i + vec3(0.0, 0.0, 0.0));
    float r100 = noise_randomValue(i + vec3(1.0, 0.0, 0.0));
    float r010 = noise_randomValue(i + vec3(0.0, 1.0, 0.0));
    float r110 = noise_randomValue(i + vec3(1.0, 1.0, 0.0));
    float r001 = noise_randomValue(i + vec3(0.0, 0.0, 1.0));
    float r101 = noise_randomValue(i + vec3(1.0, 0.0, 1.0));
    float r011 = noise_randomValue(i + vec3(0.0, 1.0, 1.0));
    float r111 = noise_randomValue(i + vec3(1.0, 1.0, 1.0));

    // Trilinear interpolation
    float x00 = noise_interpolate(r000, r100, f.x);
    float x10 = noise_interpolate(r010, r110, f.x);
    float x01 = noise_interpolate(r001, r101, f.x);
    float x11 = noise_interpolate(r011, r111, f.x);

    float y0 = noise_interpolate(x00, x10, f.y);
    float y1 = noise_interpolate(x01, x11, f.y);

    float z = noise_interpolate(y0, y1, f.z);
    return z;
}

float SimpleNoise(vec3 p) {
    float t = 0.0;

    float freq = pow(2.0, 0.0);
    float amp = pow(0.5, 3.0 - 0.0);
    t += unity_valueNoise(p / freq) * amp;

    freq = pow(2.0, 1.0);
    amp = pow(0.5, 3.0 - 1.0);
    t += unity_valueNoise(p / freq) * amp;

    freq = pow(2.0, 2.0);
    amp = pow(0.5, 3.0 - 2.0);
    t += unity_valueNoise(p / freq) * amp;

    return t * 2.0 - 1.0;
}
/// </SIMPLEX3D>
/// <VORONOI3D>
vec3 voronoi_noise_randomVector3(vec3 pos, float offset)
{
    // Matrice 3x3 pour variation pseudo-aléatoire
    mat3 m = mat3(
        15.27, 47.63, 88.42,
        99.41, 89.98, 67.31,
        12.13, 73.57, 42.67
    );
    pos = fract(sin(m * pos) * 46839.32);
    return vec3(
        sin(pos.x * offset) * 0.5 + 0.5,
        cos(pos.y * offset) * 0.5 + 0.5,
        sin(pos.z * offset) * 0.5 + 0.5
    );
}

float Voronoi3D(vec3 pos, float angleOffset, float cellDensity)
{
    vec3 g = floor(pos * cellDensity);
    vec3 f = fract(pos * cellDensity);
    float minDist = 8.0;
    //vec3 cellCoord = vec3(0.0);

    for (int z = -1; z <= 1; z++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                vec3 lattice = vec3(float(x), float(y), float(z));
                vec3 offset = voronoi_noise_randomVector3(lattice + g, angleOffset);
                float d = distance(lattice + offset, f);
                if (d < minDist)
                {
                    minDist = d;
                    //cellCoord = offset;
                }
            }
        }
    }

    return minDist;
}
/// </VORONOI3D>

float applyHeightTransition(float value, float fy, float minY, float maxY, float trp) 
{
    float tr = clamp(trp,0.0,1.0) * 0.5 * (maxY - minY);
    float v = clamp(value, -1, 1);
    if (fy <= minY)
    {
        return 1.0; // Solide
    }
    else if (fy >= maxY) 
    {
        return -1.0; // Vide
    }
    else if (fy > minY && fy < minY + tr) 
    {
        float t = (fy - minY) / tr;
        return mix(1.0, v, t);
    }
    else if (fy < maxY && fy > maxY - tr)
    {
        float t = (maxY- fy) / tr;
        return mix(-1.0, v, t);
    }
    return v; // Pas de modification
}

#define HEIGHT_BASE 10000
const uint MATRIX_SIZE = 5u;

vec2 foretTempere(vec3 uvw)//2-2
{    
    return vec2(perlin3D(uvw*0.01), 0.8f);
}

vec2 desertSec(vec3 uvw)//2-2
{
    return vec2(0, 1.0f);
}

vec2 desertRoche(vec3 uvw)//tres Sec 0-4
{
    return vec2(perlin3D(uvw * 0.025), 0.8f);
}


vec2 getMatrix(vec3 uvw, uvec3 p)
{
    // Exemple de sélection de valeur selon les indices.
    // Tu peux ajouter ta logique selon la case ici.
    // p.x = humidité, p.y = chaleur, p.z = hauteur

    if (p.x == 0) {
        // très sec
        return desertRoche(uvw);
    }
    else if (p.x == 1) {
        return desertSec(uvw);
    }
    else if (p.x == 2) {
        return foretTempere(uvw);
    }
    else if (p.x == 3) {
        // humide
    }
    else if (p.x == 4) {
        // très humide
    }

    return vec2(0, 1.0f);
}

vec2 trilinearMatrixSample(vec3 buvw, float humidity, float heat, float height)
{
    vec3 uvw = clamp(vec3(humidity, heat, height), 0.0, 1.0);
    vec3 gridPos = uvw * float(MATRIX_SIZE - 1);

    uvec3 p0 = uvec3(clamp(floor(gridPos), 0.0, float(MATRIX_SIZE - 1)));
    uvec3 p1 = min(p0 + uvec3(1), uvec3(MATRIX_SIZE - 1));

    vec3 f = fract(gridPos);

    // Récupération des 8 coins du cube
    vec2 c000 = getMatrix(buvw, uvec3(p0.x, p0.y, p0.z));
    vec2 c100 = getMatrix(buvw, uvec3(p1.x, p0.y, p0.z));
    vec2 c010 = getMatrix(buvw, uvec3(p0.x, p1.y, p0.z));
    vec2 c110 = getMatrix(buvw, uvec3(p1.x, p1.y, p0.z));
    vec2 c001 = getMatrix(buvw, uvec3(p0.x, p0.y, p1.z));
    vec2 c101 = getMatrix(buvw, uvec3(p1.x, p0.y, p1.z));
    vec2 c011 = getMatrix(buvw, uvec3(p0.x, p1.y, p1.z));
    vec2 c111 = getMatrix(buvw, uvec3(p1.x, p1.y, p1.z));

    // Interpolation trilinéaire
    vec2 c00 = mix(c000, c100, f.x);
    vec2 c10 = mix(c010, c110, f.x);
    vec2 c01 = mix(c001, c101, f.x);
    vec2 c11 = mix(c011, c111, f.x);

    vec2 c0 = mix(c00, c10, f.y);
    vec2 c1 = mix(c01, c11, f.y);

    return mix(c0, c1, f.z);
}


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

layout(std430, binding = 4) buffer FieldValue3D
{
    float data3D[];
};

uniform uint length;
uniform float chunkScale;
uniform vec3 offset;

layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE, local_size_z = GROUP_SIZE) in;
void main()
{
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;
    uint z = gl_GlobalInvocationID.z;

    if (x >= length || y >= length || z >= length)
    {
        return;
    }

    uint index3d = x + y * length + z * length * length;
    uint index2d = x + z * length;
    vec3 uvw = (vec3(x, y, z) * chunkScale + offset);

    float hd = height_data[index2d];
    vec2 field3DResult = trilinearMatrixSample(uvw, humidity_data[index2d], heat_data[index2d], (hd+HEIGHT_BASE)/(HEIGHT_BASE*2));
    float field2DResult = (hd - offset.y);
    float smd = smooth_data[index2d]*0.5;

    data3D[index3d] = applyHeightTransition(field3DResult.x, y * chunkScale, field2DResult - smd, field2DResult + smd, field3DResult.y);
}
