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
    data[index] = SimpleNoise(pos) * heightMult;
#elif OPERATOR == ADD
    data[index] += SimpleNoise(pos) * heightMult;
#elif OPERATOR == SUB
    data[index] -= SimpleNoise(pos) * heightMult;
#elif OPERATOR == MULT
    data[index] *= SimpleNoise(pos) * heightMult;
#endif
}
