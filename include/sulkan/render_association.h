#pragma once

#include <sulkan/renderer.h>
#include <sulkan/basic_components.h>

// COMPONENT
typedef struct skRenderAssociation
{
    u32 objectIndex;
    vec3 position;
    vec4 rotation; // Quaternion
    vec3 scale;
} skRenderAssociation;
