#version 450

#define GROUP_SIZE 256
#define MAX_DIM_GROUPS 256
#define MAX_DIM_THREADS (GROUP_SIZE * MAX_DIM_GROUPS)
#define MAX_DIM_THREADS_THREADS (MAX_DIM_THREADS * MAX_DIM_GROUPS)

layout(local_size_x = GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer ParticleData
{
    float data[];
} particle;

uniform uint u_offset;   // offset dans le SSBO
uniform uint u_count;    // nombre de particules

void main()
{
    uint id = uint(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * MAX_DIM_THREADS +gl_GlobalInvocationID.z * MAX_DIM_THREADS_THREADS);

    if (id >= u_count)
    {
        return;
    }

    /*
        glm::vec3 position;
        uint16_t cr;
        uint16_t cg;
        uint16_t cb;
        uint16_t ca;
        float time;
        uint16_t scale;
        uint16_t vx;
        uint16_t vy;
        uint16_t vz;
    */

    uint bid = u_offset + id * 8u;
    particle.data[bid + 0] = 0;
    particle.data[bid + 1] = 0;
    particle.data[bid + 2] = 0;
    particle.data[bid + 3] = uintBitsToFloat(0x04000400u);
    particle.data[bid + 4] = uintBitsToFloat(0x04000400u);
    particle.data[bid + 5] = -1.0f;
    uint scale16 = 0u;         // zero scale
    uint vm16 = 32768u;     // zero velocity

    uint packed1 = (scale16) | (vm16 << 16);
    uint packed2 = (vm16) | (vm16 << 16);

    particle.data[bid + 6] = uintBitsToFloat(packed1);
    particle.data[bid + 7] = uintBitsToFloat(packed2);
}
