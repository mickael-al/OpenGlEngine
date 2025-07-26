#version 450

#define GROUP_SIZE 16

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
    data[index] = 0.0;
}
