#pragma once

#include <jolt/joltc.h>
#include <sulkan/vector.h>
#include <sulkan/essentials.h>
#include <cglm/cglm.h>
#include <sulkan/render_association.h>

#define SK_DEBUG

typedef struct skPhysics3DState
{
    JPH_JobSystem*                     jobSystem;
    JPH_ObjectLayerPairFilter*         objectLayerPairFilterTable;
    JPH_BroadPhaseLayerInterface*      broadPhaseLayerInterfaceTable;
    JPH_ObjectVsBroadPhaseLayerFilter* objectVsBroadPhaseLayerFilter;

    JPH_PhysicsSystemSettings settings;
    JPH_PhysicsSystem*        system;
    JPH_BodyInterface*        bodyInterface;
    JPH_BodyInterface*        bodyInterfaceNoLock;

    skVector* objectLayers; // JPH_ObjectLayer
    skVector* broadPhaseLayers; // JPH_BroadPhaseLayer
} skPhysics3DState;

typedef enum skCollider3DType
{
    skCollider3DType_Box,
    skCollider3DType_Capsule,
    skCollider3DType_Sphere,
    skCollider3DType_Mesh,
} skCollider3DType;

// COMPONENT
typedef struct skRigidbody3D
{
    int bodyType;     // JPH_ObjectLayer
    int colliderType; // skCollider3DType

    float mass;
    float friction;
    float linearDamping;
    float angularDamping;
    float restitution;
    bool  fixedRotation;

    vec3  boxHalfwidths;
    float sphereRadius;
    float capsuleRadius;
    float capsuleHeight;

    JPH_BodyID bodyID;

#ifdef SK_DEBUG
    int lineIndex;
#endif
} skRigidbody3D;

void             skPhysics3DTraceImpl(const char* message);
skPhysics3DState skPhysics3DState_Create();
void             skPhysics3DState_Step(skPhysics3DState* state, float dt);
void             skPhysics3DState_Destroy(skPhysics3DState* state);
void             skPhysics3DState_ClearWorld(skPhysics3DState* state);
void skPhysics3DState_CreateBody(skPhysics3DState*    state,
                                 skECSState*          ecsState,
                                 skRigidbody3D*       rigid,
                                 skRenderAssociation* assoc);

void skRigidbody3D_Sys(skECSState* state);
void skRigidbody3D_StartSys(skECSState* state);
