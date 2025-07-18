#version 450

layout(location = 0) in vec3 fragColor;
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
    vec3 baseColor = texture(texSampler, fragTexCoord).rgb;
    
    vec3 cameraPos = vec3(2.0f, 0.0f, 1.0f);
        
    skLight light = lights[0];

    outColor = vec4(light.position.x, 0.0, 0.0, 1.0);
}
