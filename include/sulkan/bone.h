#pragma once

#include <sulkan/vector.h>
#include <cglm/cglm.h>
#include <assimp/scene.h>

typedef struct skKeyPosition
{
    vec3 position;
    float timeStamp;
} skKeyPosition;

typedef struct skKeyRotation
{
    vec4 rotation; // Quaternion
    float timeStamp;
} skKeyRotation;

typedef struct skKeyScale
{
    vec3 scale;
    float timeStamp;
} skKeyScale;

typedef struct skBone
{
    skVector* positions; // skPosition
    skVector* rotations; // skRotation
    skVector* scales; // skScale
    int numPositions;
    int numRotations;
    int numScales;

    mat4 localTransform;
    char name[128];
    int ID;
} skBone;

skBone skBone_Create(const char* name, int ID, const struct aiNodeAnim* channel);
void skBone_Update(skBone* bone, float animationTime);

// Get the current position key frame we're at
int skBone_GetPositionIndex(skBone* bone, float animationTime);
// Get the current rotation key frame we're at
int skBone_GetRotationIndex(skBone* bone, float animationTime);
// Get the current scale key frame we're at
int skBone_GetScaleIndex(skBone* bone, float animationTime);

float skGetScaleFactor(float lastTimeStamp, float nextTimeStamp,
                       float animationTime);

void skBone_InterpolatePosition(skBone* bone, float animationTime,
                                mat4 dest);
void skBone_InterpolateRotation(skBone* bone, float animationTime,
                                mat4 dest);
void skBone_InterpolateScale(skBone* bone, float animationTime,
                               mat4 dest);
