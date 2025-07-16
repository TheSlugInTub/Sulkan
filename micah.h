#pragma once

#include "include/sulkan/basic_components.h"
#include "include/sulkan/render_association.h"
#include "include/sulkan/imgui_layer.h"
#include "include/sulkan/state.h"
#include "include/sulkan/json_api.h"

// micah.h
// Procedurally generated header file for the Sulkan game engine
// This contains drawer/serializer/deserializer functions for all registered components

void skName_DrawComponent(skName* object, skECSState* state)
{
    if (skImGui_CollapsingHeader("skName"))
    {
        skImGui_InputText("name", object->name, 128, 0);
    }
}

skJson skName_SaveComponent(skName* object)
{
    skJson j = skJson_Create();

    skJson_SaveString(j, "name", object->name);
    return j;
}

void skName_LoadComponent(skName* object, skJson j)
{
    skJson_LoadString(j, "name", object->name);
}

void skRenderAssociation_DrawComponent(skRenderAssociation* object, skECSState* state)
{
    if (skImGui_CollapsingHeader("skRenderAssociation"))
    {
        if (skImGui_DragFloat3("position", object->position, 0.1f) || 
            skImGui_DragFloat4("rotation", object->rotation, 0.1f) ||
            skImGui_DragFloat3("scale", object->scale, 0.1f))
        {
            mat4 trans = GLM_MAT4_IDENTITY_INIT;
            glm_translate(trans, object->position);
            glm_quat_rotate(trans, object->rotation, trans);
            glm_scale(trans, object->scale);
            
            skRenderObject* obj = skVector_Get(state->renderer->renderObjects,
                    object->objectIndex);
            glm_mat4_copy(trans, obj->transform);
        }
    }
}

skJson skRenderAssociation_SaveComponent(skRenderAssociation* object)
{
    skJson j = skJson_Create();

    skJson_SaveFloat3(j, "position", object->position);
    skJson_SaveFloat4(j, "rotation", object->rotation);
    skJson_SaveFloat3(j, "scale", object->scale);
    return j;
}

void skRenderAssociation_LoadComponent(skRenderAssociation* object, skJson j)
{
    skJson_LoadFloat3(j, "position", object->position);
    skJson_LoadFloat4(j, "rotation", object->rotation);
    skJson_LoadFloat3(j, "scale", object->scale);
}

void Micah_DrawAllComponents(skECSState* state, skEntityID ent)
{
    skName* skNameObj = SK_ECS_GET(state->scene, ent, skName);

    if (skNameObj != NULL)
    {
        skName_DrawComponent(skNameObj, state);
    }

    skRenderAssociation* skRenderAssociationObj = SK_ECS_GET(state->scene, ent, skRenderAssociation);

    if (skRenderAssociationObj != NULL)
    {
        skRenderAssociation_DrawComponent(skRenderAssociationObj, state);
    }

}

skJson Micah_SaveAllComponents(skECSState* state)
{
    skJson j = skJson_Create();

    for (int i = 0; i < skECS_EntityCount(state->scene); i++)
    {
        skEntityID ent = skECS_GetEntityAtIndex(state->scene, i);
        skJson entJson = skJson_Create();

        skName* skNameObj = SK_ECS_GET(state->scene, ent, skName);

        if (skNameObj != NULL)
        {
            skJson compJson = skName_SaveComponent(skNameObj);
            skJson_SaveString(compJson, "type", "skName");
            skJson_PushBack(entJson, compJson);
            skJson_Destroy(compJson);
        }

        skRenderAssociation* skRenderAssociationObj = SK_ECS_GET(state->scene, ent, skRenderAssociation);

        if (skRenderAssociationObj != NULL)
        {
            skJson compJson = skRenderAssociation_SaveComponent(skRenderAssociationObj);
            skJson_SaveString(compJson, "type", "skRenderAssociation");
            skJson_PushBack(entJson, compJson);
            skJson_Destroy(compJson);
        }

        skJson_PushBack(j, entJson);
        skJson_Destroy(entJson);
    }

    return j;
}


void Micah_LoadAllComponents(skECSState* state, skJson j)
{
    int entityCount = skJson_GetArraySize(j);

    for (int i = 0; i < entityCount; i++)
    {
        skJson entJson = skJson_GetArrayElement(j, i);
        skEntityID ent = skECS_AddEntity(state->scene);

        int componentCount = skJson_GetArraySize(entJson);

        for (int compIndex = 0; compIndex < componentCount; compIndex++)
        {
            skJson compJson = skJson_GetArrayElement(entJson, compIndex);

            char componentType[256];
            skJson_LoadString(compJson, "type", componentType);

            // Check component type and assign/load accordingly
            if (false) {} // Dummy condition for cleaner generated code
            else if (strcmp(componentType, "skName") == 0)
            {
                skName* comp = SK_ECS_ASSIGN(state->scene, ent, skName);
                skName_LoadComponent(comp, compJson);
            }
            else if (strcmp(componentType, "skRenderAssociation") == 0)
            {
                skRenderAssociation* comp = SK_ECS_ASSIGN(state->scene, ent, skRenderAssociation);
                skRenderAssociation_LoadComponent(comp, compJson);
            }
        }
    }
}

void Micah_ComponentAddMenu(skECSState* state, skEntityID ent)
{
    if (skImGui_BeginPopupContextWindow())
    {
        if (skImGui_MenuItem("skName"))
        {
            SK_ECS_ASSIGN(state->scene, ent, skName);
        }
        if (skImGui_MenuItem("skRenderAssociation"))
        {
            SK_ECS_ASSIGN(state->scene, ent, skRenderAssociation);
        }

        skImGui_EndPopup();
    }
}
