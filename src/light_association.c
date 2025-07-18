#include <sulkan/light_association.h>
#include <sulkan/state.h>

void skLightAssociation_CreateLight(skLightAssociation* assoc,
        skECSState* state)
{
    skLight* lightPtr = (skLight*)skVector_Get(state->renderer->lights, 
            assoc->lightIndex);

    if (lightPtr != NULL)
    {
        return;
    }

    skLight light = {0};
    glm_vec3_copy(assoc->position, light.position);
    glm_vec3_copy(assoc->color, light.color);
    light.intensity = assoc->intensity;
    light.radius = assoc->radius;

    assoc->lightIndex = state->renderer->lights->size;
    skRenderer_AddLight(state->renderer, &light);
}

void skLightAssociation_StartSys(skECSState* state)
{
    SK_ECS_ITER_START(state->scene,
                      SK_ECS_COMPONENT_TYPE(skLightAssociation))
    {
        skLightAssociation* assoc =
            SK_ECS_GET(state->scene, _entity, skLightAssociation);

        skLightAssociation_CreateLight(assoc, state);
    }
    SK_ECS_ITER_END();
}
