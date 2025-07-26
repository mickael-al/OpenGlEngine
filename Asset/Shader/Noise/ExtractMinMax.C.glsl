#version 450

#define GROUP_SIZE 16

layout(std430, binding = 0) readonly buffer FieldValueHeight
{
    float dh[];
};

layout(std430, binding = 1) readonly buffer FieldValueSmooth
{
    float ds[];
};

layout(std430, binding = 2) buffer MinMaxBuffer 
{
    int minValue;
    int maxValue;
};

uniform uint length;
uniform float heightMult;

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

    int orderedMax = int(((dh[index] + ds[index] * 0.5) / heightMult) * 2147483647.0f);
    int orderedMin = int(((dh[index] - ds[index] * 0.5) / heightMult) * 2147483647.0f);

    atomicMin(minValue, orderedMin);
    atomicMax(maxValue, orderedMax);
}
