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
    data[index] = gradientNoise(pos) * heightMult;
#elif OPERATOR == ADD
    data[index] += gradientNoise(pos) * heightMult;
#elif OPERATOR == SUB
    data[index] -= gradientNoise(pos) * heightMult;
#elif OPERATOR == MULT
    data[index] *= gradientNoise(pos) * heightMult;
#endif
}
