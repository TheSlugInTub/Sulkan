#pragma once

#include <sulkan/renderer.h>
#include <sulkan/ecs_api.h>
#include <sulkan/camera.h>

typedef struct skPhysics3DState skPhysics3DState;

typedef struct skECSState 
{
    skRenderer*       renderer;
    skSceneHandle     scene;
    skCamera*         camera;
    skWindow*         window;
    float             deltaTime;
    skPhysics3DState* physics3dState;
} skECSState;
