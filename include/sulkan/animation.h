#pragma once

#include <sulkan/vector.h>
#include <sulkan/map.h>
#include <cglm/cglm.h>
#include <sulkan/model.h>
#include <sulkan/bone.h>

typedef struct skAssimpNodeData
{
    mat4 transformation;
    char name[128];
    int childrenCount;
    skVector* children; // skAssimpNodeData
} skAssimpNodeData;

typedef struct skAnimation
{
    skVector* bones; // skBone
    skMap* boneInfoMap; // char*, skBoneInfo
    float duration;
    int ticksPerSecond;
    skAssimpNodeData rootNode;
} skAnimation;

skAnimation skAnimation_Create(const char* animationPath, skModel* model);
void skAnimation_Free(skAnimation* animation);
skBone* skAnimation_FindBone(skAnimation* animation, const char* name);

void skAnimation_ReadMissingBones(skAnimation* animation, 
        const struct aiAnimation* aiAnim, skModel* model);
void skAnimation_ReadHierarchyData(skAssimpNodeData* dest, const struct aiNode* src);
void skAssimpNodeData_Free(skAssimpNodeData* nodeData);

typedef struct skAnimator
{
    skVector* finalBoneMatrices; // mat4
    skAnimation* currentAnimation;
    float currentTime;
    float deltaTime;
} skAnimator;

skAnimator skAnimator_Create(skAnimation* animation);
void skAnimator_UpdateAnimation(skAnimator* animator, float dt);
void skAnimator_PlayAnimation(skAnimator* animator, skAnimation* anim);
void skAnimator_CalculateBoneTransform(skAnimator* animator, skAssimpNodeData* node, 
        mat4 parentTransform);
