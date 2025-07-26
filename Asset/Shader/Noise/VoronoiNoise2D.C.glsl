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
    vec2 uv = (vec2(x, y) + offset.xz) * scale;

#if OPERATOR == EQUAL
    data[index] = Voronoi_float(uv, angleOffset, cellDensity) * heightMult;
#elif OPERATOR == ADD
    data[index] += Voronoi_float(uv, angleOffset, cellDensity) * heightMult;
#elif OPERATOR == SUB
    data[index] -= Voronoi_float(uv, angleOffset, cellDensity) * heightMult;
#elif OPERATOR == MULT
    data[index] *= Voronoi_float(uv, angleOffset, cellDensity) * heightMult;
#endif
}
