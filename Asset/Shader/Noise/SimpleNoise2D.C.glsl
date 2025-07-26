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
    data[index] = SimpleNoise(uv) * heightMult;
#elif OPERATOR == ADD
    data[index] += SimpleNoise(uv) * heightMult;
#elif OPERATOR == SUB
    data[index] -= SimpleNoise(uv) * heightMult;
#elif OPERATOR == MULT
    data[index] *= SimpleNoise(uv) * heightMult;
#endif
}