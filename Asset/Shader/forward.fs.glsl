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

const mat4 biasMat = mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gColorSpec;
layout(binding = 3) uniform sampler2D gOther;
layout(binding = 4) uniform sampler2DArray shadowDepth;


float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex)
{
	float shadow = 1.0;
	float bias = 0.005;

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

float filterPCF(vec4 sc, uint cascadeIndex)
{
	ivec3 texDim = textureSize(shadowDepth, 0);

	float scale = 0.75;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;

	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx * x, dy * y), cascadeIndex);
			count++;
		}
	}
	return shadowFactor / count;
}

in vec2 v_UV;
out vec4 o_FragColor;

void main()
{
	vec4 positionAO = texture(gPosition, v_UV).rgba;
	vec4 normalRoughness = texture(gNormal, v_UV).rgba;
	vec4 colSpec = texture(gColorSpec, v_UV).rgba;
	float viewZ = texture(gOther, v_UV).r;

	vec3 color = colSpec.rgb;	
	color = pow(color, vec3(ubd.gamma));

	vec3 ambient = vec3(0.01f) * color * positionAO.a;

	vec3 V = normalize(ubc.camPos - positionAO.rgb);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, color, colSpec.a);

	// Reflectance equation
	vec3 Lo = vec3(0.0);
	for (int i = 0; i < ubd.maxLight; i++)
	{
		float shadow = 1.0;
		vec3 L = normalize(ubl.ubl[i].position - positionAO.rgb);		
		float distance = length(ubl.ubl[i].position - positionAO.rgb);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = ubl.ubl[i].color * attenuation * ubl.ubl[i].range;
		if (ubl.ubl[i].status == 0)
		{
			L = normalize(-ubl.ubl[i].direction);
		}
		vec3 H = normalize(V + L);

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(normalRoughness.rgb, H, normalRoughness.a);
		float G = GeometrySmith(normalRoughness.rgb, V, L, normalRoughness.a);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.001), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - colSpec.a;

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(normalRoughness.rgb, V), 0.0) * max(dot(normalRoughness.rgb, L), 0.0) + 0.0001;
		vec3 specular = (numerator / denominator);

		// Add to outgoing radiance Lo
		float NdotL = max(dot(normalRoughness.rgb, L), 0.0);

		if (ubl.ubl[i].status == 0) // DirLight
		{
			if (ubl.ubl[i].shadowID >= 0)
			{
				uint cascadeIndex = 0;
				for (uint k = 0; k < SHADOW_MAP_CASCADE_COUNT - 1; ++k)
				{
					if (viewZ < ubs.s[ubl.ubl[i].shadowID + k].splitDepth)
					{
						cascadeIndex = k + 1;       
					}
				}			
				vec4 shadowCoord = (biasMat * ubs.s[ubl.ubl[i].shadowID + cascadeIndex].projview) * vec4(positionAO.rgb, 1.0);
				shadow = filterPCF(shadowCoord / shadowCoord.w, ubl.ubl[i].shadowID + cascadeIndex);				
			}
			Lo += ((kD * color / PI + specular) * ubl.ubl[i].color *(ubl.ubl[i].range / 10.0)* NdotL)* shadow;
		}
		else if (ubl.ubl[i].status == 1) // PointLight
		{
			if (ubl.ubl[i].shadowID >= 0)
			{
				for (uint k = 0; k < SHADOW_MAP_CUBE_COUNT; ++k)
				{
					vec4 shadowCoord = (biasMat * ubs.s[ubl.ubl[i].shadowID + k].projview) * vec4(positionAO.rgb, 1.0);
					shadow *= filterPCF(shadowCoord / shadowCoord.w, ubl.ubl[i].shadowID + k);
				}
			}
			Lo += (kD * color / PI + specular) * radiance * NdotL * shadow;
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

				Lo += (kD * color / PI + specular) * radiance * NdotL * pow(smoothFactor, 2.0);
			}
		}
	}

	color = ambient + Lo;

	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0 / ubd.gamma));

	o_FragColor = vec4(color.rgb, 1.0);
}