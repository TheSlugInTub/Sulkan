#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdbool.h>

// Opaque pointer types
typedef struct skScene_t*  skSceneHandle;
typedef unsigned long long skEntityID;

// Constants
#define SK_ECS_MAX_COMPONENTS 200
#define SK_ECS_MAX_ENTITIES   1000
#define SK_ECS_INVALID_ENTITY ((skEntityID) - 1)

// Scene management
skSceneHandle skECS_CreateScene(void);
void          skECS_DestroyScene(skSceneHandle scene);
void          skECS_ClearScene(skSceneHandle scene);
int           skECS_EntityCount(skSceneHandle);

// Entity management
skEntityID skECS_AddEntity(skSceneHandle scene);
void skECS_DestroyEntity(skSceneHandle scene, skEntityID entity);
skEntityID skECS_CloneEntity(skSceneHandle scene,
                             skEntityID    sourceEntity);
bool       skECS_IsEntityValid(skEntityID entity);
skEntityID skECS_GetEntityAtIndex(skSceneHandle scene, int index);

// Component management
// We now automatically register components by size and hash
typedef size_t skComponentTypeID;

// Get a component type ID from a component type name (using string
// hashing) This is a macro that generates a unique ID for each
// component type at compile time
#define SK_ECS_COMPONENT_TYPE(component_type)                 \
    ((skComponentTypeID)((sizeof(component_type) << 16) |     \
                         (skECS_HashString(#component_type) & \
                          0xFFFF)))

#define SK_ECS_COMPONENT_TYPE_N(component_name, size)        \
    ((skComponentTypeID)((size << 16) |                      \
                         (skECS_HashString(component_name) & \
                          0xFFFF)))

// String hashing function (exposed for the macro above)
unsigned int skECS_HashString(const char* str);

// Generic component operations
void* skECS_AssignComponent(skSceneHandle scene, skEntityID entity,
                            skComponentTypeID componentTypeId,
                            size_t            componentSize);
void* skECS_GetComponent(skSceneHandle scene, skEntityID entity,
                         skComponentTypeID componentTypeId);
void  skECS_RemoveComponent(skSceneHandle scene, skEntityID entity,
                            skComponentTypeID componentTypeId);

// Convenience macros for component operations with automatic type
// handling
#define SK_ECS_ASSIGN(scene, entity, component_type)          \
    ((component_type*)skECS_AssignComponent(                  \
        scene, entity, SK_ECS_COMPONENT_TYPE(component_type), \
        sizeof(component_type)))

#define SK_ECS_ASSIGN_N(scene, entity, component_name, size) \
    (skECS_AssignComponent(                                  \
        scene, entity,                                       \
        SK_ECS_COMPONENT_TYPE_N(component_name, size), size))

#define SK_ECS_GET(scene, entity, component_type) \
    ((component_type*)skECS_GetComponent(         \
        scene, entity, SK_ECS_COMPONENT_TYPE(component_type)))

#define SK_ECS_GET_N(scene, entity, component_name, size) \
    (skECS_GetComponent(                                  \
        scene, entity,                                    \
        SK_ECS_COMPONENT_TYPE_N(component_name, size)))

#define SK_ECS_REMOVE(scene, entity, component_type) \
    skECS_RemoveComponent(scene, entity,             \
                          SK_ECS_COMPONENT_TYPE(component_type))

#define SK_ECS_REMOVE_N(scene, entity, component_name, size) \
    skECS_RemoveComponent(                                   \
        scene, entity,                                       \
        SK_ECS_COMPONENT_TYPE_N(component_name, size))

typedef struct skECSState skECSState;

// System management
typedef void (*skSystemFunction)(skECSState*);
void skECS_AddSystem(skSystemFunction system,
                     bool isStartSystem);
void skECS_UpdateSystems(skECSState* state);
void skECS_StartStartSystems(skECSState* state);

// Entity iteration
typedef struct skEntityIterator_t* skEntityIteratorHandle;

// Creates an iterator for entities with all specified component types
skEntityIteratorHandle
                       skECS_CreateEntityIterator(skSceneHandle            scene,
                                                  const skComponentTypeID* componentTypeIds,
                                                  int                      componentCount);
skEntityIteratorHandle skECS_CreateAllEntityIterator(
    skSceneHandle scene); // Iterates all valid entities
skEntityID skECS_IteratorNext(
    skEntityIteratorHandle iterator); // Returns skECS_INVALID_ENTITY
                                      // when no more entities
void skECS_DestroyEntityIterator(skEntityIteratorHandle iterator);

// Helper macro for creating iterators with specific component types
#define SK_ECS_ITER_START(scene, ...)                              \
    do {                                                           \
        skComponentTypeID      _types[] = {__VA_ARGS__};           \
        skEntityIteratorHandle _iter = skECS_CreateEntityIterator( \
            scene, _types, sizeof(_types) / sizeof(_types[0]));    \
        skEntityID _entity;                                        \
        while ((_entity = skECS_IteratorNext(_iter)) !=            \
               SK_ECS_INVALID_ENTITY)                              \
        {

#define SK_ECS_ITER_END()               \
    }                                   \
    skECS_DestroyEntityIterator(_iter); \
    }                                   \
    while (0)

#ifdef __cplusplus
}
#endif
