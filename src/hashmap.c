#include <sulkan/hashmap.h>

skHashMap* skHashMap_Create(size_t keySize, size_t valueSize,
                            size_t initial_capacity)
{
    if (initial_capacity == 0)
        initial_capacity = 16; // Default capacity

    skHashMap* map = (skHashMap*)malloc(sizeof(skHashMap));
    if (map == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for hash map.\n");
        assert(1);
        return NULL;
    }

    map->buckets = (skHashMapEntry**)calloc(initial_capacity,
                                            sizeof(skHashMapEntry*));
    if (map->buckets == NULL)
    {
        free(map);
        fprintf(stderr,
                "Failed to allocate memory for hash map buckets.\n");
        assert(1);
        return NULL;
    }

    map->keySize = keySize;
    map->valueSize = valueSize;
    map->size = 0;
    map->capacity = initial_capacity;
    map->hash_func = skHashMap_DefaultHash;
    map->compare_func = skHashMap_DefaultCompare;

    return map;
}

static int skHashMap_Rehash(skHashMap* map)
{
    size_t           old_capacity = map->capacity;
    skHashMapEntry** old_buckets = map->buckets;

    // Double the capacity
    map->capacity *= 2;
    map->buckets = (skHashMapEntry**)calloc(map->capacity,
                                            sizeof(skHashMapEntry*));
    if (map->buckets == NULL)
    {
        map->buckets = old_buckets;
        map->capacity = old_capacity;
        fprintf(stderr, "Failed to rehash hash map.\n");
        assert(1);
        return -1;
    }

    size_t old_size = map->size;
    map->size = 0;

    // Re-insert all elements
    for (size_t i = 0; i < old_capacity; i++)
    {
        skHashMapEntry* entry = old_buckets[i];
        while (entry != NULL)
        {
            skHashMapEntry* next = entry->next;

            // Re-insert this entry
            size_t hash = map->hash_func(entry->key, map->keySize);
            size_t bucket_index = hash % map->capacity;

            entry->next = map->buckets[bucket_index];
            map->buckets[bucket_index] = entry;
            map->size++;

            entry = next;
        }
    }

    free(old_buckets);
    return 0;
}

int skHashMap_Insert(skHashMap* map, const void* key,
                     const void* value)
{
    if (map == NULL || key == NULL || value == NULL)
    {
        fprintf(stderr, "Invalid parameters for hash map insert.\n");
        assert(1);
        return -1;
    }

    // Check if we need to rehash (load factor > 0.75)
    if (map->size >= map->capacity * 3 / 4)
    {
        if (skHashMap_Rehash(map) != 0)
            return -1;
    }

    size_t hash = map->hash_func(key, map->keySize);
    size_t bucket_index = hash % map->capacity;

    // Check if key already exists
    skHashMapEntry* entry = map->buckets[bucket_index];
    while (entry != NULL)
    {
        if (map->compare_func(entry->key, key, map->keySize) == 0)
        {
            // Update existing value
            memcpy(entry->value, value, map->valueSize);
            return 0;
        }
        entry = entry->next;
    }

    // Create new entry
    entry = (skHashMapEntry*)malloc(sizeof(skHashMapEntry));
    if (entry == NULL)
    {
        fprintf(stderr,
                "Failed to allocate memory for hash map entry.\n");
        assert(1);
        return -1;
    }

    entry->key = malloc(map->keySize);
    entry->value = malloc(map->valueSize);
    if (entry->key == NULL || entry->value == NULL)
    {
        free(entry->key);
        free(entry->value);
        free(entry);
        fprintf(
            stderr,
            "Failed to allocate memory for hash map entry data.\n");
        assert(1);
        return -1;
    }

    memcpy(entry->key, key, map->keySize);
    memcpy(entry->value, value, map->valueSize);
    entry->next = map->buckets[bucket_index];
    map->buckets[bucket_index] = entry;
    map->size++;

    return 0;
}

void* skHashMap_Get(skHashMap* map, const void* key)
{
    if (map == NULL || key == NULL)
    {
        fprintf(stderr, "Invalid parameters for hash map get.\n");
        assert(1);
        return NULL;
    }

    size_t hash = map->hash_func(key, map->keySize);
    size_t bucket_index = hash % map->capacity;

    skHashMapEntry* entry = map->buckets[bucket_index];
    while (entry != NULL)
    {
        if (map->compare_func(entry->key, key, map->keySize) == 0)
        {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL; // Key not found
}

int skHashMap_Contains(skHashMap* map, const void* key)
{
    return skHashMap_Get(map, key) != NULL;
}

int skHashMap_Remove(skHashMap* map, const void* key)
{
    if (map == NULL || key == NULL)
    {
        fprintf(stderr, "Invalid parameters for hash map remove.\n");
        assert(1);
        return -1;
    }

    size_t hash = map->hash_func(key, map->keySize);
    size_t bucket_index = hash % map->capacity;

    skHashMapEntry* entry = map->buckets[bucket_index];
    skHashMapEntry* prev = NULL;

    while (entry != NULL)
    {
        if (map->compare_func(entry->key, key, map->keySize) == 0)
        {
            // Found the entry to remove
            if (prev == NULL)
            {
                map->buckets[bucket_index] = entry->next;
            }
            else
            {
                prev->next = entry->next;
            }

            free(entry->key);
            free(entry->value);
            free(entry);
            map->size--;
            return 0;
        }
        prev = entry;
        entry = entry->next;
    }

    return -1; // Key not found
}

size_t skHashMap_Size(skHashMap* map)
{
    return map ? map->size : 0;
}

void skHashMap_Clear(skHashMap* map)
{
    if (map == NULL)
        return;

    for (size_t i = 0; i < map->capacity; i++)
    {
        skHashMapEntry* entry = map->buckets[i];
        while (entry != NULL)
        {
            skHashMapEntry* next = entry->next;
            free(entry->key);
            free(entry->value);
            free(entry);
            entry = next;
        }
        map->buckets[i] = NULL;
    }
    map->size = 0;
}

void skHashMap_Free(skHashMap* map)
{
    if (map == NULL)
        return;

    skHashMap_Clear(map);
    free(map->buckets);
    free(map);
}

void skHashMap_SetHashFunction(skHashMap* map,
                               size_t (*hash_func)(const void* key,
                                                   size_t keySize))
{
    if (map && hash_func)
        map->hash_func = hash_func;
}

void skHashMap_SetCompareFunction(
    skHashMap* map,
    int (*compare_func)(const void* key1, const void* key2,
                        size_t keySize))
{
    if (map && compare_func)
        map->compare_func = compare_func;
}

// Example usage and specialized functions for common key types

// Hash function for string keys
size_t skHashMap_StringHash(const void* key, size_t keySize)
{
    const char* str = *(const char**)key; // key is a pointer to char*
    size_t      hash = 5381;
    int         c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

// Comparison function for string keys
int skHashMap_StringCompare(const void* key1, const void* key2,
                            size_t keySize)
{
    const char* str1 = *(const char**)key1;
    const char* str2 = *(const char**)key2;
    return strcmp(str1, str2);
}
