#pragma once

#include <cglm/cglm.h>
#include <sulkan/renderer.h>
#include <sulkan/ecs_api.h>

typedef struct skSystemInfo 
{
    skSceneHandle scene;
    skRenderer* renderer;
} skSystemInfo;

// COMPONENT
typedef struct skName
{
    char name[128];
} skName;

// COMPONENT
typedef struct skTransform
{
    mat4 transform;
} skTransform;
