#include <sulkan/render_association.h>
#include <sulkan/state.h>

void skRenderAssociation_CreateRenderObject(
    skRenderAssociation* assoc, skECSState* state)
{
    skRenderObject obj;

    if (assoc->type == skRenderObjectType_Model)
    {
        skModel model = skModel_Create();
        skModel_Load(&model, assoc->modelPath);

        obj = skRenderObject_CreateFromModel(state->renderer, &model,
                                             assoc->texturePath, 
                                             assoc->normalTexturePath,
                                             assoc->roughnessTexturePath);
    }
    else if (assoc->type == skRenderObjectType_Sprite)
    {
        obj = skRenderObject_CreateFromSprite(state->renderer,
                                              assoc->texturePath,
                                              assoc->normalTexturePath,
                                              assoc->roughnessTexturePath);
    }

    mat4 trans = GLM_MAT4_IDENTITY_INIT;
    glm_translate(trans, assoc->position);
    glm_quat_rotate(trans, assoc->rotation, trans);
    glm_scale(trans, assoc->scale);

    glm_mat4_copy(trans, obj.transform);

    assoc->objectIndex = state->renderer->renderObjects->size;
    
    skRenderer_AddRenderObject(state->renderer, &obj);
}

void skRenderAssociation_StartSys(skECSState* state)
{
    SK_ECS_ITER_START(state->scene,
                      SK_ECS_COMPONENT_TYPE(skRenderAssociation))
    {
        skRenderAssociation* assoc =
            SK_ECS_GET(state->scene, _entity, skRenderAssociation);

        skRenderAssociation_CreateRenderObject(assoc, state);
    }
    SK_ECS_ITER_END();
}
