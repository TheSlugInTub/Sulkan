#version 450

layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragWorldPos;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec3 fragTangent;
layout(location = 5) in vec3 fragBitangent;
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

void main() 
{
    vec3 baseColor = texture(texSampler, fragTexCoord).rgb;
    vec3 ambient = 0.15 * baseColor;

    float specularStrength = 2.5;
    
    vec3 norm = normalize(fragNormal);
        
    vec3 normal = texture(normalSampler, fragTexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 result = ambient;

    for (int i = 0; i < gubo.lightCount; i++)
    {
        vec3 tangentViewPos = fragTBN * gubo.viewPos;
        vec3 tangentLightPos = fragTBN * lights[i].position;
        vec3 tangentFragPos = fragTBN * fragWorldPos;

        vec3 viewDir = normalize(tangentViewPos - tangentFragPos);

        vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
        vec3 reflectDir = reflect(-lightDir, normal);
      
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
        vec3 specular = specularStrength * spec * lights[i].color;
        
        float diff = max(dot(normal, lightDir), 0.0);

        vec3 diffuse = vec3(diff);
  
        result += ((diffuse + specular) * baseColor * lights[i].color) * 
            lights[i].intensity;
    }

    outColor = vec4(result, 1.0);
}
