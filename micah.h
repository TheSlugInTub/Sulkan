#include "include/sulkan/basic_components.h"
#include "include/sulkan/render_association.h"
#include "include/sulkan/imgui_layer.h"
#include "include/sulkan/state.h"

void skName_DrawComponent(skName* object, skECSState* state)
{
    if (skImGui_CollapsingHeader("skName"))
    {
        skImGui_InputText("name", object->name, 128, 0);
    }
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
