#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    void** data; // Array of void pointers to store any type
    size_t elemSize;
    size_t size;     // Current number of elements in the vector
    size_t capacity; // Amount of data allocated for the vector
} skVector;

// Initialize the vector
skVector* skVector_Create(size_t elemSize, size_t initial_capacity);

// Push an element into a vector
int skVector_PushBack(skVector* vector, const void* element);

// Get the data at an index of the vector
void* skVector_Get(skVector* vector, size_t index);

// Remove an element from the vector by index
int skVector_Remove(skVector* vector, size_t index);

// Free the memory used by the vector
void skVector_Free(skVector* vector);

// Set the size to zero, the memory remains allocated for future use
void skVector_Clear(skVector* vector);
