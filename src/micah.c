#include "../micah.h"

// micah.c
// Procedurally generated source file for the Sulkan game engine
// This contains drawer/serializer/deserializer functions for all
// registered components

void skName_DrawComponent(skName* object, skECSState* state,
                          skEntityID ent)
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

// IGNORE
void skRenderAssociation_DrawComponent(skRenderAssociation* object,
                                       skECSState*          state,
                                       skEntityID           ent)
{
    if (skImGui_CollapsingHeader("skRenderAssociation"))
    {
        if (skImGui_Button(
                "Create skRenderObject for this skRenderAssociation"))
        {
            skRenderAssociation_CreateRenderObject(object, state);
        }

        if (skImGui_Button("Remove skRenderObject"))
        {
            skVector_Remove(state->renderer->renderObjects,
                            object->objectIndex);
        }

        skImGui_InputInt("objectIndex", &object->objectIndex);

        const char* types[] = {"Model", "Sprite"};
        int         currentType = (int)object->type;

        if (skImGui_ComboBox("renderObjectType", types, &currentType,
                             2))
        {
            object->type = currentType;
        }

        if (object->type == skRenderObjectType_Model)
        {
            skImGui_InputText("modelPath", object->modelPath, 128, 0);
            skImGui_InputText("texturePath", object->texturePath, 128,
                              0);
            skImGui_InputText("normalTexturePath",
                              object->normalTexturePath, 128, 0);
            skImGui_InputText("roughnessTexturePath",
                              object->roughnessTexturePath, 128, 0);

            if (skImGui_Button("Update Model"))
            {
                skModel model = skModel_Create();
                skModel_Load(&model, object->modelPath);

                skRenderObject* obj = (skRenderObject*)skVector_Get(
                    state->renderer->renderObjects,
                    object->objectIndex);
                *obj = skRenderObject_CreateFromModel(
                    state->renderer, &model, 0, object->texturePath,
                    object->normalTexturePath,
                    object->roughnessTexturePath);

                VkDeviceSize bufferSize =
                    sizeof(skUniformBufferObject);

                for (int frame = 0; frame < SK_FRAMES_IN_FLIGHT;
                     frame++)
                {
                    skRenderer_CreateBuffer(
                        state->renderer, bufferSize,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        &obj->uniformBuffers[frame],
                        &obj->uniformBuffersMemory[frame]);

                    vkMapMemory(state->renderer->device,
                                obj->uniformBuffersMemory[frame], 0,
                                bufferSize, 0,
                                &obj->uniformBuffersMap[frame]);
                }

                skRenderer_CreateDescriptorSetsForObject(
                    state->renderer, obj);

                mat4 trans = GLM_MAT4_IDENTITY_INIT;
                glm_translate(trans, object->position);
                glm_quat_rotate(trans, object->rotation, trans);
                glm_scale(trans, object->scale);

                glm_mat4_copy(trans, obj->transform);
            }
        }
        if (object->type == skRenderObjectType_Sprite)
        {
            skImGui_InputText("texturePath", object->texturePath, 128,
                              0);
            skImGui_InputText("normalTexturePath",
                              object->normalTexturePath, 128, 0);
            skImGui_InputText("roughnessTexturePath",
                              object->roughnessTexturePath, 128, 0);

            if (skImGui_Button("Update Sprite"))
            {
                skRenderObject* obj = (skRenderObject*)skVector_Get(
                    state->renderer->renderObjects,
                    object->objectIndex);
                *obj = skRenderObject_CreateFromSprite(
                    state->renderer, object->texturePath,
                    object->normalTexturePath,
                    object->roughnessTexturePath);

                VkDeviceSize bufferSize =
                    sizeof(skUniformBufferObject);

                for (int frame = 0; frame < SK_FRAMES_IN_FLIGHT;
                     frame++)
                {
                    skRenderer_CreateBuffer(
                        state->renderer, bufferSize,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        &obj->uniformBuffers[frame],
                        &obj->uniformBuffersMemory[frame]);

                    vkMapMemory(state->renderer->device,
                                obj->uniformBuffersMemory[frame], 0,
                                bufferSize, 0,
                                &obj->uniformBuffersMap[frame]);
                }

                skRenderer_CreateDescriptorSetsForObject(
                    state->renderer, obj);
            }
        }

        skRenderObject* obj = (skRenderObject*)skVector_Get(
            state->renderer->renderObjects, object->objectIndex);

        if (obj != NULL)
        {
            if ((skImGui_DragFloat3("position", object->position,
                                    0.1f) ||
                 skImGui_DragFloat4("rotation", object->rotation,
                                    0.1f) ||
                 skImGui_DragFloat3("scale", object->scale, 0.1f)))
            {
                mat4 trans = GLM_MAT4_IDENTITY_INIT;
                glm_translate(trans, object->position);
                glm_quat_normalize(object->rotation);
                glm_quat_rotate(trans, object->rotation, trans);
                glm_scale(trans, object->scale);

                glm_mat4_copy(trans, obj->transform);
            }
        }
    }
}

