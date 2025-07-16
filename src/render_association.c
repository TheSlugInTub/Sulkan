#include <sulkan/render_association.h>
#include <sulkan/state.h>

void skRenderAssociation_StartSys(skECSState* state)
{
    SK_ECS_ITER_START(state->scene,
                      SK_ECS_COMPONENT_TYPE(skRenderAssociation))
    {
        skRenderAssociation* assoc =
            SK_ECS_GET(state->scene, _entity, skRenderAssociation);

        // mat4 trans = GLM_MAT4_IDENTITY_INIT;
        // glm_translate(trans, assoc->position);
        // glm_quat_rotate(trans, assoc->rotation, trans);
        // glm_scale(trans, assoc->scale);

        // skRenderObject* obj = (skRenderObject*)skVector_Get(
        //     state->renderer->renderObjects, assoc->objectIndex);
        // glm_mat4_copy(trans, obj->transform);
    }
    SK_ECS_ITER_END();
}
