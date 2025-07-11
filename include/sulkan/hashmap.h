#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Hash table entry (key-value pair)
typedef struct skHashMapEntry
{
    void* key;
    void* value;
    struct skHashMapEntry* next; // For collision resolution (chaining)
} skHashMapEntry;

typedef struct
{
    skHashMapEntry** buckets;    // Array of bucket pointers
    size_t keySize;              // Size of key type
    size_t valueSize;            // Size of value type
    size_t size;                 // Current number of key-value pairs
    size_t capacity;             // Number of buckets
    size_t (*hash_func)(const void* key, size_t keySize); // Hash function
    int (*compare_func)(const void* key1, const void* key2, size_t keySize); // Key comparison function
} skHashMap;

// Default hash function (simple hash for any data type)
static size_t skHashMap_DefaultHash(const void* key, size_t keySize)
{
    const unsigned char* bytes = (const unsigned char*)key;
    size_t hash = 5381;
    for (size_t i = 0; i < keySize; i++)
    {
        hash = ((hash << 5) + hash) + bytes[i];
    }
    return hash;
}

// Default comparison function (memcmp)
static int skHashMap_DefaultCompare(const void* key1, 
                                    const void* key2,
                                    size_t keySize)
{
    return memcmp(key1, key2, keySize);
}

// Initialize the hash map
skHashMap* skHashMap_Create(size_t keySize, size_t valueSize, size_t initial_capacity);

// Insert or update a key-value pair
int skHashMap_Insert(skHashMap* map, const void* key, const void* value);

// Get a value by key (returns pointer to value or NULL if not found)
void* skHashMap_Get(skHashMap* map, const void* key);

// Check if a key exists in the map
int skHashMap_Contains(skHashMap* map, const void* key);

// Remove a key-value pair by key
int skHashMap_Remove(skHashMap* map, const void* key);

// Get the number of key-value pairs
size_t skHashMap_Size(skHashMap* map);

// Clear all elements (keeps allocated memory)
void skHashMap_Clear(skHashMap* map);

// Free the memory used by the hash map
void skHashMap_Free(skHashMap* map);

// Set custom hash and comparison functions
void skHashMap_SetHashFunction(skHashMap* map, size_t (*hash_func)(const void* key, size_t keySize));
void skHashMap_SetCompareFunction(skHashMap* map, int (*compare_func)(const void* key1, const void* key2, size_t keySize));
