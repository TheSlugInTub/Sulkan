#pragma once

#include <cglm/cglm.h>
#include <sulkan/ecs_api.h>

typedef struct skCamera
{
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;

    float yaw;
    float pitch;
    float roll;
    float zoom;
    float FOV;

    vec3 worldUp;
} skCamera;

// Initializes the camera
skCamera skCamera_Create(vec3 position, vec3 up, float yaw,
                         float pitch, float FOV);

// Updates the right, up and front vectors
void skCamera_UpdateVectors(skCamera* camera);

// Gets the view matrix of the camera
void skCamera_GetViewMatrix(skCamera* camera, mat4 view);

// The camera's system that updates the renderer's view 
// matrix every frame
void skCamera_Sys(skECSState* state);
