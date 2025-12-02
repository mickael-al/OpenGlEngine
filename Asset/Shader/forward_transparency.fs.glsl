#version 450

#define SHADOW_MAP_CASCADE_COUNT 4
#define SHADOW_MAP_CUBE_COUNT 6

const float PI = 3.14159265359;

layout(std430, binding = 0) buffer UniformBufferCamera
{
	vec3 camPos;
	mat4 view;
	mat4 invView;
	mat4 proj;
} ubc;

struct StructUBM
{
	vec4 albedo;
	vec2 offset;
	vec2 tilling;
	float metallic;
	float roughness;
	float normal;
	float ao;
	float emit;
};

layout(std430, binding = 2) buffer UniformBufferMaterials
{
	StructUBM ubm[];
} ubm;

layout(std430, binding = 3) buffer UniformBufferDivers
{
	int maxLight;
	float u_time;
	float gamma;
	float ambiant;
	float fov;
	bool ortho;
} ubd;

struct StructUBL
{
	vec3 position;
	vec3 color;
	vec3 direction;
	float range;
	float spotAngle;
	int status;
	int shadowID;
};

layout(std430, binding = 4) buffer UniformBufferLight
{
	StructUBL ubl[];
} ubl;

struct StructShadow
{
	mat4 projview;
	vec3 pos;
	float splitDepth;
};

layout(std430, binding = 5) buffer UniformBufferShadow
{
	StructShadow s[];
} ubs;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

const mat4 biasMat = mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);

layout(binding = 0) uniform sampler2D albedoTexture;
layout(binding = 1) uniform sampler2D normalTexture;
layout(binding = 2) uniform sampler2D metallicTexture;
layout(binding = 3) uniform sampler2D roughnessTexture;
layout(binding = 4) uniform sampler2D oclusionTexture;
layout(binding = 5) uniform sampler2DArray shadowDepth;
layout(binding = 6) uniform samplerCube irradianceMap;
layout(binding = 7) uniform samplerCube prefilterMap;
layout(binding = 8) uniform sampler2D brdfLUT;
layout(binding = 9) uniform sampler2D fragTexture;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 Color;
layout(location = 2) in vec3 WorldPos;
layout(location = 3) in mat3 TBN;
layout(location = 6) flat in int imaterial;

float calculateBias(vec4 shadowCoord, vec3 normal, vec3 lightDir)
{
	float bias = 0.005;
	float angle = max(dot(normal, lightDir), 0.0);
	return bias * tan(acos(angle));
}

float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex, float bias)
{
	float shadow = 1.0;

	if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
	{
		float dist = texture(shadowDepth, vec3(vec2(shadowCoord.st + offset), cascadeIndex)).r;
		if (shadowCoord.w > 0 && dist < shadowCoord.z - bias)
		{
			shadow = 0.0;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc, uint cascadeIndex, float bias)
{
	ivec3 texDim = textureSize(shadowDepth, 0);
	float scale = 0.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;

	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx * x, dy * y), cascadeIndex, bias);
			count++;
		}
	}
	return shadowFactor / count;
}

uniform vec2 u_invResolution;

out vec3 o_FragColor;

