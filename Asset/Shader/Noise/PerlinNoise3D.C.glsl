#version 450

#define GROUP_SIZE 8

//#define OPERATOR 
#define EQUAL 0
#define ADD 1
#define SUB 2
#define MULT 3

//#define OUPUT_MODE
#define UNIQUE_BUFFER 0
#define MATRIX_BUFFER 1

layout(std430, binding = 0) buffer FieldValue 
{
    float data[];
};

#if OUPUT_MODE == MATRIX_BUFFER
uniform uint offsetMatrix;
#endif
uniform uint length;
uniform float heightMult;
uniform float scale;
uniform vec3 offset;

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

    uint index = x + y * length + z * length * length;
#if OUPUT_MODE == MATRIX_BUFFER
    index += offsetMatrix;
#endif
    vec3 pos = (vec3(x, y, z) + offset) * scale;

#if OPERATOR == EQUAL
    data[index] = perlin3D(pos) * heightMult;
#elif OPERATOR == ADD
    data[index] += perlin3D(pos) * heightMult;
#elif OPERATOR == SUB
    data[index] -= perlin3D(pos) * heightMult;
#elif OPERATOR == MULT
    data[index] *= perlin3D(pos) * heightMult;
#endif
}
