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
        skImGui_DragFloat3("position", object->position, 0.1f);
        skImGui_DragFloat4("rotation", object->rotation, 0.1f);
        skImGui_DragFloat3("scale", object->scale, 0.1f);
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
