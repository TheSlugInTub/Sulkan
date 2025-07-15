#pragma once

#include <cglm/cglm.h>
#include <sulkan/renderer.h>
#include <sulkan/ecs_api.h>

// COMPONENT
typedef struct skTransform
{
    mat4 transform;
} skTransform;

typedef struct skSystemInfo 
{
    skSceneHandle scene;
    skRenderer* renderer;
} skSystemInfo;
