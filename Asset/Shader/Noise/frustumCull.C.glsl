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
    uint visible = 0;
    for (uint i = 0; i < 8; ++i)
    {
        vec4 clip = viewProjMatrix * vec4(corners[i & 1].x, corners[(i >> 1) & 1].y, corners[(i >> 2) & 1].z, 1.0);
#if PROJECTION_MODE == PERSPECTIVE
        if (abs(clip.x) <= clip.w && abs(clip.y) <= clip.w && clip.z >= 0.0 && clip.z <= clip.w)
        {
            visible = 1;
            break;
        }
#elif PROJECTION_MODE == ORTHOGRAPHIC
        vec3 ndc = clip.xyz / clip.w;
        if (abs(ndc.x) <= 1.0 &&
            abs(ndc.y) <= 1.0 &&
            abs(ndc.z) <= 1.0)
        {
            visible = 1;
            break;
        }
#endif
    }

    commands[id].instanceCount = visible;
}