skJson skRenderAssociation_SaveComponent(skRenderAssociation* object)
{
    skJson j = skJson_Create();

    skJson_SaveInt(j, "objectIndex", object->objectIndex);
    skJson_SaveInt(j, "type", object->type);
    skJson_SaveString(j, "modelPath", object->modelPath);
    skJson_SaveString(j, "texturePath", object->texturePath);
    skJson_SaveString(j, "normalTexturePath",
                      object->normalTexturePath);
    skJson_SaveString(j, "roughnessTexturePath",
                      object->roughnessTexturePath);
    skJson_SaveFloat3(j, "position", object->position);
    skJson_SaveFloat4(j, "rotation", object->rotation);
    skJson_SaveFloat3(j, "scale", object->scale);
    return j;
}

void skRenderAssociation_LoadComponent(skRenderAssociation* object,
                                       skJson               j)
{
    skJson_LoadInt(j, "objectIndex", &object->objectIndex);
    skJson_LoadInt(j, "type", &object->type);
    skJson_LoadString(j, "modelPath", object->modelPath);
    skJson_LoadString(j, "texturePath", object->texturePath);
    skJson_LoadString(j, "normalTexturePath",
                      object->normalTexturePath);
    skJson_LoadString(j, "roughnessTexturePath",
                      object->roughnessTexturePath);
    skJson_LoadFloat3(j, "position", object->position);
    skJson_LoadFloat4(j, "rotation", object->rotation);
    skJson_LoadFloat3(j, "scale", object->scale);
}

// IGNORE
void skLightAssociation_DrawComponent(skLightAssociation* object,
                                      skECSState*         state,
                                      skEntityID          ent)
{
    if (skImGui_CollapsingHeader("skLightAssociation"))
    {
        if (skImGui_Button("Create skLight for this association"))
        {
            skLightAssociation_CreateLight(object, state);
        }

        if (skImGui_Button("Destroy skLight"))
        {
            skVector_Remove(state->renderer->lights,
                            object->lightIndex);
        }

        skImGui_InputInt("lightIndex", &object->lightIndex);

        if (skImGui_DragFloat3("position", object->position, 0.1f) ||
            skImGui_DragFloat3("color", object->color, 0.1f) ||
            skImGui_DragFloat("radius", &object->radius, 0.1f) ||
            skImGui_DragFloat("intensity", &object->intensity, 0.1f))
        {
            skLight* light = (skLight*)skVector_Get(
                state->renderer->lights, object->lightIndex);
            light->intensity = object->intensity;
            light->radius = object->radius;
            glm_vec3_copy(object->position, light->position);
            glm_vec3_copy(object->color, light->color);
        }
    }
}

skJson skLightAssociation_SaveComponent(skLightAssociation* object)
{
    skJson j = skJson_Create();

    skJson_SaveInt(j, "lightIndex", object->lightIndex);
    skJson_SaveFloat3(j, "position", object->position);
    skJson_SaveFloat3(j, "color", object->color);
    skJson_SaveFloat(j, "radius", object->radius);
    skJson_SaveFloat(j, "intensity", object->intensity);
    return j;
}

void skLightAssociation_LoadComponent(skLightAssociation* object,
                                      skJson              j)
{
    skJson_LoadInt(j, "lightIndex", &object->lightIndex);
    skJson_LoadFloat3(j, "position", object->position);
    skJson_LoadFloat3(j, "color", object->color);
    skJson_LoadFloat(j, "radius", &object->radius);
    skJson_LoadFloat(j, "intensity", &object->intensity);
}

