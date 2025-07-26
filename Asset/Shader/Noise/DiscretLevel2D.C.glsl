#version 450

#define GROUP_SIZE 16

//#define OUPUT_MODE
#define UNIQUE_BUFFER 0
#define MATRIX_BUFFER 1

layout(std430, binding = 0) buffer FieldValue
{
    float data[];
};

float discretize(float x, int levels) 
{
    return floor(x * float(levels)) / float(levels - 1);
}

/*
float concave(float x) {
    return sqrt(x);
}
float concave2(float x) {
    return pow(x, 0.5); // même que sqrt(x)
}
float convex(float x) {
    return x * x;
}
float convex2(float x) {
    return pow(x, 2.0);
}
float smoothStepCurve(float x) {
    return smoothstep(0.0, 1.0, x);
}
float sineEase(float x) {
    return 0.5 - 0.5 * cos(x * 3.141592);
}
*/
#if OUPUT_MODE == MATRIX_BUFFER
uniform uint offsetMatrix;
#endif  
uniform uint length;
uniform float scale;
uniform vec3 offset;
uniform int discretLevel;

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

    data[index] = discretize(data[index], discretLevel);
}
