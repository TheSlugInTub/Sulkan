#pragma once

#include <sulkan/renderer.h>
#include <sulkan/basic_components.h>

typedef enum skRenderObjectType
{
    skRenderObjectType_Model,
    skRenderObjectType_Sprite
} skRenderObjectType;

// COMPONENT
typedef struct skRenderAssociation
{
    int objectIndex;
    int type;

    char modelPath[128];
    char texturePath[128];
    char normalTexturePath[128];

    vec3 position;
    vec4 rotation; // Quaternion
    vec3 scale;
} skRenderAssociation;

void skRenderAssociation_CreateRenderObject(skRenderAssociation* assoc,
        skECSState* state);
void skRenderAssociation_StartSys(skECSState* state);
