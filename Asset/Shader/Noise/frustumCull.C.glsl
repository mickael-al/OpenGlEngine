#version 450

#define GROUP_SIZE 256
#define MAX_DIM_GROUPS 256
#define MAX_DIM_THREADS (GROUP_SIZE * MAX_DIM_GROUPS)
#define MAX_DIM_THREADS_THREADS (MAX_DIM_THREADS * MAX_DIM_GROUPS)

#define PERSPECTIVE 0
#define ORTHOGRAPHIC 1

layout(std430, binding = 1) buffer BufferChunk
{
    float bc[];
};

struct DrawElementsIndirectCommand {
    uint count;
    uint instanceCount;
    uint firstIndex;
    uint baseVertex;
    uint baseInstance;
};

layout(std430, binding = 2) buffer DrawCommands
{
    DrawElementsIndirectCommand commands[];
};

uniform uint sizeElts;
uniform mat4 viewProjMatrix;

layout(local_size_x = GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint id = uint(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * MAX_DIM_THREADS + gl_GlobalInvocationID.z * MAX_DIM_THREADS_THREADS);
    if (id >= sizeElts)
    {
        return;
    }
    vec3 minPos = vec3(bc[id * 3], bc[id * 3 + 1], bc[id * 3 + 2]);
    vec3 corners[2] = vec3[](minPos,minPos + vec3(CHUNK_SIZE));

    vec4 clip;
    vec3 normal;
    float distance;
    for (uint j = 0; j < 6; j++)
    {
        uint col = j >> 1;
        if ((j & 1) == 0)
        {
            normal.x = viewProjMatrix[0][3] + viewProjMatrix[0][col];
            normal.y = viewProjMatrix[1][3] + viewProjMatrix[1][col];
            normal.z = viewProjMatrix[2][3] + viewProjMatrix[2][col];
            distance = viewProjMatrix[3][3] + viewProjMatrix[3][col];
        }
        else
        {
            normal.x = viewProjMatrix[0][3] - viewProjMatrix[0][col];
            normal.y = viewProjMatrix[1][3] - viewProjMatrix[1][col];
            normal.z = viewProjMatrix[2][3] - viewProjMatrix[2][col];
            distance = viewProjMatrix[3][3] - viewProjMatrix[3][col];
        }
        float lengthN = length(normal);
        normal /= lengthN;
        distance /= lengthN;
        uint outCount = 0;
        for (uint i = 0; i < 8; ++i)
        {            
            if (dot(normal, vec3(corners[i & 1].x, corners[(i >> 1) & 1].y, corners[(i >> 2) & 1].z)) + distance < 0.0)
            {
                outCount++;
            }
        }
        if (outCount == 8) 
        {
            commands[id].instanceCount = 0;
            return; // tous les points hors plan -> cull
        }
    }
    commands[id].instanceCount = 1;
}