// IGNORE
void skRigidbody3D_DrawComponent(skRigidbody3D* object,
                                 skECSState* state, skEntityID ent)
{
    if (skImGui_CollapsingHeader("skRigidbody3D"))
    {
        skImGui_InputInt("bodyType", &object->bodyType);
        skImGui_InputInt("colliderType", &object->colliderType);
        skImGui_DragFloat("mass", &object->mass, 0.1f);
        skImGui_DragFloat("friction", &object->friction, 0.1f);
        skImGui_DragFloat("linearDamping", &object->linearDamping,
                          0.1f);
        skImGui_DragFloat("angularDamping", &object->angularDamping,
                          0.1f);
        skImGui_DragFloat("restitution", &object->restitution, 0.1f);
        skImGui_DragFloat3("boxHalfwidths", object->boxHalfwidths,
                           0.1f);
        skImGui_DragFloat("sphereRadius", &object->sphereRadius,
                          0.1f);
        skImGui_DragFloat("capsuleRadius", &object->capsuleRadius,
                          0.1f);
        skImGui_DragFloat("capsuleHeight", &object->capsuleHeight,
                          0.1f);
        skImGui_Checkbox("created", &object->created);

        if (skImGui_Button(
                "Create/Update Jolt body for this skRigidbody3D"))
        {
            skRenderAssociation* assoc =
                SK_ECS_GET(state->scene, ent, skRenderAssociation);

            skModel model = skModel_Create();
            skModel_Load(&model, assoc->modelPath);

            skPhysics3DState_CreateBody(state->physics3dState, state,
                                        object, assoc, &model);
            object->created = true;
        }

        if (skImGui_Button("Update Jolt body for this skRigidbody3D"))
        {
            skRenderAssociation* assoc =
                SK_ECS_GET(state->scene, ent, skRenderAssociation);

            skModel model = skModel_Create();
            skModel_Load(&model, assoc->modelPath);

            skPhysics3DState_DestroyBody(state->physics3dState,
                                         state->renderer, object);
            skPhysics3DState_CreateBody(state->physics3dState, state,
                                        object, assoc, &model);
        }
    }
}

skJson skRigidbody3D_SaveComponent(skRigidbody3D* object)
{
    skJson j = skJson_Create();

    skJson_SaveInt(j, "bodyType", object->bodyType);
    skJson_SaveInt(j, "colliderType", object->colliderType);
    skJson_SaveFloat(j, "mass", object->mass);
    skJson_SaveFloat(j, "friction", object->friction);
    skJson_SaveFloat(j, "linearDamping", object->linearDamping);
    skJson_SaveFloat(j, "angularDamping", object->angularDamping);
    skJson_SaveFloat(j, "restitution", object->restitution);
    skJson_SaveBool(j, "fixedRotation", object->fixedRotation);
    skJson_SaveFloat3(j, "boxHalfwidths", object->boxHalfwidths);
    skJson_SaveFloat(j, "sphereRadius", object->sphereRadius);
    skJson_SaveFloat(j, "capsuleRadius", object->capsuleRadius);
    skJson_SaveFloat(j, "capsuleHeight", object->capsuleHeight);
    return j;
}

void skRigidbody3D_LoadComponent(skRigidbody3D* object, skJson j)
{
    skJson_LoadInt(j, "bodyType", &object->bodyType);
    skJson_LoadInt(j, "colliderType", &object->colliderType);
    skJson_LoadFloat(j, "mass", &object->mass);
    skJson_LoadFloat(j, "friction", &object->friction);
    skJson_LoadFloat(j, "linearDamping", &object->linearDamping);
    skJson_LoadFloat(j, "angularDamping", &object->angularDamping);
    skJson_LoadFloat(j, "restitution", &object->restitution);
    skJson_LoadBool(j, "fixedRotation", &object->fixedRotation);
    skJson_LoadFloat3(j, "boxHalfwidths", object->boxHalfwidths);
    skJson_LoadFloat(j, "sphereRadius", &object->sphereRadius);
    skJson_LoadFloat(j, "capsuleRadius", &object->capsuleRadius);
    skJson_LoadFloat(j, "capsuleHeight", &object->capsuleHeight);
}

void Micah_DrawAllComponents(skECSState* state, skEntityID ent)
{
    skName* skNameObj = SK_ECS_GET(state->scene, ent, skName);

    if (skNameObj != NULL)
    {
        skName_DrawComponent(skNameObj, state, ent);
    }

    skRenderAssociation* skRenderAssociationObj =
        SK_ECS_GET(state->scene, ent, skRenderAssociation);

    if (skRenderAssociationObj != NULL)
    {
        skRenderAssociation_DrawComponent(skRenderAssociationObj,
                                          state, ent);
    }

    skLightAssociation* skLightAssociationObj =
        SK_ECS_GET(state->scene, ent, skLightAssociation);

    if (skLightAssociationObj != NULL)
    {
        skLightAssociation_DrawComponent(skLightAssociationObj, state,
                                         ent);
    }

    skRigidbody3D* skRigidbody3DObj =
        SK_ECS_GET(state->scene, ent, skRigidbody3D);

    if (skRigidbody3DObj != NULL)
    {
        skRigidbody3D_DrawComponent(skRigidbody3DObj, state, ent);
    }
}

