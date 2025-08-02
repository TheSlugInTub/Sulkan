#include <sulkan/physics_3d.h>
#include <sulkan/state.h>

void skPhysics3DTraceImpl(const char* message)
{
    printf("JOLT ERROR: %s\n", message);
}

skPhysics3DState skPhysics3DState_Create()
{
    skPhysics3DState state = {0};

    state.objectLayers = skVector_Create(sizeof(JPH_ObjectLayer), 2);
    state.broadPhaseLayers =
        skVector_Create(sizeof(JPH_BroadPhaseLayer), 2);

    JPH_ObjectLayer nonMovingLayer = 0;
    JPH_ObjectLayer movingLayer = 1;

    if (!JPH_Init())
    {
        printf("Jolt physics (Physics3D) failed to initialize\n");
    }

    JPH_SetTraceHandler(skPhysics3DTraceImpl);

    state.jobSystem = JPH_JobSystemThreadPool_Create(NULL);

    state.objectLayerPairFilterTable =
        JPH_ObjectLayerPairFilterTable_Create(2);

    JPH_ObjectLayerPairFilterTable_EnableCollision(
        state.objectLayerPairFilterTable, nonMovingLayer,
        movingLayer);

    JPH_ObjectLayerPairFilterTable_EnableCollision(
        state.objectLayerPairFilterTable, movingLayer,
        nonMovingLayer);

    JPH_ObjectLayerPairFilterTable_EnableCollision(
        state.objectLayerPairFilterTable, movingLayer, movingLayer);

    // We use a 1-to-1 mapping between object layers and broadphase
    // layers
    state.broadPhaseLayerInterfaceTable =
        JPH_BroadPhaseLayerInterfaceTable_Create(2, 2);

    // Map object layers to broad phase layers (COMPLETE MAPPING)
    JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(
        state.broadPhaseLayerInterfaceTable, nonMovingLayer,
        nonMovingLayer);
    JPH_BroadPhaseLayerInterfaceTable_MapObjectToBroadPhaseLayer(
        state.broadPhaseLayerInterfaceTable, movingLayer,
        movingLayer);

    // Create object vs broad phase layer filter table with complete
    // mapping
    state.objectVsBroadPhaseLayerFilter =
        JPH_ObjectVsBroadPhaseLayerFilterTable_Create(
            state.broadPhaseLayerInterfaceTable, 2,
            state.objectLayerPairFilterTable, 2);

    state.settings.maxBodies = 65536;
    state.settings.numBodyMutexes = 0;
    state.settings.maxBodyPairs = 65536;
    state.settings.maxContactConstraints = 65536;
    state.settings.broadPhaseLayerInterface =
        state.broadPhaseLayerInterfaceTable;
    state.settings.objectLayerPairFilter =
        state.objectLayerPairFilterTable;
    state.settings.objectVsBroadPhaseLayerFilter =
        state.objectVsBroadPhaseLayerFilter;
    state.system = JPH_PhysicsSystem_Create(&state.settings);

    JPH_Vec3 gravity = (JPH_Vec3) {0.0f, -9.81f, 0.0f};
    JPH_PhysicsSystem_SetGravity(state.system, &gravity);

    state.bodyInterface =
        JPH_PhysicsSystem_GetBodyInterface(state.system);
    state.bodyInterfaceNoLock =
        JPH_PhysicsSystem_GetBodyInterfaceNoLock(state.system);

    return state;
}

void skPhysics3DState_Step(skPhysics3DState* state, float dt)
{
    const int cCollisionSteps = 1;

    JPH_PhysicsSystem_Update(state->system, dt, cCollisionSteps,
                             state->jobSystem);
}

void skPhysics3DState_Destroy(skPhysics3DState* state)
{
}

void skPhysics3DState_ClearWorld(skPhysics3DState* state)
{
}

