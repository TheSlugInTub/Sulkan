#pragma once

#include <bitset>
#include <vector>

// This file implements a basic ECS which is the core of the engine

// Counter to keep track of all the components
inline int componentCounter = 0;

// Max constants
const int MAX_COMPONENTS = 200;
const int MAX_ENTITIES = 1000;

// Typedefs to aid in reading
typedef std::bitset<MAX_COMPONENTS> ComponentMask;
typedef unsigned long long          EntityID;
typedef unsigned int                EntityIndex;
typedef unsigned int                EntityVersion;

// Functions for the validation of entities and entity versions,
// please don't use these in your code

inline EntityID CreateEntityId(EntityIndex   index,
                               EntityVersion version)
{
    // Shift the index up 32, and put the version in the bottom
    return ((EntityID)index << 32) | ((EntityID)version);
}
inline EntityIndex GetEntityIndex(EntityID id)
{
    // Shift down 32 so we lose the version and get our index
    return id >> 32;
}
inline EntityVersion GetEntityVersion(EntityID id)
{
    // Cast to a 32 bit int to get our version number (loosing the top
    // 32 bits)
    return (EntityVersion)id;
}
inline bool IsEntityValid(EntityID id)
{
    // Check if the index is our invalid index
    return (id >> 32) != EntityIndex(-1);
}

#define INVALID_ENTITY CreateEntityId(EntityIndex(-1), 0)

// Gets the id of a component
template<class T> int GetId()
{
    static int componentId = componentCounter++;
    return componentId;
}

// Memory pool for the components
// Just stores chars
struct ComponentPool
{
    ComponentPool(size_t elementsize)
    {
        // We'll allocate enough memory to hold MAX_ENTITIES, each
        // with element size
        elementSize = elementsize;
        pData = new char[elementSize * MAX_ENTITIES];
    }

    ~ComponentPool() { delete[] pData; }

    inline void* get(size_t index)
    {
        // looking up the component at the desired index
        return pData + index * elementSize;
    }

    char*  pData {nullptr};
    size_t elementSize {0};
};

// Scene struct, holds all the entities, basically a registry of
// entities
struct Scene
{
    // Struct to store all the information needed for an entity
    struct EntityDesc
    {
        EntityID      id;
        ComponentMask mask;
    };

    // Creates an entity in the scene
    EntityID AddEntity()
    {
        if (!freeEntities.empty())
        {
            EntityIndex newIndex = freeEntities.back();
            freeEntities.pop_back();
            EntityID newID = CreateEntityId(
                newIndex, GetEntityVersion(entities[newIndex].id));
            entities[newIndex].id = newID;
            return entities[newIndex].id;
        }

        entities.push_back(
            {CreateEntityId(EntityIndex(entities.size()), 0),
             ComponentMask()});
        return entities.back().id;
    }

    // Assigns a component to an entity ID
    template<typename T> T* Assign(EntityID id)
    {
        int componentId = GetId<T>();

        if (componentPools.size() <=
            componentId) // Not enough component pool
        {
            componentPools.resize(componentId + 1, nullptr);
        }
        if (componentPools[componentId] ==
            nullptr) // New component, make a new pool
        {
            componentPools[componentId] =
                new ComponentPool(sizeof(T));
        }

        // Looks up the component in the pool, and initializes it with
        // placement new
        T* pComponent =
            new (componentPools[componentId]->get(GetEntityIndex(id)))
                T();

        // Set the bit for this component to true and return the
        // created component
        entities[GetEntityIndex(id)].mask.set(componentId);
        return pComponent;
    }

    // Assigns a component to an entity ID with a list of parameters
    // (a constructor). use: <ent, 1.0f, 2.0f...>
    template<typename T, typename... Args>
    T* AssignParam(EntityID id, Args&&... args)
    {
        int componentId = GetId<T>();

        if (componentPools.size() <=
            componentId) // Not enough component pool
        {
            componentPools.resize(componentId + 1, nullptr);
        }
        if (componentPools[componentId] ==
            nullptr) // New component, make a new pool
        {
            componentPools[componentId] =
                new ComponentPool(sizeof(T));
        }

        // Look up the component in the pool, and use placement new to
        // initialize it with the provided arguments
        T* pComponent =
            new (componentPools[componentId]->get(GetEntityIndex(id)))
                T(std::forward<Args>(args)...);

        // Set the bit for this component to true and return the
        // created component
        entities[GetEntityIndex(id)].mask.set(componentId);
        return pComponent;
    }

    // Retrieves a pointer to a given component from an entity id,
    // use: Get<Type>(ent)
    template<typename T> T* Get(EntityID id)
    {
        int componentId = GetId<T>();
        if (!entities[GetEntityIndex(id)].mask.test(componentId))
            return nullptr;

        T* pComponent = static_cast<T*>(
            componentPools[componentId]->get(GetEntityIndex(id)));
        return pComponent;
    }

    // Removes a component from an entity ID
    template<typename T> void Remove(EntityID id)
    {
        // ensures you're not accessing an entity that has been
        // deleted
        if (entities[GetEntityIndex(id)].id != id)
            return;

        int componentId = GetId<T>();
        entities[GetEntityIndex(id)].mask.reset(componentId);
    }

    // Destroys an entity, resets its mask, adds the given entity's
    // index to the list of free entities
    void DestroyEntity(EntityID id)
    {
        EntityID newID =
            CreateEntityId(EntityIndex(-1), GetEntityVersion(id) + 1);
        entities[GetEntityIndex(id)].id = newID;
        entities[GetEntityIndex(id)].mask.reset();
        freeEntities.push_back(GetEntityIndex(id));
    }

