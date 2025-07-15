#include <sulkan/editor.h>
#include <sulkan/imgui_layer.h>
#include <sulkan/basic_components.h>
#include <sulkan/input.h>
#include <assert.h>

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
            skEntityID   ent = skECS_AddEntity(scene);
            skTransform* trans =
                SK_ECS_ASSIGN(scene, ent, skTransform);

            glm_mat4_identity(trans->transform);
            glm_scale(trans->transform, (vec3) {1.0f, 1.0f, 1.0f});

            skName* name = SK_ECS_ASSIGN(scene, ent, skName);
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

    // skRegistry_DrawComponents(editor->selectedEntity);

    if (skInput_GetKeyDown(editor->ecsState->window, SK_KEY_DELETE))
    {
        skECS_DestroyEntity(scene, editor->selectedEntity);
    }

    // Begin popup menu on right click
    if (skImGui_BeginPopupContextWindow())
    {
        // for (int i = 0; i < g_componentRegistry.registrationCount;
        //      ++i)
        // {
        //     if (skImGui_MenuItem(g_componentRegistry.registrations[i]
        //                              .componentType))
        //     {
        //         // Check if the component doens't already exist and
        //         // add it if it doenn't
        //         SK_ECS_ASSIGN_N(
        //             skState.scene, sk_selectedEntity,
        //             g_componentRegistry.registrations[i]
        //                 .componentType,
        //             g_componentRegistry.registrations[i].size);
        //     }
        // }

        skImGui_EndPopup();
    }

    skImGui_End();
}

void skEditor_DrawTray(skEditor* editor)
{

}
