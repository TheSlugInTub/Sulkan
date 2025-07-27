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
layout(location = 2) out vec3 fragWorldPos;
layout(location = 3) out vec3 fragNormal;
layout(location = 6) out mat3 fragTBN;

layout(set = 3, binding = 0, std430) restrict readonly buffer MatrixBuffer {
    mat4 boneMatrices[];
};

void main() 
{
    fragWorldPos = vec3(ubo.model * vec4(inPosition, 1.0));
    fragNormal = normalize(mat3(transpose(inverse(ubo.model))) * inNormal);
    fragTexCoord = inTexCoord;

    vec4 totalPosition = vec4(0.0f);

    for (int i = 0; i < 4; i++)
    {
        if (inBoneIDs[i] == -1)
            continue;

        vec4 localPosition = boneMatrices[inBoneIDs[i]] * vec4(inPosition, 1.0f);
        totalPosition += localPosition * inWeights[i];
    }
    
    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));

    vec3 T = normalize(normalMatrix * inTangent);
    vec3 N = normalize(normalMatrix * inNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    fragTBN = transpose(mat3(T, B, N));

    gl_Position = ubo.proj * ubo.view * ubo.model * totalPosition;
}
