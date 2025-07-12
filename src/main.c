#include <sulkan/sulkan.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    skWindow window =
        skWindow_Create("Sulkan", 800, 600, false, false);

    skModel model = skModel_Create();
    skModel_Load(&model, "res/room.fbx");

    skRenderer renderer = {0};
    skRenderer_InitializeVulkan(&renderer, &window);

    skRenderObject obj = skRenderObject_CreateFromModel(
        &renderer, &model, "res/room.png");
    glm_translate(obj.transform, (vec3) {-1.0f, 0.0f, 0.0f});

    skRenderer_AddRenderObject(&renderer, &obj);
    
    skRenderObject obj2 = skRenderObject_CreateFromModel(
        &renderer, &model, "res/room.png");
    glm_translate(obj2.transform, (vec3) {1.0f, 0.0f, 0.0f});

    skRenderer_AddRenderObject(&renderer, &obj2);

    skRenderer_InitializeUniformsAndDescriptors(&renderer);

    float fps = 0.0f;
    int   frameCount = 0;
    float lastTime = glfwGetTime();
    float timeAccumulator = 0.0f;
    char  fpsString[16] = {0};

    // skSceneHandle scene = skECS_CreateScene();
    // skEntityID ent = skECS_AddEntity(scene);
    // SK_ECS_ASSIGN(scene, ent, skRenderObject);

    while (!skWindow_ShouldClose(&window))
    {
        skRenderer_DrawFrame(&renderer);

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
