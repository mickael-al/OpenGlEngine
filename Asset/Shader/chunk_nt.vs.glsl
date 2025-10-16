#version 460

layout(std430, binding = 0) buffer UniformBufferCamera
{
	vec3 camPos;
	mat4 view;
	mat4 proj;
} ubc;

layout(std430, binding = 1) buffer BufferChunk
{
	float data[];
} bc;

layout(std430, binding = 3) buffer UniformBufferDivers
{
	int maxLight;
	float u_time;
	float gamma;
	float ambiant;
	float fov;
	bool ortho;
} ubd;

vec3 decodeOctahedral(vec2 e)
{
	vec3 v = vec3(e.x, 1.0 - abs(e.x) - abs(e.y), e.y);
	if (v.y < 0.0)
	{
		v.xz = (1.0 - abs(v.zx)) * vec2(v.x >= 0.0 ? 1.0 : -1.0, v.z >= 0.0 ? 1.0 : -1.0);
		v.y = -v.y;
	}
	return normalize(v);
}

vec3 DecodeNormal11_11_10(uint pac)
{
	float xp = float(pac & 0x7FFu);
	float yp = float((pac >> 11) & 0x7FFu);
	float zp = float((pac >> 22) & 0x3FFu);

	xp = (xp / 2047.0) * 2.0 - 1.0;
	yp = (yp / 2047.0) * 2.0 - 1.0;
	zp = (zp / 1023.0) * 2.0 - 1.0;

	return normalize(vec3(xp, yp, zp));
}

layout(location = 0) in uint a_Pos0; // x (16b) | y (16b)
layout(location = 1) in uint a_Pos1; // z (16b) | humidity (8b) | heat (8b)
layout(location = 2) in uint a_Pos2; // encoded Normal

layout(location = 0) out vec4 ViewPos;
layout(location = 1) out vec4 Normal;
layout(location = 2) out vec3 WorldPos;

void main()
{
	const float divPos = (1.0 / 65535.0) * CHUNK_SIZE;
	uint bid = gl_BaseInstance * 3;
	vec3 offsetPos = vec3(bc.data[bid], bc.data[bid + 1], bc.data[bid + 2]);

	uint x = a_Pos0 & 0xFFFFu;
	uint y = (a_Pos0 >> 16u) & 0xFFFFu;
	uint z = a_Pos1 & 0xFFFFu;
	vec3 localPos = vec3(x, y, z) * divPos;

	ViewPos.w = float((a_Pos1 >> 16u) & 0xFFu) / 255.0;
	Normal.w = float((a_Pos1 >> 24u) & 0xFFu) / 255.0;
	Normal.xyz = DecodeNormal11_11_10(a_Pos2);

	vec4 wp = vec4(localPos + offsetPos, 1.0);
	WorldPos = wp.xyz;
	wp = ubc.view * wp;
	ViewPos.xyz = wp.xyz;
	gl_Position = ubc.proj * wp;
}
