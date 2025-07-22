#version 450

layout(set = 0, binding = 0) uniform skUniformBufferObject 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragWorldPos;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec3 fragTangent;
layout(location = 5) out vec3 fragBitangent;
layout(location = 6) out mat3 fragTBN;

void main() 
{
    fragWorldPos = vec3(ubo.model * vec4(inPosition, 1.0));
    fragNormal = normalize(mat3(transpose(inverse(ubo.model))) * inNormal);
    fragTexCoord = inTexCoord;
    fragTangent = inTangent;
    fragBitangent = inBitangent;
        
    vec3 T = normalize(vec3(ubo.model * vec4(inTangent,   0.0)));
    vec3 B = normalize(vec3(ubo.model * vec4(inBitangent, 0.0)));
    vec3 N = normalize(vec3(ubo.model * vec4(inNormal,    0.0)));
    fragTBN = transpose(mat3(T, B, N));

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
}
