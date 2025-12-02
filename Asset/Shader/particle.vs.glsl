#version 460 core

layout(std430, binding = 0) buffer UniformBufferCamera
{
    vec3 camPos;
    mat4 view;
    mat4 invView;
    mat4 proj;
} ubc;

layout(std430, binding = 6) buffer ParticleData
{
    float data[];
} particle;

uniform uint u_offset;
uniform uint u_offsetSettings;   // offset des particulesGroups
uniform uint u_count;

out VS_OUT 
{
    vec2 uv;
    vec3 worldPos;
    vec4 color;
} vs_out;


vec3 ReadPosition(uint id)
{
    uint bid = u_offset + id * 8u;
    return vec3(
        particle.data[bid + 0],
        particle.data[bid + 1],
        particle.data[bid + 2]
    );
}

float ReadScale(uint id)
{
    uint bid = u_offset + id * 8u;
    uint packed1 = floatBitsToUint(particle.data[bid + 6]);
    uint scale16 = packed1 & 0xFFFFu;
    return (float(scale16) / 65535.0) * 64.0;
}

vec3 ReadVelocity(uint id)
{
    uint bid = u_offset + id * 8u;

    uint packed1 = floatBitsToUint(particle.data[bid + 6]);
    uint packed2 = floatBitsToUint(particle.data[bid + 7]);

    uint vx16 = packed1 >> 16;
    uint vy16 = packed2 & 0xFFFFu;
    uint vz16 = packed2 >> 16;
    vec3 vel;
    vel.x = (float(vx16) / 32767.5) - 1.0;
    vel.y = (float(vy16) / 32767.5) - 1.0;
    vel.z = (float(vz16) / 32767.5) - 1.0;

    return vel * 256.0;
}


vec4 ReadColor(uint id)
{
    uint base = u_offset + id * 8u;

    uint packed1 = floatBitsToUint(particle.data[base + 3]);
    uint packed2 = floatBitsToUint(particle.data[base + 4]);

    float r = float(packed1 & 0xFFFFu) / 65535.0;
    float g = float((packed1 >> 16) & 0xFFFFu) / 65535.0;
    float b = float(packed2 & 0xFFFFu) / 65535.0;
    float a = float((packed2 >> 16) & 0xFFFFu) / 65535.0;

    return vec4(r, g, b, a) * 64.0; // si ta range est 0..64
}

const uint FLAG_ATTACH_TO_EMIT_POINT = 1u << 0;
const uint FLAG_BOX_EMIT = 1u << 1;
const uint FLAG_FOLLOW_TARGET = 1u << 2;
const uint FLAG_ROTATE = 1u << 3;
const uint FLAG_VEL_ORTIENTATION = 1u << 4;

void main()
{
    uint particleID = gl_VertexID / 3u;
    int localVertex = gl_VertexID % 3;

    if (particleID >= u_count)
    {
        return;
    }

    vec3 pos = ReadPosition(particleID);
    float scale = ReadScale(particleID);
    vs_out.color = ReadColor(particleID);
    if (scale <= 0 || scale >= particle.data[u_offsetSettings + 13])
    {
        return;
    }

    vec3 camRight = vec3(ubc.invView[0]);
    vec3 camUp = vec3(ubc.invView[1]);
    uint flags = floatBitsToUint(particle.data[u_offsetSettings + 9]);
    if ((flags & FLAG_VEL_ORTIENTATION) != 0u)
    {
        vec3 vel = ReadVelocity(particleID);
        if (length(vel) < 0.0001)
        {
            vel = vec3(0, 1, 0);
        }

        vec3 dir = normalize(vel);
        vec3 worldUp = vec3(1.0, 0.0, 0.0);
        /*if (abs(dot(dir, worldUp)) > 0.95)
        {
            worldUp = vec3(0.0, 1.0, 0.0);
        }*/

        camRight = normalize(cross(worldUp, dir));
        camUp = normalize(cross(dir, camRight));
    }

    if ((flags & FLAG_ATTACH_TO_EMIT_POINT) != 0u)
    {
        pos += vec3(particle.data[u_offsetSettings + 2], particle.data[u_offsetSettings + 3], particle.data[u_offsetSettings + 4]);
    }
    vec2 corn[3] =
    {
        vec2(-1.0, -1.0), // bas gauche
        vec2(3.0, -1.0), // bas droite
        vec2(-1.0,  3.0)  // haut gauche
    };

    vec2 corner = corn[localVertex];
    vec3 offset = (camRight * corner.x + camUp * corner.y) * scale;

    vec3 finalPos = pos + offset;

    vs_out.worldPos = finalPos;
    vs_out.uv = corner * 0.5 + 0.5;

    gl_Position = ubc.proj * ubc.view * vec4(finalPos, 1.0);
}
