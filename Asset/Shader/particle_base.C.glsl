#version 450

#define GROUP_SIZE 256
#define MAX_DIM_GROUPS 256
#define MAX_DIM_THREADS (GROUP_SIZE * MAX_DIM_GROUPS)
#define MAX_DIM_THREADS_THREADS (MAX_DIM_THREADS * MAX_DIM_GROUPS)

#extension GL_NV_shader_atomic_float : require

layout(local_size_x = GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 6) buffer ParticleData
{
    float data[];
} particle;

uniform uint u_offset;   // offset des particules
uniform uint u_offsetSettings;   // offset des particulesGroups
uniform uint u_count;    // nombre de particules
uniform float u_dt;      // delta time

vec3 ReadPosition(uint id)
{
    uint bid = u_offset + id * 8u;
    return vec3(
        particle.data[bid + 0],
        particle.data[bid + 1],
        particle.data[bid + 2]
    );
}

void WritePosition(uint id, vec3 p)
{
    uint bid = u_offset + id * 8u;
    particle.data[bid + 0] = p.x;
    particle.data[bid + 1] = p.y;
    particle.data[bid + 2] = p.z;
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

void WriteColor(uint id, vec4 color)
{
    uint base = u_offset + id * 8u;

    uint r = uint(clamp(color.r / 64.0, 0.0, 1.0) * 65535.0);
    uint g = uint(clamp(color.g / 64.0, 0.0, 1.0) * 65535.0);
    uint b = uint(clamp(color.b / 64.0, 0.0, 1.0) * 65535.0);
    uint a = uint(clamp(color.a / 64.0, 0.0, 1.0) * 65535.0);

    uint packed1 = (r) | (g << 16);
    uint packed2 = (b) | (a << 16);

    particle.data[base + 3] = uintBitsToFloat(packed1);
    particle.data[base + 4] = uintBitsToFloat(packed2);
}

float ReadTime(uint id)
{
    uint bid = u_offset + id * 8u;
    return particle.data[bid + 5];
}

void WriteTime(uint id, float t)
{
    uint bid = u_offset + id * 8u;
    particle.data[bid + 5] = t;
}

void ReadScaleVelocity(uint id, out float scale, out vec3 vel)
{
    uint bid = u_offset + id * 8u;

    uint packed1 = floatBitsToUint(particle.data[bid + 6]);
    uint packed2 = floatBitsToUint(particle.data[bid + 7]);

    uint scale16 = packed1 & 0xFFFFu;
    scale = (float(scale16) / 65535.0) * 64.0;

    uint vx16 = packed1 >> 16;
    uint vy16 = packed2 & 0xFFFFu;
    uint vz16 = packed2 >> 16;

    vel.x = (float(vx16) / 32767.5) - 1.0;
    vel.y = (float(vy16) / 32767.5) - 1.0;
    vel.z = (float(vz16) / 32767.5) - 1.0;

    vel *= 256.0;
}

uint EncodeVelocity16(float v)
{
    float n = clamp(v / 256.0, -1.0, 1.0);
    return uint((n + 1.0) * 32767.5);
}

void WriteScaleVelocity(uint id, float scale, vec3 vel)
{
    uint bid = u_offset + id * 8u;
    uint scale16 = uint(clamp(scale / 64.0, 0.0, 1.0) * 65535.0);

    uint vx16 = EncodeVelocity16(vel.x);
    uint vy16 = EncodeVelocity16(vel.y);
    uint vz16 = EncodeVelocity16(vel.z);

    uint packed1 = (scale16) | (vx16 << 16);
    uint packed2 = (vy16) | (vz16 << 16);

    particle.data[bid + 6] = uintBitsToFloat(packed1);
    particle.data[bid + 7] = uintBitsToFloat(packed2);
}

float hash(uint x)
{
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return float(x) * (1.0 / 4294967295.0);
}

float rand(inout uint seed)
{
    seed += (seed << 10u);
    seed ^= (seed >> 6u);
    seed += (seed << 3u);
    seed ^= (seed >> 11u);
    seed += (seed << 15u);
    return hash(seed);
}

vec3 randomDirectionInCone(inout uint seed, vec3 axis, float angleDeg)
{
    axis = normalize(axis);
    float angleRad = radians(angleDeg);

    float cosMax = cos(angleRad);

    float u = rand(seed);
    float v = rand(seed);

    float cosTheta = mix(1.0, cosMax, u);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float phi = 6.28318530718 * v;

    vec3 localDir = vec3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta
    );

    vec3 up = abs(axis.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
    vec3 tangent = normalize(cross(up, axis));
    vec3 bitangent = cross(axis, tangent);

    return tangent * localDir.x +
        bitangent * localDir.y +
        axis * localDir.z;
}

vec3 RotateAroundLocalY(vec3 dir, float angleDeg)
{
    vec3 forward = normalize(dir);
    const vec3 ref = vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(forward, ref));
    if (length(tangent) < 0.0001)
    {
        tangent = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    }    
    vec3 localY = normalize(cross(tangent, forward));
    
    vec3 X = tangent;
    vec3 Y = localY; 
    vec3 Z = forward;

    float a = radians(angleDeg);
    float c = cos(a);
    float s = sin(a);

    return normalize(X * c + Z * s);
}

