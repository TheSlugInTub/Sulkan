#version 450

layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragWorldPos;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

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

void main() 
{
    vec3 cameraPos = vec3(2.0, 0.0, 1.0);

    vec3 baseColor = texture(texSampler, fragTexCoord).rgb;
    
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lights[0].position - fragWorldPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 diffuse = diff * lights[0].color;
  
    vec3 result = (diffuse) * baseColor;

    outColor = vec4(result, 1.0);
}
