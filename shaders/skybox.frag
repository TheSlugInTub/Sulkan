#version 450

layout(location = 1) in vec2 fragTexCoord;

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

    outColor = vec4(baseColor, 1.0);
}
