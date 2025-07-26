#version 450

#define GROUP_SIZE 16

//#define OPERATOR 
#define EQUAL 0
#define ADD 1
#define SUB 2
#define MULT 3

//#define OUPUT_MODE
#define UNIQUE_BUFFER 0
#define MATRIX_BUFFER 1

//#define ZERO_ONE_MODE 
#define MINUS_ONE_ONE 0
#define ZERO_ONE 1

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
uniform float chunkScale;
uniform vec3 offset;

// Fonction de permutation (hash) simple
uint permute(uint x)
{
    return ((34u * x + 1u) * x) % 289u;
}

// Gradient vector hash : génère un vecteur gradient pseudo-aléatoire unitaire à partir d'un entier
vec2 gradient(uint hash)
{
    // On génère un angle pseudo-aléatoire à partir du hash
    float angle = float(hash % 256u) * 6.2831853 / 256.0; // [0, 2pi[
    return vec2(cos(angle), sin(angle));
}

// Fonction de fade (courbe d'interpolation de Perlin)
float fade(float t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

// Produit scalaire du gradient
float grad_dot(uint hash, vec2 pos)
{
    vec2 g = gradient(hash);
    return dot(g, pos);
}

float toUnsigned(float x) //-1,1 -> 0,1
{
    return (x + 1.0) * 0.5;
}

float perlin(vec2 uv)
{
    // Cellule de la grille
    ivec2 i = ivec2(floor(uv));
    vec2 f = fract(uv);

    // Hash pour chaque coin de la cellule
    uint aa = permute(uint(i.x) + permute(uint(i.y)));
    uint ab = permute(uint(i.x) + permute(uint(i.y + 1)));
    uint ba = permute(uint(i.x + 1) + permute(uint(i.y)));
    uint bb = permute(uint(i.x + 1) + permute(uint(i.y + 1)));

    // Calcul produit scalaire entre vecteurs gradients et vecteurs position
    float dot_aa = grad_dot(aa, f - vec2(0.0, 0.0));
    float dot_ba = grad_dot(ba, f - vec2(1.0, 0.0));
    float dot_ab = grad_dot(ab, f - vec2(0.0, 1.0));
    float dot_bb = grad_dot(bb, f - vec2(1.0, 1.0));

    // Courbe d'interpolation
    vec2 u = vec2(fade(f.x), fade(f.y));

    // Interpolation bilinéaire
    float lerp_x1 = mix(dot_aa, dot_ba, u.x);
    float lerp_x2 = mix(dot_ab, dot_bb, u.x);
    float lerp_y = mix(lerp_x1, lerp_x2, u.y);
    
#if ZERO_ONE_MODE == ZERO_ONE
    return toUnsigned(lerp_y);
#else
    // On retourne une valeur normalisée approximativement dans [-1, 1]
    return lerp_y;
#endif    
}

//glDispatchCompute((width + 15) / 16, (height + 15) / 16, 1);
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
#if OUPUT_MODE == MATRIX_BUFFER
    index += offsetMatrix;
#endif
    vec2 uv = (vec2(x, y)* chunkScale + offset.xz) * scale;
#if OPERATOR == EQUAL
    data[index] = perlin(uv) * heightMult;
#elif OPERATOR == ADD
    data[index] += perlin(uv) * heightMult;
#elif OPERATOR == SUB
    data[index] -= perlin(uv) * heightMult;
#elif OPERATOR == MULT
    data[index] *= perlin(uv) * heightMult;
#endif
}
