
#version 450

layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragWorldPos;
layout(location = 3) in vec3 fragNormal;
layout(location = 6) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texSampler;
layout(set = 0, binding = 2) uniform sampler2D normalSampler;

struct skLight 
{
    vec3 position;
    vec3 color;
    float radius;
    float intensity;
};

layout(std430, set = 1, binding = 0) readonly buffer LightBuffer {
    skLight lights[];
};

layout(set = 2, binding = 0) uniform skGlobalUniformBufferObject 
{
    vec3 viewPos;
    int lightCount;
} gubo;
        
const float PI = 3.14159265359;

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

void main() 
{
    vec3 baseColor = pow(texture(texSampler, fragTexCoord).rgb, vec3(2.2));

    float roughness = 0.2;
    float metallic = 0.0;
    
    vec3 norm = normalize(fragNormal);
        
    vec3 normal = texture(normalSampler, fragTexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 result = vec3(0.0f);
        
    vec3 tangentFragPos = fragTBN * fragWorldPos;
    vec3 tangentViewPos = fragTBN * gubo.viewPos;
        
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);

    for (int i = 0; i < gubo.lightCount; i++)
    {
        vec3 tangentLightPos = fragTBN * lights[i].position;

        vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
        vec3 H = normalize(viewDir + lightDir);

        float dist = length(tangentLightPos - tangentFragPos);
        float attenuation = 1.0 / (dist * dist);
        vec3 radiance     = lights[i].color * attenuation;
      
        vec3 F0 = vec3(0.04); 
        F0      = mix(F0, baseColor, metallic);
      
        // Cook torrance BRDF
        float NDF = DistributionGGX(normal, H, roughness);       
        float G   = GeometrySmith(normal, viewDir, lightDir, roughness);
        vec3 F  = FresnelSchlick(max(dot(H, viewDir), 0.0), F0);
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
          
        kD *= 1.0 - metallic;

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * 
            max(dot(normal, lightDir), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;

        float NdotL = max(dot(normal, lightDir), 0.0);        
        result += (kD * baseColor / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * baseColor;
    result = ambient + result;

    // Gamma correction
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));

    outColor = vec4(result, 1.0);
}