void skPhysics3DState_CreateBody(skPhysics3DState*    state,
                                 skRigidbody3D*       rigid,
                                 skRenderAssociation* assoc)
{
    switch (rigid->colliderType)
    {
        case skCollider3DType_Box:
        {
            JPH_Vec3 halfExtents = {rigid->boxHalfwidths[0],
                                    rigid->boxHalfwidths[1],
                                    rigid->boxHalfwidths[2]};
            JPH_Vec3 position = {assoc->position[0],
                                 assoc->position[1],
                                 assoc->position[2]};

            JPH_BoxShape* shape = JPH_BoxShape_Create(
                &halfExtents, JPH_DEFAULT_CONVEX_RADIUS);

            vec4 glmQuat;
            glm_vec4_copy(assoc->rotation, glmQuat);

            JPH_Quat quat = (JPH_Quat) {glmQuat[0], glmQuat[1],
                                        glmQuat[2], glmQuat[3]};

            JPH_BodyCreationSettings* settings =
                JPH_BodyCreationSettings_Create3(
                    (const JPH_Shape*)shape, &position, &quat,
                    rigid->bodyType == 0 ? JPH_MotionType_Static
                                         : JPH_MotionType_Dynamic,
                    (int)rigid->bodyType);

            JPH_MassProperties msp = {};

            // Lock rotation
            if (rigid->fixedRotation)
            {
                JPH_BodyCreationSettings_SetAllowedDOFs(
                    settings, JPH_AllowedDOFs_TranslationX |
                                  JPH_AllowedDOFs_TranslationY |
                                  JPH_AllowedDOFs_TranslationZ);
            }

            JPH_MassProperties_ScaleToMass(&msp, rigid->mass);
            JPH_BodyCreationSettings_SetMassPropertiesOverride(
                settings, &msp);
            JPH_BodyCreationSettings_SetOverrideMassProperties(
                settings,
                JPH_OverrideMassProperties_CalculateInertia);

            rigid->bodyID = JPH_BodyInterface_CreateAndAddBody(
                state->bodyInterface, settings,
                rigid->bodyType == 0 ? JPH_Activation_DontActivate
                                     : JPH_Activation_Activate);

            JPH_BodyCreationSettings_Destroy(settings);
            break;
        }
        case skCollider3DType_Sphere:
        {
            JPH_Vec3 position = {assoc->position[0],
                                 assoc->position[1],
                                 assoc->position[2]};

            JPH_SphereShape* shape =
                JPH_SphereShape_Create(rigid->sphereRadius);

            JPH_BodyCreationSettings* settings =
                JPH_BodyCreationSettings_Create3(
                    (const JPH_Shape*)shape, &position, NULL,
                    rigid->bodyType == 0 ? JPH_MotionType_Static
                                         : JPH_MotionType_Dynamic,
                    rigid->bodyType);

            JPH_MassProperties msp = {};
            JPH_MassProperties_ScaleToMass(&msp, rigid->mass);

            // Lock rotation
            if (rigid->fixedRotation)
            {
                JPH_BodyCreationSettings_SetAllowedDOFs(
                    settings, JPH_AllowedDOFs_TranslationX |
                                  JPH_AllowedDOFs_TranslationY |
                                  JPH_AllowedDOFs_TranslationZ);
            }

            JPH_BodyCreationSettings_SetMassPropertiesOverride(
                settings, &msp);
            JPH_BodyCreationSettings_SetOverrideMassProperties(
                settings,
                JPH_OverrideMassProperties_CalculateInertia);

            rigid->bodyID = JPH_BodyInterface_CreateAndAddBody(
                state->bodyInterface, settings,
                rigid->bodyType == 0 ? JPH_Activation_DontActivate
                                     : JPH_Activation_Activate);

            JPH_BodyCreationSettings_Destroy(settings);
            break;
        }
        case skCollider3DType_Capsule:
        {
            JPH_Vec3 position = {assoc->position[0],
                                 assoc->position[1],
                                 assoc->position[2]};

            // Create capsule shape
            // Parameters: radius, half height (from center to end of
            // capsule)
            JPH_CapsuleShape* shape = JPH_CapsuleShape_Create(
                rigid->capsuleRadius,       // radius
                rigid->capsuleHeight * 0.5f // half height
            );

            // Quaternion for rotation
            vec4 glmQuat;
            glm_vec4_copy(assoc->rotation, glmQuat);

            JPH_Quat quat = (JPH_Quat) {glmQuat[0], glmQuat[1],
                                        glmQuat[2], glmQuat[3]};

            // Create body creation settings
            JPH_BodyCreationSettings* settings =
                JPH_BodyCreationSettings_Create3(
                    (const JPH_Shape*)shape, &position, &quat,
                    rigid->bodyType == 0 ? JPH_MotionType_Static
                                         : JPH_MotionType_Dynamic,
                    rigid->bodyType);

            // Set mass properties
            JPH_MassProperties msp = {};
            JPH_MassProperties_ScaleToMass(&msp, rigid->mass);

            // Lock rotation
            if (rigid->fixedRotation)
            {
                JPH_BodyCreationSettings_SetAllowedDOFs(
                    settings, JPH_AllowedDOFs_TranslationX |
                                  JPH_AllowedDOFs_TranslationY |
                                  JPH_AllowedDOFs_TranslationZ);
            }

            JPH_BodyCreationSettings_SetMassPropertiesOverride(
                settings, &msp);
            JPH_BodyCreationSettings_SetOverrideMassProperties(
                settings,
                JPH_OverrideMassProperties_CalculateInertia);

            // Create and add body
            rigid->bodyID = JPH_BodyInterface_CreateAndAddBody(
                state->bodyInterface, settings,
                rigid->bodyType == 0 ? JPH_Activation_DontActivate
                                     : JPH_Activation_Activate);

            // Clean up
            JPH_BodyCreationSettings_Destroy(settings);
            break;
        }
        case skCollider3DType_Mesh:
        {
            // TODO
            // JPH_Vec3 position = {assoc->position[0],
            //                      assoc->position[1],
            //                      assoc->position[2]};

            // // Convert rotation to JPH_Quat
            // vec4 glmQuat;
            // glm_euler_xyz_quat(assoc->rotation, glmQuat);
            // JPH_Quat quat = (JPH_Quat) {glmQuat[0], glmQuat[1],
            //                             glmQuat[2], glmQuat[3]};

            // // Collect all triangles from all meshes
            // size_t totalTriangles = 0;
            // for (size_t meshIdx = 0; meshIdx < model->meshes->size;
            //      meshIdx++)
            // {
            //     skMesh* mesh =
            //         (skMesh*)smVector_Get(assoc->->meshes,
            //         meshIdx);
            //     totalTriangles += mesh->indices->size / 3;
            // }

            // if (totalTriangles == 0)
            //     break;

            // JPH_Triangle* triangles = (JPH_Triangle*)malloc(
            //     sizeof(JPH_Triangle) * totalTriangles);
            // if (!triangles)
            //     break;

            // size_t triangleIndex = 0;
            // for (size_t meshIdx = 0; meshIdx < model->meshes->size;
            //      meshIdx++)
            // {
            //     smMesh* mesh =
            //         (smMesh*)smVector_Get(model->meshes, meshIdx);
            //     smVector* vertices = mesh->vertices;
            //     smVector* indices = mesh->indices;

            //     size_t numIndices = indices->size;
            //     size_t numTriangles = numIndices / 3;

            //     for (size_t i = 0; i < numTriangles; i++)
            //     {
            //         unsigned int idx0 =
            //             *(unsigned int*)smVector_Get(indices, i *
            //             3);
            //         unsigned int idx1 = *(unsigned
            //         int*)smVector_Get(
            //             indices, i * 3 + 1);
            //         unsigned int idx2 = *(unsigned
            //         int*)smVector_Get(
            //             indices, i * 3 + 2);

            //         smVertex* v0 =
            //             (smVertex*)smVector_Get(vertices, idx0);
            //         smVertex* v1 =
            //             (smVertex*)smVector_Get(vertices, idx1);
            //         smVertex* v2 =
            //             (smVertex*)smVector_Get(vertices, idx2);

            //         triangles[triangleIndex].v1.x =
            //         v0->position[0]; triangles[triangleIndex].v1.y
            //         = v0->position[1];
            //         triangles[triangleIndex].v1.z =
            //         v0->position[2]; triangles[triangleIndex].v2.x
            //         = v1->position[0];
            //         triangles[triangleIndex].v2.y =
            //         v1->position[1]; triangles[triangleIndex].v2.z
            //         = v1->position[2];
            //         triangles[triangleIndex].v3.x =
            //         v2->position[0]; triangles[triangleIndex].v3.y
            //         = v2->position[1];
            //         triangles[triangleIndex].v3.z =
            //         v2->position[2];
            //         triangles[triangleIndex].materialIndex = 0;
            //         triangleIndex++;
            //     }
            // }

            // // Create mesh shape
            // JPH_MeshShapeSettings* meshSettings =
            //     JPH_MeshShapeSettings_Create(
            //         triangles, (uint32_t)totalTriangles);

            // JPH_MeshShapeSettings_Sanitize(meshSettings);
            // JPH_MeshShape* shape =
            //     JPH_MeshShapeSettings_CreateShape(meshSettings);
            // free(triangles);

            // if (!shape)
            //     break;

            // // Create body creation settings
            // JPH_BodyCreationSettings* settings =
            //     JPH_BodyCreationSettings_Create3(
            //         (const JPH_Shape*)shape, &position, &quat,
            //         JPH_MotionType_Static, rigid->bodyType);

            // // Set mass properties (for static bodies, mass is
            // // ignored)
            // JPH_MassProperties msp = {};
            // JPH_MassProperties_ScaleToMass(&msp, rigid->mass);
            // JPH_BodyCreationSettings_SetMassPropertiesOverride(
            //     settings, &msp);
            // JPH_BodyCreationSettings_SetOverrideMassProperties(
            //     settings,
            //     JPH_OverrideMassProperties_CalculateInertia);

            // // Create and add body
            // rigid->bodyID = JPH_BodyInterface_CreateAndAddBody(
            //     skCollider3DType_state.bodyInterface, settings,
            //     JPH_Activation_DontActivate);

            // // Clean up
            // JPH_BodyCreationSettings_Destroy(settings);
            break;
        }
    }
}