skJson Micah_SaveAllComponents(skECSState* state)
{
    skJson j = skJson_Create();

    for (int i = 0; i < skECS_EntityCount(state->scene); i++)
    {
        skEntityID ent = skECS_GetEntityAtIndex(state->scene, i);
        skJson     entJson = skJson_Create();

        if (!skECS_IsEntityValid(ent))
        {
            continue;
        }

        skName* skNameObj = SK_ECS_GET(state->scene, ent, skName);

        if (skNameObj != NULL)
        {
            skJson compJson = skName_SaveComponent(skNameObj);
            skJson_SaveString(compJson, "componentType", "skName");
            skJson_PushBack(entJson, compJson);
            skJson_Destroy(compJson);
        }

        skRenderAssociation* skRenderAssociationObj =
            SK_ECS_GET(state->scene, ent, skRenderAssociation);

        if (skRenderAssociationObj != NULL)
        {
            skJson compJson = skRenderAssociation_SaveComponent(
                skRenderAssociationObj);
            skJson_SaveString(compJson, "componentType",
                              "skRenderAssociation");
            skJson_PushBack(entJson, compJson);
            skJson_Destroy(compJson);
        }

        skLightAssociation* skLightAssociationObj =
            SK_ECS_GET(state->scene, ent, skLightAssociation);

        if (skLightAssociationObj != NULL)
        {
            skJson compJson = skLightAssociation_SaveComponent(
                skLightAssociationObj);
            skJson_SaveString(compJson, "componentType",
                              "skLightAssociation");
            skJson_PushBack(entJson, compJson);
            skJson_Destroy(compJson);
        }

        skRigidbody3D* skRigidbody3DObj =
            SK_ECS_GET(state->scene, ent, skRigidbody3D);

        if (skRigidbody3DObj != NULL)
        {
            skJson compJson =
                skRigidbody3D_SaveComponent(skRigidbody3DObj);
            skJson_SaveString(compJson, "componentType",
                              "skRigidbody3D");
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
        skJson     entJson = skJson_GetArrayElement(j, i);
        skEntityID ent = skECS_AddEntity(state->scene);

        int componentCount = skJson_GetArraySize(entJson);

        for (int compIndex = 0; compIndex < componentCount;
             compIndex++)
        {
            skJson compJson =
                skJson_GetArrayElement(entJson, compIndex);

            char componentType[256];
            skJson_LoadString(compJson, "componentType",
                              componentType);

            // Check component type and assign/load accordingly
            if (false) {
            } // Dummy condition for cleaner generated code
            else if (strcmp(componentType, "skName") == 0)
            {
                skName* comp =
                    SK_ECS_ASSIGN(state->scene, ent, skName);
                skName_LoadComponent(comp, compJson);
            }
            else if (strcmp(componentType, "skRenderAssociation") ==
                     0)
            {
                skRenderAssociation* comp = SK_ECS_ASSIGN(
                    state->scene, ent, skRenderAssociation);
                skRenderAssociation_LoadComponent(comp, compJson);
            }
            else if (strcmp(componentType, "skLightAssociation") == 0)
            {
                skLightAssociation* comp = SK_ECS_ASSIGN(
                    state->scene, ent, skLightAssociation);
                skLightAssociation_LoadComponent(comp, compJson);
            }
            else if (strcmp(componentType, "skRigidbody3D") == 0)
            {
                skRigidbody3D* comp =
                    SK_ECS_ASSIGN(state->scene, ent, skRigidbody3D);
                skRigidbody3D_LoadComponent(comp, compJson);
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
        if (skImGui_MenuItem("skLightAssociation"))
        {
            SK_ECS_ASSIGN(state->scene, ent, skLightAssociation);
        }
        if (skImGui_MenuItem("skRigidbody3D"))
        {
            SK_ECS_ASSIGN(state->scene, ent, skRigidbody3D);
        }

        skImGui_EndPopup();
    }
}
