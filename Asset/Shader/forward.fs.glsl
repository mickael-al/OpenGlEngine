#version 450

#define SHADOW_MAP_CASCADE_COUNT 4
#define SHADOW_MAP_CUBE_COUNT 6

const float PI = 3.14159265359;

layout(std430, binding = 0) buffer UniformBufferCamera
{
	vec3 camPos;
	mat4 view;
	mat4 proj;
} ubc;

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

layout(std430, binding = 1) buffer UniformBufferLight
{
	StructUBL ubl[];
} ubl;

layout(std430, binding = 2) buffer UniformBufferDivers
{
	int maxLight;
	float u_time;
	float gamma;
	float ambiant;
	float fov;
	bool ortho;
} ubd;

struct StructShadow
{
	mat4 projview;
	vec3 pos;
	float splitDepth;
};

layout(std430, binding = 3) buffer UniformBufferShadow
{
	StructShadow s[];
} ubs;

layout(std430, binding = 4) buffer UniformBufferSSAO
{
	vec3 samples[];
} ubssao;

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

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gColor;
layout(binding = 3) uniform sampler2D gOther;
layout(binding = 4) uniform sampler2DArray shadowDepth;
layout(binding = 5) uniform sampler2D texNoise;
layout(binding = 6) uniform samplerCube irradianceMap;
layout(binding = 7) uniform samplerCube prefilterMap;
layout(binding = 8) uniform sampler2D brdfLUT;

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

in vec2 v_UV;

float SSAO(vec3 viewPos, vec3 normal)
{
	ivec2 ts = textureSize(gPosition,0);
	vec3 randomVec = normalize(texture(texNoise, v_UV * vec2(float(ts.x) / 4.0, float(ts.y) / 4.0)).xyz);

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);
	float occlusion = 0.0;
	float radius = 0.5;	
	float bias = 0.025;

	for (int i = 0; i < 64; ++i)
	{
		vec3 samplePos = TBN * ubssao.samples[i];
		samplePos = viewPos + samplePos * radius;
		vec4 offset = vec4(samplePos, 1.0);
		offset = ubc.proj * offset;    // from view to clip-space		
		offset.xyz /= offset.w;               // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5;
		float sampleDepth = texture(gPosition, offset.xy).z;
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0);
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	return 1.0 - (occlusion / 64);
}

out vec3 o_FragColor;

void main()
{	
	vec3 viewPos = texture(gPosition, v_UV).rgb;
	vec4 normal = texture(gNormal, v_UV).rgba;
	vec4 col = texture(gColor, v_UV).rgba;
	vec3 other = texture(gOther, v_UV).rgb;//metallic roughness ao emit

	vec3 color = col.rgb;
	color = pow(color, vec3(ubd.gamma));
	vec3 worldPos = (inverse(ubc.view) * vec4(viewPos, 1)).xyz;

	vec3 V = normalize(ubc.camPos - worldPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, color, other.x);
	if (normal.a <= 0)
	{
		// Reflectance equation
		vec3 Lo = vec3(0.0);
		for (int i = 0; i < ubd.maxLight; i++)
		{
			float shadow = 1.0;
			vec3 L = normalize(ubl.ubl[i].position - worldPos);
			float distance = length(ubl.ubl[i].position - worldPos);
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
					vec4 shadowCoord = (biasMat * ubs.s[ubl.ubl[i].shadowID + cascadeIndex].projview) * vec4(worldPos, 1.0);
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
						vec4 shadowCoord = (biasMat * ubs.s[ubl.ubl[i].shadowID + k].projview) * vec4(worldPos, 1.0);
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

		color = ambient + Lo * SSAO(viewPos, normal.rgb);
	}
	else
	{
		color = normal.a * color;
	}

	o_FragColor = color;
}