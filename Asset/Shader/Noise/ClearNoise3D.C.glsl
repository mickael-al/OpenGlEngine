#version 450

#define GROUP_SIZE 8

//#define OUPUT_MODE
#define UNIQUE_BUFFER 0
#define MATRIX_BUFFER 1

layout(std430, binding = 0) buffer FieldValue
{
    float data[];
};

layout(std140, binding = 1) buffer NoiseParams
{
    uint width;
    uint depth; // ignored (should be 1)
    uint height;
};

#if OUPUT_MODE == MATRIX_BUFFER
uniform uint offsetMatrix;
#endif  

uniform uint length;

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
    data[index] = 0.0;
}
