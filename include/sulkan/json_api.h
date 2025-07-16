#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdbool.h>
#include <cglm/cglm.h>

typedef struct skJson_t*        skJson;
typedef struct skJsonIterator_t skJsonIterator;

skJson skJson_Create();
skJson skJson_CreateArray();
void   skJson_Destroy(skJson j);

void skJson_SaveBool(skJson j, const char* name, const bool val);
void skJson_SaveString(skJson j, const char* name, const char* val);
void skJson_SaveInt(skJson j, const char* name, const int val);
void skJson_SaveFloat(skJson j, const char* name, const float val);
void skJson_SaveDouble(skJson j, const char* name, const double val);

void skJson_SaveFloat2(skJson j, const char* name, const vec2 val);
void skJson_SaveFloat3(skJson j, const char* name, const vec3 val);
void skJson_SaveFloat4(skJson j, const char* name, const vec4 val);

void skJson_SaveMat4(skJson j, const char* name, const mat4 val);

void skJson_SaveFloatArray(skJson j, const char* name,
                           const float* val, size_t size);

size_t skJson_LoadFloatArray(skJson j, const char* name, float* val);
void   skJson_LoadBool(skJson j, const char* key, bool* val);
void   skJson_LoadString(skJson j, const char* key, char* val);
void   skJson_LoadInt(skJson j, const char* key, int* val);
void   skJson_LoadFloat(skJson j, const char* key, float* val);
void   skJson_LoadDouble(skJson j, const char* key, double* val);

void skJson_LoadFloat2(skJson j, const char* key, vec2 val);
void skJson_LoadFloat3(skJson j, const char* key, vec3 val);
void skJson_LoadFloat4(skJson j, const char* key, vec4 val);

void skJson_LoadFloat16(skJson j, const char* key, mat4 val);

void skJson_PushBack(skJson j, const skJson val);

typedef void (*skJsonIteratorFunc)(skJson j);

void skJson_Iterate(skJson j, skJsonIteratorFunc sys);

skJson skJson_GetskJsonAtIndex(skJson j, int index);
int    skJson_GetskJsonArraySize(skJson j);
bool   skJson_HasKey(skJson j, const char* key);

bool   skJson_SaveToFile(skJson j, const char* filename);
skJson skJson_LoadFromFile(const char* filename);

#ifdef __cplusplus
}

#    include <nlohmann/json.hpp>

nlohmann::json skJson_GetskJson(skJson j);

void skJson_SetskJson(const skJson j, const nlohmann::json& json);

#endif
