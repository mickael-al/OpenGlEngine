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

struct TextureUnit 
{
    ivec4 texIndices;   // color, normal, roughness, metallic
    vec4 color;         // rgb + .a = oclusion id
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
    int noiseIndices;
    float noiseScale;
    vec2 _padding; // align to 16 bytes
};

layout(std430, binding = 2) buffer BufferTextureBiome 
{
    Biome biomeData[];
};

layout(binding = 0) uniform sampler2DArray u_textureArray;

const float HEIGHT_BASE = 20.0;
const uint MATRIX_SIZE = 5u;
const float INTERPOLATION_EPSILON = 0.01;
const float EPSIT = 0.01;

struct PbrData
{
    vec4 Color;//color = rgba
    vec4 Normal;//normal = rgb / emit = a
    vec3 gOther;//metallic roughness ao
};

vec3 getTriplanarBlend(vec3 normal)
{
    vec3 blend = abs(normal);
    blend = max(blend, 0.00001);
    float sum = blend.x + blend.y + blend.z;
    return blend / sum;
}

float sampleTex(int up, int side, vec3 worldPos, vec3 blend, float def)
{
    float x, z;
    if (side != -1)
    {
        x = texture(u_textureArray, vec3(worldPos.zy, side)).r * def;
        z = texture(u_textureArray, vec3(worldPos.xy, side)).r * def;
    }
    else
    {
        x = def;
        z = def;
    }
    float y = up != -1 ? texture(u_textureArray, vec3(worldPos.xz, up)).r : def;

    return x * blend.x + y * blend.y + z * blend.z;
}

vec2 sampleTex2(int up, int side, vec3 worldPos, vec3 blend, vec2 def)
{
    vec2 x, z;
    if (side != -1)
    {
        x = texture(u_textureArray, vec3(worldPos.zy, side)).rg * def;
        z = texture(u_textureArray, vec3(worldPos.xy, side)).rg * def;
    }
    else
    {
        x = def;
        z = def;
    }
    vec2 y = up != -1 ? texture(u_textureArray, vec3(worldPos.xz, up)).rg : def;

    return x * blend.x + y * blend.y + z * blend.z;
}

vec3 sampleTexNormal(int up, int side, vec3 worldPos, vec3 blend)
{
    vec3 nx, nz;
    if (side != -1)
    {
        nx = texture(u_textureArray, vec3(worldPos.zy, side)).rgb * 2.0 - 1.0;
        nz = texture(u_textureArray, vec3(worldPos.xy, side)).rgb * 2.0 - 1.0;
    }
    else
    {
        nx = vec3(0, 0, 1.0);
        nz = vec3(0, 0, 1.0);
    }
    vec3 ny = up != -1 ? (texture(u_textureArray, vec3(worldPos.xz, up)).rgb * 2.0 - 1.0) : vec3(0, 0, 1.0);
 
    return normalize(nx * blend.x + ny * blend.y + nz * blend.z);
}

vec4 sampleTex4(int up, int side, vec3 worldPos, vec3 blend,vec4 def)
{
    vec4 x, z;
    if (side != -1)
    {
        x = texture(u_textureArray, vec3(worldPos.zy, side)) * def;
        z = texture(u_textureArray, vec3(worldPos.xy, side)) * def;
    }
    else
    {
        x = def;
        z = def;
    }
    vec4 y = up != -1 ? texture(u_textureArray, vec3(worldPos.xz, up)) : def;

    return x * blend.x + y * blend.y + z * blend.z;
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

PbrData interpolate(PbrData p1, PbrData p2, float f)
{
    PbrData pd;
    pd.Color = mix(p1.Color, p2.Color, f);
    pd.Normal = mix(p1.Normal, p2.Normal, f);
    pd.gOther = mix(p1.gOther, p2.gOther, f);
    return pd;
}

PbrData sampleBiomePartUnit(BiomePart bp,vec3 worldPos,vec3 blend)
{
    PbrData pd;
    int aoe = int(bp.up.color.a);
    vec4 d = sampleValue4(bp.up.data, bp.side.data, blend);
    vec2 aoeTex = sampleTex2(aoe, aoe, worldPos, blend, vec2(d.z, d.w));
    pd.Color = sampleTex4(bp.up.texIndices.x, bp.side.texIndices.x, worldPos, blend, vec4(sampleValue3(bp.up.color.rgb, bp.side.color.rgb, blend), 1.0f));
    pd.Normal = vec4(sampleTexNormal(bp.up.texIndices.y, bp.side.texIndices.y, worldPos, blend), aoeTex.y);
    pd.gOther.x = sampleTex(bp.up.texIndices.w, bp.side.texIndices.w, worldPos, blend, d.y);
    pd.gOther.y = sampleTex(bp.up.texIndices.z, bp.side.texIndices.z, worldPos, blend, d.x);
    pd.gOther.z = aoeTex.x;
    return pd;
}

const PbrData getMatrix(uint p, vec3 worldPos, vec3 blend)
{
    PbrData pd;
    Biome b = biomeData[p];    
    
    if (b.noiseIndices != -1)
    {
        float n = sampleTex(b.noiseIndices, b.noiseIndices, worldPos*b.noiseScale, blend,0.5);
        float r = 1.0 - n;
        if (n > EPSIT && r > EPSIT)
        {
            pd = interpolate(sampleBiomePartUnit(b.main, worldPos, blend), sampleBiomePartUnit(b.secondary, worldPos, blend),n);
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

layout(location = 0) in vec4 ViewPos;
layout(location = 1) in vec4 Normal;
layout(location = 2) in vec3 WorldPos;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gColor;
layout(location = 3) out vec3 gOther;

vec3 blendNormals(vec3 worldNormal, vec3 normalMapValue, float strength)
{    
    vec3 perturbed = normalize(mix(vec3(0.0, 0.0, 1.0), normalMapValue, strength));
    perturbed.x = -perturbed.x;
    perturbed.y = -perturbed.y;
    vec3 tangentX = normalize(vec3(worldNormal.y, -worldNormal.x, 0.0));
    vec3 tangentY = normalize(cross(worldNormal, tangentX));

    vec3 finalNormal = normalize(
        perturbed.x * tangentX +
        perturbed.y * tangentY +
        perturbed.z * worldNormal
    );

    return finalNormal;
}

void main(void)
{	
    /*gPosition = ViewPos.xyz;
    gNormal = vec4(Normal.xyz,0);
    //gColor = vec4(ViewPos.w, Normal.w, 0, 1);
    gColor = vec4(ViewPos.w, 0, 0, 1);
    gOther = vec3(0.5, 0.5, 0.5);
    return;*/
	vec3 uvw = vec3(ViewPos.w, Normal.w, (WorldPos.y+ HEIGHT_BASE) / (HEIGHT_BASE *2));
    vec3 blend = getTriplanarBlend(Normal.xyz);

    PbrData pd = getMatrix(0, WorldPos*0.25, blend);
	
	gPosition = ViewPos.xyz;	
    pd.Normal.rgb = blendNormals(Normal.xyz,pd.Normal.xyz,0.25f);
    gNormal = pd.Normal;//normal = rgb + emit = a    
    gColor = pd.Color;//color = rgba
    gOther = pd.gOther;//metallic = r + roughness = g + ao = b
}