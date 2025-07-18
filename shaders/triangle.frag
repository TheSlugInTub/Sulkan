#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragWorldPos;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

struct skLight 
{
    vec3 position;
    vec3 color;
    float radius;
    float intensity;
};

layout(std430, binding = 2) readonly buffer LightBuffer {
    skLight lights[];
};

void main() 
{
    vec3 baseColor = texture(texSampler, fragTexCoord).rgb;
    
    vec3 cameraPos = vec3(2.0f, 0.0f, 1.0f);

    // Normalize normal and calculate view direction
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(cameraPos - fragWorldPos);
    
    // Lighting parameters
    vec3 ambient = 0.05 * baseColor;  // Low ambient factor
    vec3 lighting = ambient;
    float specularStrength = 0.5;     // Material specular intensity
    float shininess = 32.0;           // Material shininess exponent

    // Process each light in the buffer
    for (int i = 0; i < lights.length(); i++) {
        skLight light = lights[i];
        vec3 L = light.position - fragWorldPos;
        float dist = length(L);
        L = normalize(L);
        
        // Skip light if beyond radius
        if (dist > light.radius) continue;
        
        // Attenuation (quadratic falloff)
        float attenFactor = 1.0 - pow(dist / light.radius, 4.0);
        float attenuation = attenFactor * attenFactor * light.intensity;
        
        // Diffuse contribution
        float NdotL = max(dot(N, L), 0.0);
        vec3 diffuse = NdotL * light.color * baseColor;
        
        // Specular (Blinn-Phong)
        vec3 H = normalize(L + V);  // Half vector
        float NdotH = max(dot(N, H), 0.0);
        vec3 specular = specularStrength * pow(NdotH, shininess) * light.color;
        
        // Combine contributions
        lighting += (diffuse + specular) * attenuation;
    }

    outColor = vec4(lighting, 1.0);
}
