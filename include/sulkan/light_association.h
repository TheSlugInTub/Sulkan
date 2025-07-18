#pragma once

#include <sulkan/renderer.h>
#include <sulkan/basic_components.h>

// COMPONENT
typedef struct skLightAssociation
{
    int lightIndex;
    vec3 position;
    vec3 color;
    float radius;
    float intensity;
} skLightAssociation;

void skLightAssociation_CreateLight(skLightAssociation* assoc,
        skECSState* state);
void skLightAssociation_StartSys(skECSState* state);
