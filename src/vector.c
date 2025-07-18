#include <sulkan/vector.h>
#include <assert.h>

// Initialize a new vector with specified element size
skVector* skVector_Create(size_t elemSize, size_t initial_capacity)
{
    skVector* vector = (skVector*)malloc(sizeof(skVector));
    if (vector == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for vector.\n");
        assert(1);
        return NULL;
    }

    vector->data = malloc(initial_capacity * elemSize);
    if (vector->data == NULL)
    {
        free(vector);
        fprintf(stderr,
                "Failed to allocate memory for vector data.\n");
        assert(1);
        return NULL;
    }

    vector->elemSize = elemSize;
    vector->size = 0;
    vector->capacity = initial_capacity;
    return vector;
}

// Add an element to the vector (copies the element)
int skVector_PushBack(skVector* vector, const void* element)
{
    // Resize if needed
    if (vector->size >= vector->capacity)
    {
        size_t new_capacity = vector->capacity * 2;
        void*  new_data =
            realloc(vector->data, new_capacity * vector->elemSize);
        if (new_data == NULL)
        {
            fprintf(stderr,
                    "Failed to reallocate memory for vector data.\n");
            assert(1);
            return -1;
        }

        vector->data = new_data;
        vector->capacity = new_capacity;
    }

    // Calculate position to insert and copy the element
    char* target =
        (char*)vector->data + (vector->size * vector->elemSize);
    memcpy(target, element, vector->elemSize);

    vector->size++;
    return 0;
}

// Get a pointer to an element
void* skVector_Get(skVector* vector, size_t index)
{
    if (index >= vector->size)
    {
        fprintf(stderr, "Index out of bounds.\n");
        assert(1);
        return NULL;
    }
    return (char*)vector->data + (index * vector->elemSize);
}

int skVector_Resize(skVector* vector, size_t new_size)
{
    if (vector == NULL)
    {
        fprintf(stderr, "Vector is NULL.\n");
        assert(1);
        return -1;
    }

    // If we need more capacity, reallocate
    if (new_size > vector->capacity)
    {
        size_t new_capacity = new_size;
        // Optionally use growth factor for future allocations
        if (new_capacity < vector->capacity * 2)
        {
            new_capacity = vector->capacity * 2;
        }

        void* new_data =
            realloc(vector->data, new_capacity * vector->elemSize);
        if (new_data == NULL)
        {
            fprintf(
                stderr,
                "Failed to reallocate memory for vector resize.\n");
            assert(1);
            return -1;
        }

        vector->data = new_data;
        vector->capacity = new_capacity;
    }

    // Update the size
    vector->size = new_size;
    return 0;
}

// Remove an element by index (shifts subsequent elements)
int skVector_Remove(skVector* vector, size_t index)
{
    if (index >= vector->size)
    {
        fprintf(stderr, "Index out of bounds.\n");
        assert(1);
        return -1;
    }

    // Calculate position to remove
    char* pos_to_remove =
        (char*)vector->data + (index * vector->elemSize);

    // Calculate position of element after the one to remove
    char* next_pos = pos_to_remove + vector->elemSize;

    // Calculate number of bytes to move
    size_t bytes_to_move =
        (vector->size - index - 1) * vector->elemSize;

    // Shift elements
    if (bytes_to_move > 0)
    {
        memmove(pos_to_remove, next_pos, bytes_to_move);
    }

    vector->size--;
    return 0;
}

// Free the memory used by the vector
void skVector_Free(skVector* vector)
{
    free(vector->data);
    free(vector);
}

void skVector_Clear(skVector* vector)
{
    vector->size = 0;
}
