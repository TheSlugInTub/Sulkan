#include <sulkan/map.h>
#include <assert.h>

// FNV-1a hash function for generic data
uint32_t skMap_DefaultHash(const void* key, size_t keySize)
{
    const uint32_t FNV_PRIME = 16777619;
    const uint32_t FNV_OFFSET = 2166136261;

    uint32_t       hash = FNV_OFFSET;
    const uint8_t* bytes = (const uint8_t*)key;

    for (size_t i = 0; i < keySize; i++)
    {
        hash ^= bytes[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

// Default byte-wise comparison
int skMap_DefaultCompare(const void* key1, const void* key2,
                         size_t keySize)
{
    return memcmp(key1, key2, keySize);
}

// String hash function
uint32_t skMap_StringHash(const void* key, size_t keySize)
{
    (void)keySize; // Unused for null-terminated strings
    const char*    str = *(const char**)key;
    const uint32_t FNV_PRIME = 16777619;
    const uint32_t FNV_OFFSET = 2166136261;

    uint32_t hash = FNV_OFFSET;
    while (*str)
    {
        hash ^= (uint8_t)*str++;
        hash *= FNV_PRIME;
    }

    return hash;
}

// String comparison function
int skMap_StringCompare(const void* key1, const void* key2,
                        size_t keySize)
{
    (void)keySize; // Unused for null-terminated strings
    const char* str1 = *(const char**)key1;
    const char* str2 = *(const char**)key2;
    return strcmp(str1, str2);
}

// Integer hash function
uint32_t skMap_IntHash(const void* key, size_t keySize)
{
    (void)keySize;
    int val = *(const int*)key;
    // Simple integer hash
    val = ((val >> 16) ^ val) * 0x45d9f3b;
    val = ((val >> 16) ^ val) * 0x45d9f3b;
    val = (val >> 16) ^ val;
    return (uint32_t)val;
}

// Integer comparison function
int skMap_IntCompare(const void* key1, const void* key2,
                     size_t keySize)
{
    (void)keySize;
    int a = *(const int*)key1;
    int b = *(const int*)key2;
    return (a > b) - (a < b);
}

// Create a new hash map
skMap* skMap_Create(size_t keySize, size_t valueSize,
                    size_t initial_bucket_count, skHashFunc hash_func,
                    skKeyCompareFunc cmp_func)
{
    if (initial_bucket_count == 0)
        initial_bucket_count = 16; // Default bucket count

    skMap* map = (skMap*)malloc(sizeof(skMap));
    if (map == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for map.\n");
        assert(1);
        return NULL;
    }

    map->buckets = (skMapEntry**)calloc(initial_bucket_count,
                                        sizeof(skMapEntry*));
    if (map->buckets == NULL)
    {
        free(map);
        fprintf(stderr,
                "Failed to allocate memory for map buckets.\n");
        assert(1);
        return NULL;
    }

    map->keySize = keySize;
    map->valueSize = valueSize;
    map->bucket_count = initial_bucket_count;
    map->size = 0;
    map->hash_func = hash_func ? hash_func : skMap_DefaultHash;
    map->cmp_func = cmp_func ? cmp_func : skMap_DefaultCompare;

    return map;
}

// Create a new entry
static skMapEntry* skMapEntry_Create(const void* key,
                                     const void* value,
                                     size_t keySize, size_t valueSize)
{
    skMapEntry* entry = (skMapEntry*)malloc(sizeof(skMapEntry));
    if (entry == NULL)
        return NULL;

    entry->key = malloc(keySize);
    entry->value = malloc(valueSize);

    if (entry->key == NULL || entry->value == NULL)
    {
        free(entry->key);
        free(entry->value);
        free(entry);
        return NULL;
    }

    memcpy(entry->key, key, keySize);
    memcpy(entry->value, value, valueSize);
    entry->next = NULL;

    return entry;
}

// Free an entry
static void skMapEntry_Free(skMapEntry* entry)
{
    if (entry)
    {
        free(entry->key);
        free(entry->value);
        free(entry);
    }
}

// Insert or update a key-value pair
int skMap_Insert(skMap* map, const void* key, const void* value)
{
    if (map == NULL || key == NULL || value == NULL)
        return -1;

    uint32_t hash = map->hash_func(key, map->keySize);
    size_t   bucket_index = hash % map->bucket_count;

    skMapEntry* current = map->buckets[bucket_index];

    // Check if key already exists
    while (current != NULL)
    {
        if (map->cmp_func(current->key, key, map->keySize) == 0)
        {
            // Update existing value
            memcpy(current->value, value, map->valueSize);
            return 0;
        }
        current = current->next;
    }

    // Create new entry
    skMapEntry* new_entry =
        skMapEntry_Create(key, value, map->keySize, map->valueSize);
    if (new_entry == NULL)
    {
        fprintf(stderr, "Failed to create new map entry.\n");
        assert(1);
        return -1;
    }

    // Insert at head of bucket
    new_entry->next = map->buckets[bucket_index];
    map->buckets[bucket_index] = new_entry;
    map->size++;

    return 0;
}

// Get a value by key
void* skMap_Get(skMap* map, const void* key)
{
    if (map == NULL || key == NULL)
        return NULL;

    uint32_t hash = map->hash_func(key, map->keySize);
    size_t   bucket_index = hash % map->bucket_count;

    skMapEntry* current = map->buckets[bucket_index];

    while (current != NULL)
    {
        if (map->cmp_func(current->key, key, map->keySize) == 0)
        {
            return current->value;
        }
        current = current->next;
    }

    return NULL; // Key not found
}

// Remove a key-value pair
int skMap_Remove(skMap* map, const void* key)
{
    if (map == NULL || key == NULL)
        return -1;

    uint32_t hash = map->hash_func(key, map->keySize);
    size_t   bucket_index = hash % map->bucket_count;

    skMapEntry* current = map->buckets[bucket_index];
    skMapEntry* prev = NULL;

    while (current != NULL)
    {
        if (map->cmp_func(current->key, key, map->keySize) == 0)
        {
            // Remove this entry
            if (prev == NULL)
            {
                // First entry in bucket
                map->buckets[bucket_index] = current->next;
            }
            else
            {
                prev->next = current->next;
            }

            skMapEntry_Free(current);
            map->size--;
            return 0;
        }

        prev = current;
        current = current->next;
    }

    return -1; // Key not found
}

// Check if a key exists
int skMap_Contains(skMap* map, const void* key)
{
    return skMap_Get(map, key) != NULL;
}

// Get size
size_t skMap_Size(skMap* map)
{
    return map ? map->size : 0;
}

// Clear all elements
void skMap_Clear(skMap* map)
{
    if (map == NULL)
        return;

    for (size_t i = 0; i < map->bucket_count; i++)
    {
        skMapEntry* current = map->buckets[i];
        while (current != NULL)
        {
            skMapEntry* next = current->next;
            skMapEntry_Free(current);
            current = next;
        }
        map->buckets[i] = NULL;
    }

    map->size = 0;
}

// Free the map
void skMap_Free(skMap* map)
{
    if (map == NULL)
        return;

    skMap_Clear(map);
    free(map->buckets);
    free(map);
}
