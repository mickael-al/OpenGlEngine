#version 430

const float PI = 3.14159265359;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D shadowMap;

uniform Materials
{
	vec3 diffuseColor;
	vec2 offset;
	vec2 tilling;
	float metallic;
	float roughness;
	float normal;
	float ao;	
}mat;

out vec4 o_FragColor;

uniform MatrixCamera
{
    vec3 position;
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
}mc;

uniform UniformBufferDiver
{
	uint maxLight;
	uint maxShadow;
	float u_time;
	float gamma;
}ubd;

uniform MatrixShadow
{
    vec3 position;
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
}ms;


struct LUBO
{
    vec3 position;
    vec3 color;
    vec3 direction;
    float range;
    float spotAngle;
    uint status;//DirLight = 0 ; PointLight = 1 ; SpotLight = 2
};

layout(binding = 1) buffer LightUBO
{
    LUBO lubo[];
} ubl;

in vec3 v_FragPosition;
in vec2 v_TexCoords;
in vec4 v_ShadowCoord;
in mat3 v_TBN;


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

float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    //projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}  

void main(void)
{
    vec3 color = texture(albedoMap, v_TexCoords).rgb * mat.diffuseColor;
    vec3 ambient = vec3(0.003) * color * mat.ao * texture(aoMap, v_TexCoords).rgb;
    vec3 metallic = texture(metallicMap, v_TexCoords).rgb * mat.metallic;
    float roughness = texture(roughnessMap, v_TexCoords).r * mat.roughness;
    vec3 normal = texture(normalMap, v_TexCoords).rgb * 2.0;
    normal = mix(vec3(0.5, 0.5, 1.0), normal, mat.normal);
    normal = normalize(normal * 2.0 - 1.0);

    vec3 N = normalize(v_TBN * vec3(0,0,1));

    vec3 V = normalize(mc.position - v_FragPosition);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, color, metallic);

    vec3 Lo = vec3(0.0);
    for (uint i = 0; i < ubd.maxLight; i++)
    {
        vec3 L = normalize(ubl.lubo[i].position - v_FragPosition);
        vec3 H = normalize(V + L);
        float distance = length(ubl.lubo[i].position - v_FragPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = ubl.lubo[i].color * attenuation * ubl.lubo[i].range;
        if (ubl.lubo[i].status == 0)
        {
            L = -ubl.lubo[i].direction;
            H = normalize(V + L);
        }

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = (numerator / denominator);

        // Add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);

        if (ubl.lubo[i].status == 0) // DirLight
        {
            Lo += (kD * color + specular) * ubl.lubo[i].color * ubl.lubo[i].range / 10.0f * NdotL;
        }
        else if (ubl.lubo[i].status == 1) // PointLight
        {
            Lo += (kD * color / PI + specular) * radiance * NdotL;
        }
        else if (ubl.lubo[i].status == 2) // SpotLight
        {
            vec3 lightDir = normalize(ubl.lubo[i].direction);
            float spotAngle = radians(ubl.lubo[i].spotAngle);
            float spotEffect = dot(lightDir, -L);

            float bias = 0.005*tan(acos(clamp(dot(N, L),0.0,1.0)));
            bias = clamp(bias, 0.0,0.0001);

                float visibility = 1.0;
                if(ShadowCalculation(v_ShadowCoord, bias) > 0.0)
                {
                    visibility = 0.0f;
                }
                if (spotEffect > cos(spotAngle / 2.0))
                {
                    float transitionAngle = radians(4.0);
                    float edge0 = cos(spotAngle / 2.0 - transitionAngle);
                    float edge1 = cos(spotAngle / 2.0);
                    float smoothFactor = smoothstep(edge1, edge0, spotEffect);

                    Lo += (kD * color / PI + specular) * radiance * NdotL * pow(smoothFactor, 2.0) * visibility;
                }
            
        }
    }

    color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    o_FragColor = vec4(color, 1.0);
}