void skRigidbody3D_Sys(skECSState* state)
{
    SK_ECS_ITER_START(state->scene,
                      SK_ECS_COMPONENT_TYPE(skRigidbody3D))
    {
        skRigidbody3D* rigid =
            SK_ECS_GET(state->scene, _entity, skRigidbody3D);
        skRenderAssociation* assoc =
            SK_ECS_GET(state->scene, _entity, skRenderAssociation);

        JPH_RVec3 pos;
        JPH_Quat  rot;
        JPH_BodyInterface_GetPositionAndRotation(
            state->physics3dState->bodyInterface, rigid->bodyID, &pos,
            &rot);

        skRenderObject* obj = (skRenderObject*)skVector_Get(
            state->renderer->renderObjects, assoc->objectIndex);

        assoc->position[0] = pos.x;
        assoc->position[1] = pos.y;
        assoc->position[2] = pos.z;

        assoc->rotation[0] = rot.x;
        assoc->rotation[1] = rot.y;
        assoc->rotation[2] = rot.z;
        assoc->rotation[3] = rot.w;

        mat4 trans = GLM_MAT4_IDENTITY_INIT;
        glm_translate(trans, assoc->position);
        glm_quat_rotate(trans, assoc->rotation, trans);
        glm_scale(trans, assoc->scale);

        glm_mat4_copy(trans, obj->transform);
    }
    SK_ECS_ITER_END();
}

void skRigidbody3D_StartSys(skECSState* state)
{
    SK_ECS_ITER_START(state->scene,
                      SK_ECS_COMPONENT_TYPE(skRigidbody3D))
    {
        skRigidbody3D* rigid =
            SK_ECS_GET(state->scene, _entity, skRigidbody3D);
        skRenderAssociation* assoc =
            SK_ECS_GET(state->scene, _entity, skRenderAssociation);

        skPhysics3DState_CreateBody(state->physics3dState, rigid,
                                    assoc);
    }
    SK_ECS_ITER_END();
}