void main()
{	
	vec4 col = texture(albedoTexture, fragTexCoord) * ubm.ubm[imaterial].albedo * vec4(Color, 1.0);
	vec3 norm = texture(normalTexture, fragTexCoord).rgb;
	norm = mix(vec3(0.5, 0.5, 1.0), norm, ubm.ubm[imaterial].normal);
	norm = normalize(norm * 2.0 - 1.0);
	norm = normalize(TBN * norm);
	vec2 aoe = texture(oclusionTexture, fragTexCoord).rg;
	vec4 normal = vec4(norm, ubm.ubm[imaterial].emit * aoe.y);
	vec3 other = vec3(ubm.ubm[imaterial].metallic * texture(metallicTexture, fragTexCoord).r,
		ubm.ubm[imaterial].roughness * texture(roughnessTexture, fragTexCoord).r,
		ubm.ubm[imaterial].ao * aoe.x);

	vec3 viewPos = (ubc.view * vec4(WorldPos,1.0)).xyz;
	vec3 color = col.rgb;
	color = pow(color, vec3(ubd.gamma));

	vec3 V = normalize(ubc.camPos - WorldPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, color, other.x);
	if (normal.a <= 0)
	{
		// Reflectance equation
		vec3 Lo = vec3(0.0);
		for (int i = 0; i < ubd.maxLight; i++)
		{
			float shadow = 1.0;
			vec3 L = normalize(ubl.ubl[i].position - WorldPos);
			float distance = length(ubl.ubl[i].position - WorldPos);
			float attenuation = 1.0 / (distance * distance);
			vec3 radiance = ubl.ubl[i].color * attenuation * ubl.ubl[i].range;
			if (ubl.ubl[i].status == 0)
			{
				L = normalize(-ubl.ubl[i].direction);
			}
			vec3 H = normalize(V + L);

			// Cook-Torrance BRDF
			float NDF = DistributionGGX(normal.rgb, H, other.y);
			float G = GeometrySmith(normal.rgb, V, L, other.y);
			vec3 F = fresnelSchlick(max(dot(H, V), 0.001), F0);

			vec3 kS = F;
			vec3 kD = vec3(1.0) - kS;
			kD *= 1.0 - other.x;

			vec3 numerator = NDF * G * F;
			float denominator = 4.0 * max(dot(normal.rgb, V), 0.0) * max(dot(normal.rgb, L), 0.0) + 0.0001;
			vec3 specular = (numerator / denominator);

			// Add to outgoing radiance Lo
			float NdotL = max(dot(normal.rgb, L), 0.0);

			if (ubl.ubl[i].status == 0) // DirLight
			{
				if (ubl.ubl[i].shadowID >= 0)
				{
					uint cascadeIndex = 0;
					for (uint k = 0; k < SHADOW_MAP_CASCADE_COUNT - 1; ++k)
					{
						if (viewPos.z < ubs.s[ubl.ubl[i].shadowID + k].splitDepth)
						{
							cascadeIndex = k + 1;
						}
					}
					vec4 shadowCoord = (biasMat * ubs.s[ubl.ubl[i].shadowID + cascadeIndex].projview) * vec4(WorldPos, 1.0);
					float bias = calculateBias(shadowCoord, normal.rgb, L);
					shadow = filterPCF(shadowCoord / shadowCoord.w, ubl.ubl[i].shadowID + cascadeIndex, bias);
				}
				Lo += ((kD * color / PI + specular) * ubl.ubl[i].color * (ubl.ubl[i].range / 10.0) * NdotL) * mix(ubd.ambiant, 1.0, shadow);
			}
			else if (ubl.ubl[i].status == 1) // PointLight
			{
				if (ubl.ubl[i].shadowID >= 0)
				{
					for (uint k = 0; k < SHADOW_MAP_CUBE_COUNT && shadow > 0.5f; ++k)
					{
						vec4 shadowCoord = (biasMat * ubs.s[ubl.ubl[i].shadowID + k].projview) * vec4(WorldPos, 1.0);
						float bias = calculateBias(shadowCoord, normal.rgb, L);
						shadow *= filterPCF(shadowCoord / shadowCoord.w, ubl.ubl[i].shadowID + k, bias);
					}
				}
				Lo += (kD * color / PI + specular) * radiance * NdotL * mix(ubd.ambiant,1.0, shadow);
			}
			else if (ubl.ubl[i].status == 2) // SpotLight
			{
				vec3 lightDir = normalize(ubl.ubl[i].direction);
				float spotAngle = radians(ubl.ubl[i].spotAngle);
				float spotEffect = dot(lightDir, -L);

				if (spotEffect > cos(spotAngle / 2.0))
				{
					float transitionAngle = radians(4.0);
					float edge0 = cos(spotAngle / 2.0 - transitionAngle);
					float edge1 = cos(spotAngle / 2.0);
					float smoothFactor = smoothstep(edge1, edge0, spotEffect);

					Lo += (kD * color / PI + specular) * radiance * NdotL * pow(smoothFactor, 2.0) * mix(ubd.ambiant, 1.0, shadow);
				}
			}
		}
		//IBL PBR
		vec3 R = reflect(-V, normal.rgb);
		vec3 F = fresnelSchlickRoughness(max(dot(normal.rgb, V), 0.0), F0, other.y);

		vec3 kS = F;
		vec3 kD = 1.0 - kS;
		kD *= 1.0 - other.x;

		vec3 irradiance = texture(irradianceMap, normal.rgb).rgb;
		vec3 diffuse = vec3(ubd.ambiant) * irradiance * color;

		// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
		const float MAX_REFLECTION_LOD = 4.0;
		vec3 prefilteredColor = textureLod(prefilterMap, R, other.y * MAX_REFLECTION_LOD).rgb;
		vec2 brdf = texture(brdfLUT, vec2(max(dot(normal.rgb, V), 0.0), other.y)).rg;
		vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
		//IBL PBR

		vec3 ambient = (kD * diffuse + specular) * other.z;

		color = ambient + Lo;
	}
	else
	{
		color = normal.a * color;
	}

	vec3 dst = texture(fragTexture, gl_FragCoord.xy * u_invResolution).rgb;
	o_FragColor = color * col.a + dst * (1.0 - col.a);
}