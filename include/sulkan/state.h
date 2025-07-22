#pragma once

#include <sulkan/renderer.h>
#include <sulkan/ecs_api.h>
#include <sulkan/camera.h>

typedef struct skECSState 
{
    skRenderer* renderer;
    skSceneHandle scene;
    skCamera* camera;
    skWindow* window;
    float deltaTime;
} skECSState;
