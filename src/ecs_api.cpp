#include <sulkan/ecs_api.h>
#include <sulkan/ecs.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstring>

// Store the Scene struct as an opaque pointer
struct skScene_t
{
    Scene cppScene;
    std::unordered_map<skComponentTypeID, int>
        componentTypeMap; // Maps our skComponentTypeID to internal
                          // IDs
    int nextInternalId;

    skScene_t()
       : nextInternalId(1) {} // Start at 1 to avoid conflicts with 0
};

// Custom structure for the entity iterator
struct skEntityIterator_t
{
    Scene*                  pScene;
    std::vector<int>        internalComponentTypes;
    size_t                  currentIndex;
    std::vector<skEntityID> matchingEntities;
    bool                    iterateAll;
};

extern "C"
{

// FNV-1a hash function for strings
unsigned int skECS_HashString(const char* str)
{
    unsigned int hash = 2166136261u; // FNV offset basis
    while (*str)
    {
        hash ^= (unsigned char)*str++;
        hash *= 16777619u; // FNV prime
    }
    return hash;
}

skSceneHandle skECS_CreateScene(void)
{
    skSceneHandle handle = new skScene_t();
    return handle;
}

void skECS_DestroyScene(skSceneHandle scene)
{
    if (scene)
    {
        scene->cppScene.Clear();
        delete scene;
    }
}

void skECS_ClearScene(skSceneHandle scene)
{
    if (scene)
    {
        scene->cppScene.Clear();
        scene->componentTypeMap.clear();
        scene->nextInternalId = 1;
    }
}

int skECS_EntityCount(skSceneHandle scene)
{
    if (scene)
    {
        return scene->cppScene.entities.size();
    }
    return 0;
}

skEntityID skECS_AddEntity(skSceneHandle scene)
{
    if (!scene)
        return SK_ECS_INVALID_ENTITY;
    return scene->cppScene.AddEntity();
}

void skECS_DestroyEntity(skSceneHandle scene, skEntityID entity)
{
    if (scene && IsEntityValid(entity))
    {
        scene->cppScene.DestroyEntity(entity);
    }
}

skEntityID skECS_CloneEntity(skSceneHandle scene,
                             skEntityID    sourceEntity)
{
    if (!scene || !IsEntityValid(sourceEntity))
        return SK_ECS_INVALID_ENTITY;
    return scene->cppScene.CloneEntity(sourceEntity);
}

bool skECS_IsEntityValid(skEntityID entity)
{
    return IsEntityValid(entity);
}

skEntityID skECS_GetEntityAtIndex(skSceneHandle scene, int index)
{
    return scene->cppScene.entities[index].id;
}

// Get or create the internal component ID for a component type
static int
GetOrCreateInternalComponentId(skSceneHandle     scene,
                               skComponentTypeID externalTypeId,
                               size_t            componentSize)
{
    // Check if we already have this component type registered
    auto it = scene->componentTypeMap.find(externalTypeId);
    if (it != scene->componentTypeMap.end())
    {
        return it->second;
    }

    // Otherwise, create a new internal ID
    int internalId = scene->nextInternalId++;
    scene->componentTypeMap[externalTypeId] = internalId;

    // Ensure the component pools are large enough
    if (scene->cppScene.componentPools.size() <= internalId)
    {
        scene->cppScene.componentPools.resize(internalId + 1,
                                              nullptr);
    }

    // Create the component pool if it doesn't exist
    if (!scene->cppScene.componentPools[internalId])
    {
        scene->cppScene.componentPools[internalId] =
            new ComponentPool(componentSize);
    }

    return internalId;
}

void* skECS_AssignComponent(skSceneHandle scene, skEntityID entity,
                            skComponentTypeID componentTypeId,
                            size_t            componentSize)
{
    if (!scene || !IsEntityValid(entity))
        return NULL;

    // Get the internal component ID, creating it if necessary
    int internalId = GetOrCreateInternalComponentId(
        scene, componentTypeId, componentSize);

    Scene*      pScene = &scene->cppScene;
    EntityIndex entityIndex = GetEntityIndex(entity);

    // Check if the entity already has this component
    if (entityIndex < pScene->entities.size() &&
        pScene->entities[entityIndex].mask.test(internalId))
    {
        // Component already exists, just return the pointer
        return pScene->componentPools[internalId]->get(entityIndex);
    }

    // Component doesn't exist, create it
    void* componentMemory =
        pScene->componentPools[internalId]->get(entityIndex);

    // Initialize the memory to zero
    std::memset(componentMemory, 0, componentSize);

    // Set the component bit
    pScene->entities[entityIndex].mask.set(internalId);

    return componentMemory;
}

void* skECS_GetComponent(skSceneHandle scene, skEntityID entity,
                         skComponentTypeID componentTypeId)
{

#ifdef SK_DEBUG_LEVEL_1
    if (!scene || !IsEntityValid(entity))
        return NULL;
#endif
    
    auto it = scene->componentTypeMap.find(componentTypeId);

#ifdef SK_DEBUG_LEVEL_1
    if (it == scene->componentTypeMap.end())
    {
        return NULL; // Component type not registered
    }
#endif

    int         internalId = it->second;
    Scene*      pScene = &scene->cppScene;
    EntityIndex entityIndex = GetEntityIndex(entity);

#ifdef SK_DEBUG_LEVEL_2
    // Check if the entity has this component
    if (entityIndex >= pScene->entities.size() ||
        !pScene->entities[entityIndex].mask.test(internalId) ||
        internalId >= pScene->componentPools.size() ||
        !pScene->componentPools[internalId])
    {
        return NULL;
    }
#endif

    return pScene->componentPools[internalId]->get(entityIndex);
}

void skECS_RemoveComponent(skSceneHandle scene, skEntityID entity,
                           skComponentTypeID componentTypeId)
{
    if (!scene || !IsEntityValid(entity))
        return;

    // Check if we know this component type
    auto it = scene->componentTypeMap.find(componentTypeId);
    if (it == scene->componentTypeMap.end())
    {
        return; // Component type not registered
    }

    int         internalId = it->second;
    Scene*      pScene = &scene->cppScene;
    EntityIndex entityIndex = GetEntityIndex(entity);

    // Check if the entity is valid
    if (entityIndex >= pScene->entities.size() ||
        pScene->entities[entityIndex].id != entity)
    {
        return;
    }

    // Reset the component bit
    pScene->entities[entityIndex].mask.reset(internalId);
}

void skECS_AddSystem(skSystemFunction system, 
                     bool isStartSystem)
{
    AddSystem(system, isStartSystem);
}

void skECS_UpdateSystems(void)
{
    UpdateSystems();
}

void skECS_StartStartSystems(void)
{
    StartStartSystems();
}

// Simple object pool for iterators
class IteratorPool
{
  private:
    struct PoolNode
    {
        skEntityIterator_t iterator;
        bool               inUse;
    };

    std::vector<PoolNode> pool;

  public:
    IteratorPool(size_t initialSize = 64)
    {
        pool.resize(initialSize);
        for (auto& node : pool) { node.inUse = false; }
    }

    skEntityIterator_t* allocate()
    {
        // Find a free slot
        for (auto& node : pool)
        {
            if (!node.inUse)
            {
                node.inUse = true;
                return &node.iterator;
            }
        }

        // Expand pool if all slots are used
        size_t oldSize = pool.size();
        pool.resize(oldSize * 2);
        for (size_t i = oldSize; i < pool.size(); i++)
        {
            pool[i].inUse = false;
        }

        pool[oldSize].inUse = true;
        return &pool[oldSize].iterator;
    }

    void deallocate(skEntityIterator_t* iterator)
    {
        for (auto& node : pool)
        {
            if (&node.iterator == iterator)
            {
                // Reset iterator state
                node.iterator.currentIndex = 0;
                node.iterator.iterateAll = false;
                node.iterator.internalComponentTypes.clear();
                node.iterator.matchingEntities.clear();
                node.iterator.pScene = nullptr;

                // Mark as available
                node.inUse = false;
                return;
            }
        }
    }
};

// Global pool
static IteratorPool g_iteratorPool;

skEntityIteratorHandle
skECS_CreateEntityIterator(skSceneHandle            scene,
                           const skComponentTypeID* componentTypeIds,
                           int                      componentCount)
{
    if (!scene)
        return NULL;

    skEntityIterator_t* iterator = g_iteratorPool.allocate();
    iterator->pScene = &scene->cppScene;
    iterator->currentIndex = 0;
    iterator->iterateAll = false;

    // Pre-allocate vectors to avoid reallocations
    iterator->internalComponentTypes.reserve(componentCount);
    // Pre-allocate with a reasonable estimate of matching entities
    iterator->matchingEntities.reserve(
        scene->cppScene.entities.size() /
        4); // Estimate 25% will match

    // Map external component types to internal IDs
    for (int i = 0; i < componentCount; i++)
    {
        auto it = scene->componentTypeMap.find(componentTypeIds[i]);
        if (it != scene->componentTypeMap.end())
        {
            iterator->internalComponentTypes.push_back(it->second);
        }
    }

    // If no component types specified or any component type wasn't
    // found
    if (componentCount > 0 &&
        iterator->internalComponentTypes.size() != componentCount)
    {
        return iterator; // Will return no entities
    }

    // Build component mask
    ComponentMask mask;
    for (int internalId : iterator->internalComponentTypes)
    {
        mask.set(internalId);
    }

    // Pre-collect matching entities for iteration
    const auto&  entities = iterator->pScene->entities;
    const size_t entSize = entities.size();
    for (size_t i = 0; i < entSize; i++)
    {
        const Scene::EntityDesc& desc = entities[i];
        if (IsEntityValid(desc.id) && mask == (mask & desc.mask))
        {
            iterator->matchingEntities.push_back(desc.id);
        }
    }

    return iterator;
}

skEntityIteratorHandle
skECS_CreateAllEntityIterator(skSceneHandle scene)
{
    if (!scene)
        return NULL;

    skEntityIterator_t* iterator = new skEntityIterator_t();
    iterator->pScene = &scene->cppScene;
    iterator->currentIndex = 0;
    iterator->iterateAll = true;

    // Pre-allocate with a reasonable estimate
    iterator->matchingEntities.reserve(
        scene->cppScene.entities.size());

    // Pre-collect valid entities
    const auto&  entities = iterator->pScene->entities;
    const size_t entSize = entities.size();
    for (size_t i = 0; i < entSize; i++)
    {
        const Scene::EntityDesc& desc = entities[i];
        if (IsEntityValid(desc.id))
        {
            iterator->matchingEntities.push_back(desc.id);
        }
    }

    return iterator;
}

skEntityID skECS_IteratorNext(skEntityIteratorHandle iterator)
{
    if (!iterator)
        return SK_ECS_INVALID_ENTITY;

    if (iterator->currentIndex < iterator->matchingEntities.size())
    {
        return iterator->matchingEntities[iterator->currentIndex++];
    }

    return SK_ECS_INVALID_ENTITY;
}

void skECS_DestroyEntityIterator(skEntityIteratorHandle iterator)
{
    if (iterator)
    {
        g_iteratorPool.deallocate(iterator);
    }
}

} // extern "C"