    // Clears all entities
    void Clear()
    {
        // First destroy all entities to ensure proper component
        // cleanup
        for (const EntityDesc& desc : entities)
        {
            if (IsEntityValid(desc.id))
            {
                DestroyEntity(desc.id);
            }
        }

        // Clear entity list and free entities list
        entities.clear();
        freeEntities.clear();

        // Clean up component pools
        for (ComponentPool* pool : componentPools) { delete pool; }
        componentPools.clear();
    }

    EntityID CloneEntity(EntityID sourceId)
    {
        // Ensure source entity is valid
        if (!IsEntityValid(sourceId) ||
            GetEntityIndex(sourceId) >= entities.size() ||
            entities[GetEntityIndex(sourceId)].id != sourceId)
        {
            return INVALID_ENTITY;
        }

        // Create new entity
        EntityID newId = AddEntity();

        // Get the component mask of the source entity
        const ComponentMask& sourceMask =
            entities[GetEntityIndex(sourceId)].mask;

        // For each set bit in the mask (each component)
        for (size_t i = 0; i < MAX_COMPONENTS; i++)
        {
            if (sourceMask.test(i))
            {
                // Ensure we have enough component pools
                if (componentPools.size() <= i)
                {
                    componentPools.resize(i + 1, nullptr);
                }

                if (ComponentPool* pool = componentPools[i])
                {
                    // Get pointers to source and destination memory
                    char* sourceComponent = static_cast<char*>(
                        pool->get(GetEntityIndex(sourceId)));
                    char* destComponent = static_cast<char*>(
                        pool->get(GetEntityIndex(newId)));

                    // Copy the component data
                    std::memcpy(destComponent, sourceComponent,
                                pool->elementSize);

                    // Set the component bit for the new entity
                    entities[GetEntityIndex(newId)].mask.set(i);
                }
            }
        }

        return newId;
    }

    std::vector<EntityDesc>     entities;
    std::vector<EntityIndex>    freeEntities;
    std::vector<ComponentPool*> componentPools;
};

struct skECSState;

using SysFunc = void (*)(skECSState*);

// All the systems that are registered
inline std::vector<SysFunc> systems;
inline std::vector<SysFunc> startSystems;

// Adds a system to the list of systems
inline void AddSystem(SysFunc sys, bool startSys)
{
    if (startSys)
    {
        startSystems.push_back(sys);
    }
    else
    {
        systems.push_back(sys);
    }
}

// Updates all the systems by calling them, call this function every
// frame to update all the systems each frame
inline void UpdateSystems(skECSState* state)
{
    for (auto system : systems) { system(state); }
}

// Updates all the start systems, call this only on the start of the
// program
inline void StartStartSystems(skECSState* state)
{
    for (auto system : startSystems) { system(state); }
}

/*
This struct is used to make it easier to iterate through a list of
entities with the components that you specify It looks like this '
SceneView<Transform, Info>(scene) '
*/

template<typename... ComponentTypes> struct SceneView
{
    SceneView(Scene& scene) : pScene(&scene)
    {
        if (sizeof...(ComponentTypes) == 0)
        {
            all = true;
        }
        else
        {
            // Unpack the template parameters into an initializer list
            int componentIds[] = {0, GetId<ComponentTypes>()...};

            for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
                componentMask.set(componentIds[i]);
        }
    }

    struct Iterator
    {
        Iterator(Scene* pScene, EntityIndex index, ComponentMask mask,
                 bool all)
           : pScene(pScene), index(index), mask(mask), all(all)
        {
        }

        EntityID operator*() const
        {
            return pScene->entities[index].id;
        }

        bool operator==(const Iterator& other) const
        {
            return index == other.index ||
                   index == pScene->entities.size();
        }

        bool operator!=(const Iterator& other) const
        {
            return index != other.index &&
                   index != pScene->entities.size();
        }

        bool ValidIndex()
        {
            return
                // It's a valid entity ID
                IsEntityValid(pScene->entities[index].id) &&
                // It has the correct component mask
                (all ||
                 mask == (mask & pScene->entities[index].mask));
        }

        Iterator& operator++()
        {
            do {
                index++;
            } while (index < pScene->entities.size() &&
                     !ValidIndex());
            return *this;
        }

        EntityIndex   index;
        Scene*        pScene;
        ComponentMask mask;
        bool          all {false};
    };

    const Iterator begin() const
    {
        int firstIndex = 0;
        while (
            firstIndex < pScene->entities.size() &&
            (componentMask != (componentMask &
                               pScene->entities[firstIndex].mask) ||
             !IsEntityValid(pScene->entities[firstIndex].id)))
        {
            firstIndex++;
        }
        return Iterator(pScene, firstIndex, componentMask, all);
    }

    const Iterator end() const
    {
        return Iterator(pScene, EntityIndex(pScene->entities.size()),
                        componentMask, all);
    }

    Scene*        pScene {nullptr};
    ComponentMask componentMask;
    bool          all {false};
};

// Macro for registering systems outside the main function
#define REGISTER_SYSTEM(scriptClass)            \
    static bool scriptClass##_registered = []() \
    {                                           \
        AddSystem(scriptClass, false, false);   \
        return true;                            \
    }();

// Macro for registering start systems outside the main function
#define REGISTER_START_SYSTEM(scriptClass)      \
    static bool scriptClass##_registered = []() \
    {                                           \
        AddSystem(scriptClass, false, true);    \
        return true;                            \
    }();
