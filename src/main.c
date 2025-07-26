#include <sulkan/sulkan.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    skWindow window =
        skWindow_Create("Sulkan", 800, 600, false, true);

    skRenderer renderer = skRenderer_Create(&window);

    skRenderer_InitImGui(&renderer);

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
                           .window = &window};

    skEditor editor = {.ecsState = &ecsState};
    strcpy(editor.sceneName, "res/scenes/main_scene.json");

    skRenderObject obj = {0};

    skModel model = skModel_Create();
    skModel_Load(&model, "res/models/dancing_vampire.dae");

    obj = skRenderObject_CreateFromModel(
        &renderer, &model, "res/textures/image.bmp",
        "res/textures/default_normal.bmp", "res/textures/default_roughness.bmp");

    mat4 trans = GLM_MAT4_IDENTITY_INIT;
    glm_translate(trans, (vec3) {0.0f, 0.0f, 0.0f});
    glm_quat_rotate(trans, (vec3) {0.0f, 0.0f, 0.0f}, trans);
    glm_scale(trans, (vec3) {0.01f, 0.01f, 0.01f});

    glm_mat4_copy(trans, obj.transform);
    
    skAnimation anim = skAnimation_Create("res/models/dancing_vampire.dae", &model);

    skAnimator animator = skAnimator_Create(&anim);

    obj.boneTransforms = animator.finalBoneMatrices;

    skRenderer_AddRenderObject(&renderer, &obj);

    skECS_AddSystem(skCamera_Sys, false);
    skECS_AddSystem(skRenderAssociation_StartSys, true);
    skECS_AddSystem(skLightAssociation_StartSys, true);
    skECS_AddSystem(skDeltaTimeSystem, false);

    skECS_StartStartSystems(&ecsState);

    while (!skWindow_ShouldClose(&window))
    {
        skECS_UpdateSystems(&ecsState);

        skAnimator_UpdateAnimation(&animator, ecsState.deltaTime);

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
