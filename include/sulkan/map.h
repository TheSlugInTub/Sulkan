#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Hash map entry (node in the linked list)
typedef struct skMapEntry
{
    void* key;
    void* value;
    struct skMapEntry* next;
} skMapEntry;

// Hash function pointer type
typedef uint32_t (*skHashFunc)(const void* key, size_t keySize);

// Key comparison function pointer type
typedef int (*skKeyCompareFunc)(const void* key1, const void* key2, size_t keySize);

typedef struct
{
    skMapEntry** buckets;    // Array of bucket heads
    size_t keySize;          // Size of key type
    size_t valueSize;        // Size of value type
    size_t bucket_count;     // Number of buckets
    size_t size;             // Number of elements in the map
    skHashFunc hash_func;    // Hash function
    skKeyCompareFunc cmp_func; // Key comparison function
} skMap;

// Initialize the map
skMap* skMap_Create(size_t keySize, size_t valueSize, size_t initial_bucket_count,
                    skHashFunc hash_func, skKeyCompareFunc cmp_func);

// Insert or update a key-value pair
int skMap_Insert(skMap* map, const void* key, const void* value);

// Get a value by key (returns pointer to value, or NULL if not found)
void* skMap_Get(skMap* map, const void* key);

// Remove a key-value pair
int skMap_Remove(skMap* map, const void* key);

// Check if a key exists in the map
int skMap_Contains(skMap* map, const void* key);

// Get the number of elements in the map
size_t skMap_Size(skMap* map);

// Clear all elements from the map
void skMap_Clear(skMap* map);

// Free the memory used by the map
void skMap_Free(skMap* map);

// Default hash function for generic data
uint32_t skMap_DefaultHash(const void* key, size_t keySize);

// Default comparison function for generic data
int skMap_DefaultCompare(const void* key1, const void* key2, size_t keySize);

// Convenience functions for common types
uint32_t skMap_StringHash(const void* key, size_t keySize);
int skMap_StringCompare(const void* key1, const void* key2, size_t keySize);
uint32_t skMap_IntHash(const void* key, size_t keySize);
int skMap_IntCompare(const void* key1, const void* key2, size_t keySize);
