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
uniform float angleOffset;
uniform float cellDensity;

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

    float voronoi = (Voronoi3D(pos, angleOffset, cellDensity) / 1.4) * 2.0 - 1.0;
#if OPERATOR == EQUAL
    data[index] = voronoi * heightMult;
#elif OPERATOR == ADD
    data[index] += voronoi * heightMult;
#elif OPERATOR == SUB
    data[index] -= voronoi * heightMult;
#elif OPERATOR == MULT
    data[index] *= voronoi * heightMult;
#endif
}
