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
uniform float chunkScale;
uniform vec3 offset;

vec2 gradientNoise_dir(vec2 p)
{
    p = mod(p, 289.0);  // % becomes mod for floats
    float x = mod((34.0 * p.x + 1.0) * p.x, 289.0) + p.y;
    x = mod((34.0 * x + 1.0) * x, 289.0);
    x = fract(x / 41.0) * 2.0 - 1.0;
    return normalize(vec2(x - floor(x + 0.5), abs(x) - 0.5));
}

float gradientNoise(vec2 p)
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
    data[index] = gradientNoise(uv) * heightMult;
#elif OPERATOR == ADD
    data[index] += gradientNoise(uv) * heightMult;
#elif OPERATOR == SUB
    data[index] -= gradientNoise(uv) * heightMult;
#elif OPERATOR == MULT
    data[index] *= gradientNoise(uv) * heightMult;
#endif
}
