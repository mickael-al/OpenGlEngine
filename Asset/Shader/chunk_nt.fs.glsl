#version 450

layout(std430, binding = 0) buffer UniformBufferCamera
{
    vec3 camPos;
    mat4 view;
    mat4 proj;
} ubc;

layout(std430, binding = 3) buffer UniformBufferDivers
{
    int maxLight;
    float u_time;
    float gamma;
    float ambiant;
    float fov;
    bool ortho;
} ubd;

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


struct TextureUnit
{
    vec4 color;         // rgb + .a normal strength
    vec4 data;          // ratio = roughness, metallic, occlusion, emit
};

struct BiomePart
{
    TextureUnit up;
    TextureUnit side;
};

struct Biome
{
    BiomePart main;
    BiomePart secondary;
    int noiseActive;
    float noiseScale;
    vec2 _padding; // align to 16 bytes
};

layout(std430, binding = 2) buffer BufferTextureBiome
{
    Biome biomeData[];
};

const uint MATRIX_SIZE = 5u;
const float INTERPOLATION_EPSILON = 0.01;
const float EPSIT = 0.01;

struct PbrData
{
    vec3 Color;//color = rgba
    vec4 gOther;//metallic roughness ao emit
};

vec3 getTriplanarBlend(vec3 normal)
{
    vec3 blend = abs(normal);
    blend = max(blend, 0.00001);
    float sum = blend.x + blend.y + blend.z;
    return blend / sum;
}

float samplePerlin(vec3 wp, vec3 blend)
{
    return perlin(wp.zy) * blend.x + perlin(wp.xz) * blend.y + perlin(wp.xy) * blend.z;
}

vec4 sampleValue4(vec4 up, vec4 side, vec3 blend)
{
    return side * blend.x + up * blend.y + side * blend.z;
}
vec3 sampleValue3(vec3 up, vec3 side, vec3 blend)
{
    return side * blend.x + up * blend.y + side * blend.z;
}
vec2 sampleValue2(vec2 up, vec2 side, vec3 blend)
{
    return side * blend.x + up * blend.y + side * blend.z;
}
float sampleValue(float up, float side, vec3 blend)
{
    return side * blend.x + up * blend.y + side * blend.z;
}

PbrData interpolate(PbrData a, PbrData b, float f) 
{
    return PbrData(
        mix(a.Color, b.Color, f),
        mix(a.gOther, b.gOther, f)
    );
}


PbrData sampleBiomePartUnit(BiomePart bp, vec3 worldPos, vec3 blend)
{
    PbrData pd;
    vec4 d = sampleValue4(bp.up.data, bp.side.data, blend);
    pd.Color = sampleValue3(bp.up.color.rgb, bp.side.color.rgb, blend);
    pd.gOther.x = d.y;
    pd.gOther.y = d.x;
    pd.gOther.z = d.z;
    pd.gOther.w = d.w;// roughness, metallic, occlusion, emit
    return pd;
}

float contrast(float value, float factor, float pivot)
{
    return (value - pivot) * factor + pivot;
}

const PbrData getMatrix(uvec2 p, vec3 worldPos, vec3 blend)
{
    PbrData pd;
    Biome b = biomeData[p.x + p.y * 5];    


    if (b.noiseActive != -1)
    {
        float n = clamp(samplePerlin(worldPos * b.noiseScale, blend)+0.25f, 0, 1);
        float r = 1.0 - n;
        if (n > EPSIT && r > EPSIT)
        {
            pd = interpolate(sampleBiomePartUnit(b.main, worldPos, blend), sampleBiomePartUnit(b.secondary, worldPos, blend), n);
        }
        else if (r <= EPSIT)
        {
            pd = sampleBiomePartUnit(b.secondary, worldPos, blend);
        }
        else
        {
            pd = sampleBiomePartUnit(b.main, worldPos, blend);
        }
    }
    else
    {
        pd = sampleBiomePartUnit(b.main, worldPos, blend);
    }

    return pd;
}

PbrData bilinearMatrixSample(vec2 uv, vec3 worldPos, vec3 normal) 
{
    vec2 gridPos = uv * float(MATRIX_SIZE - 1);
    vec2 f = fract(gridPos);
    uvec2 p0 = uvec2(gridPos);
    uvec2 p1 = min(p0 + uvec2(1), MATRIX_SIZE - 1);

    vec3 blend = getTriplanarBlend(normal);

    PbrData m00 = getMatrix(p0, worldPos, blend);
    PbrData m10 = getMatrix(uvec2(p1.x, p0.y), worldPos, blend);
    PbrData m01 = getMatrix(uvec2(p0.x, p1.y), worldPos, blend);
    PbrData m11 = getMatrix(p1, worldPos, blend);

    float w00 = (1.0 - f.x) * (1.0 - f.y);
    float w10 = f.x * (1.0 - f.y);
    float w01 = (1.0 - f.x) * f.y;
    float w11 = f.x * f.y;

    PbrData result;
    result.Color = m00.Color * w00 + m10.Color * w10 + m01.Color * w01 + m11.Color * w11;
    result.gOther = m00.gOther * w00 + m10.gOther * w10 + m01.gOther * w01 + m11.gOther * w11;

    return result;
}

layout(location = 0) in vec4 ViewPos;
layout(location = 1) in vec4 Normal;
layout(location = 2) in vec3 WorldPos;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gColor;
layout(location = 3) out vec3 gOther;

void main(void)
{
    vec2 uv = vec2(ViewPos.w, Normal.w);
    PbrData pd = bilinearMatrixSample(uv, WorldPos * CSCALE, Normal.xyz);

    gPosition = ViewPos.xyz;
    gNormal = vec4(Normal.xyz, pd.gOther.w);//normal = rgb + emit = a    
    gColor = vec4(pd.Color, 1.0);//color = rgba
    gOther = pd.gOther.xyz;//metallic = r + roughness = g + ao = b
}