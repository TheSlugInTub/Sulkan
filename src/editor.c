#include <sulkan/editor.h>
#include <sulkan/imgui_layer.h>
#include <sulkan/basic_components.h>
#include <sulkan/input.h>
#include <assert.h>
#include "../micah.h"
#include <sulkan/json_api.h>

void skEditor_SaveScene(skECSState* state, skSceneHandle scene,
                        const char* sceneName)
{
    skJson json = Micah_SaveAllComponents(state);
    skJson_SaveToFile(json, sceneName);
    skJson_Destroy(json);
}

void skEditor_LoadScene(skECSState* state, skSceneHandle scene, const char* filepath)
{
    skECS_ClearScene(scene);

    skJson j = skJson_LoadFromFile(filepath);

    if (j == NULL)
    {
        printf("SK ERROR: in skEditor_LoadScene, filepath is invalid.");
        return;
    }

    Micah_LoadAllComponents(state, j);

    skJson_Destroy(j);
}

void skEditor_DrawHierarchy(skEditor* editor)
{
    skSceneHandle scene = editor->ecsState->scene;

    skImGui_Begin("Hierarchy");

    for (int i = 0; i < skECS_EntityCount(scene); i++)
    {

        skEntityID ent = skECS_GetEntityAtIndex(scene, i);

        if (!skECS_IsEntityValid(ent))
            continue;

        skImGui_PushID((int)i);

        skName* name = SK_ECS_GET(scene, ent, skName);

        bool selected = editor->selectedEntityIndex == (int)i;
        assert(name != NULL);
        if (skImGui_Selectable(name->name, selected))
        {
            editor->selectedEntityIndex = (int)i;
            editor->selectedEntity = skECS_GetEntityAtIndex(
                scene, editor->selectedEntityIndex);
        }

        if (skInput_GetKeyDown(editor->ecsState->window, SK_KEY_F1))
        {
            skECS_CloneEntity(scene, editor->selectedEntity);
        }

        // if (skImGui_BeginDragDropSource(i << 3))
        // {
        //     skImGui_SetDragDropPayload("DND_DEMO_CELL", &i,
        //                                sizeof(size_t));
        //     skImGui_Textf("Dragging Object %d", i);
        //     skImGui_EndDragDropSource();
        // }

        // if (skImGui_BeginDragDropTarget())
        // {
        //     skImGuiPayload payload =
        //         skImGui_AcceptDragDropPayload("DND_DEMO_CELL");
        //     if (payload != NULL)
        //     {
        //         assert(payload->DataSize == sizeof(size_t));
        //         size_t payload_n =
        //             *(size_t*)skImGuiPayload_GetData(payload);
        //         if (payload_n != i)
        //         {
        //             //
        //             std::swap(engineState.scene.entities[payload_n],
        //             //           engineState.scene.entities[i]);
        //             if (sk_selectedEntityIndex == (int)i)
        //             {
        //                 sk_selectedEntityIndex = (int)payload_n;
        //                 sk_selectedEntity = skECS_GetEntityAtIndex(
        //                     skState.scene, sk_selectedEntityIndex);
        //             }
        //             else if (sk_selectedEntityIndex ==
        //             (int)payload_n)
        //             {
        //                 sk_selectedEntityIndex = (int)i;
        //                 sk_selectedEntity = skECS_GetEntityAtIndex(
        //                     skState.scene, sk_selectedEntityIndex);
        //             }
        //         }
        //     }
        //     skImGui_EndDragDropTarget();
        // }

        skImGui_PopID();
    }

    if (skImGui_BeginPopupContextWindow())
    {
        if (skImGui_MenuItem("Add Entity"))
        {
            skEntityID ent = skECS_AddEntity(scene);
            skName*    name = SK_ECS_ASSIGN(scene, ent, skName);
            strcpy(name->name, "Entity");
        }
        skImGui_EndPopup();
    }

    skImGui_End();
}

void skEditor_DrawInspector(skEditor* editor)
{
    if (editor->selectedEntityIndex == -1)
        return;

    skSceneHandle scene = editor->ecsState->scene;

    skImGui_Begin("Inspector");

    Micah_DrawAllComponents(editor->ecsState, editor->selectedEntity);

    if (skInput_GetKeyDown(editor->ecsState->window, SK_KEY_DELETE))
    {
        skECS_DestroyEntity(scene, editor->selectedEntity);
    }

    Micah_ComponentAddMenu(editor->ecsState, editor->selectedEntity);

    skImGui_End();
}

void skEditor_DrawTray(skEditor* editor)
{
    skImGui_Begin("Tray");

    if (skImGui_Button("Save"))
    {
        skEditor_SaveScene(editor->ecsState, editor->ecsState->scene, "scene.json");
    }
    
    if (skImGui_Button("Load"))
    {
        skEditor_LoadScene(editor->ecsState, editor->ecsState->scene, "scene.json");
    }

    skImGui_End();
}