const uint FLAG_ATTACH_TO_EMIT_POINT = 1u << 0;
const uint FLAG_BOX_EMIT = 1u << 1;
const uint FLAG_FOLLOW_TARGET = 1u << 2;
const uint FLAG_ROTATE = 1u << 3;
const uint FLAG_VEL_ORTIENTATION = 1u << 4;

void main()
{
    uint id = uint(gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * MAX_DIM_THREADS +gl_GlobalInvocationID.z * MAX_DIM_THREADS_THREADS);
    
    if (id >= u_count)
    {
        return;
    }
    
    uint seed = u_offsetSettings + id + u_offset;
    float rlt = rand(seed);
    float rro = rand(seed);
    float rsro = rand(seed);

    uint sid = u_offsetSettings;
    uint flags = floatBitsToUint(particle.data[sid + 9]);
    float lifetime = ReadTime(id);
    if (lifetime < 0)
    {            
        float rv = rand(seed);
        float rs = rand(seed);

        WriteTime(id, mix(particle.data[sid], particle.data[sid + 1], rlt));         
        vec3 emitDirection = vec3(particle.data[sid + 5], particle.data[sid + 6], particle.data[sid + 7]);
        float spreadAngle = particle.data[sid + 8];   
        float scaleStart = particle.data[sid + 12];
        float scaleEnd = particle.data[sid + 13];
        vec3 emitScaleMin = vec3(particle.data[sid + 14],particle.data[sid + 15], particle.data[sid + 16]);
        vec3 emitScaleMax = vec3(particle.data[sid + 17], particle.data[sid + 18], particle.data[sid + 19]);

        vec3 emitPosition;
        if ((flags & FLAG_ATTACH_TO_EMIT_POINT) != 0u)
        {
            emitPosition = vec3(0, 0, 0);
        }
        else
        {
            emitPosition = vec3(particle.data[sid + 2], particle.data[sid + 3], particle.data[sid + 4]);
        }
        if ((flags & FLAG_BOX_EMIT) != 0u)
        {
            float x = mix(emitScaleMin.x, emitScaleMax.x, rand(seed));
            float y = mix(emitScaleMin.y, emitScaleMax.y, rand(seed));
            float z = mix(emitScaleMin.z, emitScaleMax.z, rand(seed));
            emitPosition += vec3(x, y, z);
        }
        vec3 dir = randomDirectionInCone(seed, emitDirection, spreadAngle);
        vec3 vel = mix(particle.data[sid + 10], particle.data[sid + 11], rv)* dir;
        float sc = mix(scaleStart, scaleEnd, rs);
        vec4 c = vec4(particle.data[sid + 31], particle.data[sid + 32], particle.data[sid + 33], particle.data[sid + 34]);

        WritePosition(id, emitPosition);
        WriteScaleVelocity(id, sc, vel);
        WriteColor(id, c);

        return;
    }   
    
    lifetime -= u_dt; 
    WriteTime(id, lifetime);    
    
    float maxLifeTime = mix(particle.data[sid], particle.data[sid + 1], rlt);
    float lifeRatio = clamp(1.0 - (lifetime / maxLifeTime), 0.0, 1.0);
    
    float scale;
    vec3 vel;
    ReadScaleVelocity(id, scale, vel);
    
    vec3 gravity = vec3(particle.data[sid + 20], particle.data[sid + 21], particle.data[sid + 22]);
    vec3 externalForce = vec3(particle.data[sid + 23], particle.data[sid + 24], particle.data[sid + 25]);
    float drag = particle.data[sid + 26];

    vel += (gravity + externalForce) * u_dt;
    vel *= (1.0 - drag * u_dt);

    vec3 pos = ReadPosition(id);
    if ((flags & FLAG_FOLLOW_TARGET) != 0u)
    {
        vec3 tempPos = pos;
        if ((flags & FLAG_ATTACH_TO_EMIT_POINT) != 0u)
        {
            tempPos += vec3(particle.data[sid + 2], particle.data[sid + 3], particle.data[sid + 4]);
        }
        float strengthFollow = particle.data[sid + 42];
        float radiusFollow = particle.data[sid + 43];
        vec3 target = vec3(particle.data[sid + 39], particle.data[sid + 40], particle.data[sid + 41]);
        if (distance(target, tempPos) < radiusFollow)
        {
            float speed = length(vel);
            vec3 newDir = normalize(mix(normalize(vel), normalize(target - tempPos), strengthFollow));
            vel = newDir * speed;
        }
    }

    pos += vel * u_dt;
    if ((flags & FLAG_ROTATE) != 0u)
    {
        float scaleRotate = mix(particle.data[sid + 29], particle.data[sid + 30], rsro);
        vec3 rdir = RotateAroundLocalY(normalize(vel), mix(particle.data[sid + 27], particle.data[sid + 28], rro) * lifeRatio) * scaleRotate;
        pos += rdir * u_dt;
    }
    WritePosition(id, pos);

    float newScale = mix(particle.data[sid + 12], particle.data[sid + 13], lifeRatio);
    WriteScaleVelocity(id, newScale, vel);

    vec4 colorStart = vec4(particle.data[sid + 31], particle.data[sid + 32], particle.data[sid + 33], particle.data[sid + 34]);
    vec4 colorEnd = vec4(particle.data[sid + 35], particle.data[sid + 36], particle.data[sid + 37], particle.data[sid + 38]);
    vec4 c = mix(colorStart, colorEnd, lifeRatio);
    WriteColor(id, c);
}
