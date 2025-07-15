#include "include/sulkan/basic_components.h"
#include "include/sulkan/imgui_layer.h"
#include "include/sulkan/state.h"

void skName_DrawComponent(skName* object)
{
    skImGui_InputText("name", object->name, 128, 0);
}

void skTransform_DrawComponent(skTransform* object)
{
    skImGui_DragFloat16("transform", object->transform, 2.0f);
}

void Micah_DrawAllComponents(skECSState* state, skEntityID ent)
{
    for (int i = 0; i < skECS_EntityCount(state->scene); i++)
    {
        skName* skNameObj = SK_ECS_GET(state->scene, ent, skName);

        if (skNameObj != NULL)
        {
            skName_DrawComponent(skNameObj);
        }

        skTransform* skTransformObj = SK_ECS_GET(state->scene, ent, skTransform);

        if (skTransformObj != NULL)
        {
            skTransform_DrawComponent(skTransformObj);
        }

    }
}
