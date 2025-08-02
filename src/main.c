#include <sulkan/sulkan.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    skWindow window =
        skWindow_Create("Sulkan", 800, 600, false, true);

    skRenderer renderer = skRenderer_Create(&window);

    skRenderer_InitImGui(&renderer);

    skPhysics3DState physicsState = skPhysics3DState_Create();

    skSceneHandle scene = skECS_CreateScene();

    float fps = 0.0f;
    int   frameCount = 0;
    float lastTime = glfwGetTime();
    float timeAccumulator = 0.0f;
    char  fpsString[16] = {0};

    skCamera camera = skCamera_Create((vec3) {0.0f, 3.0f, 0.0f},
                                      (vec3) {0.0f, 0.0f, 1.0f},
                                      -90.0f, 0.0f, 80.0f);
    skCamera_UpdateVectors(&camera);

    skECSState ecsState = {.scene = scene,
                           .renderer = &renderer,
                           .camera = &camera,
                           .window = &window,
                           .physics3dState = &physicsState};

    skEditor editor = {.ecsState = &ecsState};
    strcpy(editor.sceneName, "res/scenes/main_scene.json");

    vec3 cubePoints[8] = {{-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
                          {1.0f, 1.0f, -1.0f},   {-1.0f, 1.0f, -1.0f},
                          {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, 1.0f},
                          {1.0f, 1.0f, 1.0f},    {-1.0f, 1.0f, 1.0f}};
    u32  cubeIndices[24] = {
        0, 1, 1, 2, 2, 3, 3, 0, // bottom
        4, 5, 5, 6, 6, 7, 7, 4, // top
        0, 4, 1, 5, 2, 6, 3, 7  // sides
    };

    skLineObject lineObj = skLineObject_Create(
        &renderer, cubePoints, cubeIndices, 8, 24, (vec3) {1.0f, 0.0f, 0.0f}, 3.0f);

    skRenderer_AddLineObject(&renderer, &lineObj);

    skECS_AddSystem(skCamera_Sys, false);
    skECS_AddSystem(skRenderAssociation_StartSys, true);
    skECS_AddSystem(skLightAssociation_StartSys, true);
    skECS_AddSystem(skDeltaTimeSystem, false);
    skECS_AddSystem(skRigidbody3D_StartSys, true);
    skECS_AddSystem(skRigidbody3D_Sys, false);

    skECS_StartStartSystems(&ecsState);

    while (!skWindow_ShouldClose(&window))
    {
        skECS_UpdateSystems(&ecsState);

        if (editor.playing)
        {
            skPhysics3DState_Step(&physicsState, ecsState.deltaTime);
        }

        skRenderer_DrawFrame(&renderer, &editor);

        float currentTime = glfwGetTime();
        float elapsed = currentTime - lastTime;
        lastTime = currentTime;

        // Update frame count
        frameCount++;

        // Accumulate time
        timeAccumulator += elapsed;

        // Update FPS every second
        if (timeAccumulator >= 0.1f)
        {
            fps = frameCount / timeAccumulator;

            frameCount = 0;
            timeAccumulator = 0.0f;

            sprintf(fpsString, "FPS: %f", fps);
            skWindow_Rename(&window, fpsString);
        }

        skWindow_Update(&window);
    }

    skRenderer_Destroy(&renderer);
    skWindow_Close(&window);

    return 0;
}
