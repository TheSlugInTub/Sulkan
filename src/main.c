#include <sulkan/sulkan.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    skWindow window =
        skWindow_Create("Sulkan", 800, 600, false, true);

    skModel model = skModel_Create();
    skModel_Load(&model, "res/room.fbx");

    skRenderer renderer = {0};
    skRenderer_InitializeVulkan(&renderer, &window);
    
    skSceneHandle scene = skECS_CreateScene();

    skEntityID ent1 = skECS_AddEntity(scene);

    skRenderAssociation* assoc = SK_ECS_ASSIGN(scene, ent1, skRenderAssociation);
    assoc->objectIndex = renderer.renderObjects->size;
    skName* name = SK_ECS_ASSIGN(scene, ent1, skName);
    strcpy(name->name, "Ent");

    skRenderObject obj = skRenderObject_CreateFromModel(
        &renderer, &model, "res/room.png");
    glm_translate(obj.transform, (vec3) {-1.0f, 0.0f, 0.0f});

    skRenderer_AddRenderObject(&renderer, &obj);
    
    skEntityID ent2 = skECS_AddEntity(scene);

    skRenderAssociation* assoc2 = SK_ECS_ASSIGN(scene, ent2, skRenderAssociation);
    assoc2->objectIndex = renderer.renderObjects->size;
    skName* name2 = SK_ECS_ASSIGN(scene, ent2, skName);
    strcpy(name2->name, "Ent");

    skRenderObject obj2 = skRenderObject_CreateFromModel(
        &renderer, &model, "res/room.png");
    glm_translate(obj2.transform, (vec3) {1.0f, 0.0f, 0.0f});

    skRenderer_AddRenderObject(&renderer, &obj2);
    
    skEntityID ent3 = skECS_AddEntity(scene);

    skRenderAssociation* assoc3 = SK_ECS_ASSIGN(scene, ent3, skRenderAssociation);
    assoc3->objectIndex = renderer.renderObjects->size;
    skName* name3 = SK_ECS_ASSIGN(scene, ent3, skName);
    strcpy(name3->name, "Ent");

    skRenderObject obj3 =
        skRenderObject_CreateFromSprite(&renderer, "res/textures/image.png");
    glm_translate(obj3.transform, (vec3) {0.0f, 2.0f, 0.0f});
    glm_scale(obj3.transform, (vec3) {1.0f, 1.0f, 1.0f});

    skRenderer_AddRenderObject(&renderer, &obj3);

    skRenderer_InitializeUniformsAndDescriptors(&renderer);

    skRenderer_InitImGui(&renderer);

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
                           .window = &window};

    skEditor editor = {.ecsState = &ecsState};

    skECS_AddSystem(skCamera_Sys, false);

    skECS_StartStartSystems(&ecsState);

    while (!skWindow_ShouldClose(&window))
    {
        skECS_UpdateSystems(&ecsState);

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
