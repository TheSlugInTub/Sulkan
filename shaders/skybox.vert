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
layout(location = 5) in ivec4 inBoneIDs;
layout(location = 6) in vec4 inWeights;

layout(location = 1) out vec2 fragTexCoord;

void main() 
{
    fragTexCoord = inTexCoord;

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
}
