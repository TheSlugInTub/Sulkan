#include <sulkan/animation.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/matrix4x4.h>
#include <assimp/vector3.h>

skAnimation skAnimation_Create(const struct aiAnimation* aiAnim,
                               const struct aiNode* rootNode,
                               skModel*    model)
{
    skAnimation animation = {0};

    animation.bones = skVector_Create(sizeof(skBone), 16);
    animation.boneInfoMap = model->boneInfoMap;

    animation.duration = (float)aiAnim->mDuration;
    animation.ticksPerSecond = (int)aiAnim->mTicksPerSecond;

    strcpy(animation.name, aiAnim->mName.data);

    skAnimation_ReadHierarchyData(&animation.rootNode,
                                  rootNode);

    skAnimation_ReadMissingBones(&animation, aiAnim, model);

    return animation;
}

void skAnimation_Free(skAnimation* animation)
{
    if (!animation)
        return;

    if (animation->bones)
    {
        for (size_t i = 0; i < animation->bones->size; i++)
        {
            skBone* bone = (skBone*)skVector_Get(animation->bones, i);
            if (bone)
            {
                if (bone->positions)
                    skVector_Free(bone->positions);
                if (bone->rotations)
                    skVector_Free(bone->rotations);
                if (bone->scales)
                    skVector_Free(bone->scales);
            }
        }
        skVector_Free(animation->bones);
    }

    skAssimpNodeData_Free(&animation->rootNode);

    // boneInfoMap isn't freed here as it belongs to the model

    *animation = (skAnimation) {0};
}

skBone* skAnimation_FindBone(skAnimation* animation, const char* name)
{
    if (!animation || !animation->bones || !name)
        return NULL;

    for (size_t i = 0; i < animation->bones->size; i++)
    {
        skBone* bone = (skBone*)skVector_Get(animation->bones, i);
        if (bone && strcmp(bone->name, name) == 0)
        {
            return bone;
        }
    }

    return NULL;
}

void skAnimation_ReadMissingBones(skAnimation*              animation,
                                  const struct aiAnimation* aiAnim,
                                  skModel*                  model)
{
    if (!animation || !aiAnim || !model)
        return;

    int size = (int)aiAnim->mNumChannels;

    // Process each channel (bone) in the animation
    for (int i = 0; i < size; i++)
    {
        const struct aiNodeAnim* channel = aiAnim->mChannels[i];
        const char* boneNamePtr = channel->mNodeName.data;

        // Check if bone exists in model's bone info map
        if (!skMap_Contains(model->boneInfoMap, &boneNamePtr))
        {
            // Add new bone info to model's map
            skBoneInfo newBoneInfo;
            newBoneInfo.id = model->boneCount;
            glm_mat4_identity(newBoneInfo.offset);

            skMap_Insert(model->boneInfoMap, &boneNamePtr,
                         &newBoneInfo);
            model->boneCount++;
        }

        // Get bone info from map
        skBoneInfo* boneInfo =
            (skBoneInfo*)skMap_Get(model->boneInfoMap, &boneNamePtr);

        // Create bone object and add to animation
        skBone bone =
            skBone_Create(boneNamePtr, boneInfo->id, channel);
        skVector_PushBack(animation->bones, &bone);
    }
}

void skAnimation_ReadHierarchyData(skAssimpNodeData*    dest,
                                   const struct aiNode* src)
{
    if (!dest || !src)
        return;

    // Copy node name
    strncpy(dest->name, src->mName.data, sizeof(dest->name) - 1);
    dest->name[sizeof(dest->name) - 1] = '\0';

    // Convert Assimp matrix to CGLM matrix
    skAssimpMat4ToGLM(&src->mTransformation, dest->transformation);

    dest->childrenCount = (int)src->mNumChildren;

    // Initialize children vector
    dest->children = skVector_Create(sizeof(skAssimpNodeData),
                                     dest->childrenCount);

    for (int i = 0; i < dest->childrenCount; i++)
    {
        skAssimpNodeData childData = {0};
        skAnimation_ReadHierarchyData(&childData, src->mChildren[i]);
        skVector_PushBack(dest->children, &childData);
    }
}

void skAssimpNodeData_Free(skAssimpNodeData* nodeData)
{
    if (!nodeData)
        return;

    if (nodeData->children)
    {
        // Recursively free children
        for (size_t i = 0; i < nodeData->children->size; i++)
        {
            skAssimpNodeData* child = (skAssimpNodeData*)skVector_Get(
                nodeData->children, i);
            if (child)
            {
                skAssimpNodeData_Free(child);
            }
        }
        skVector_Free(nodeData->children);
        nodeData->children = NULL;
    }
}

// Get bone by index
skBone* skAnimation_GetBone(skAnimation* animation, size_t index)
{
    if (!animation || !animation->bones ||
        index >= animation->bones->size)
    {
        return NULL;
    }
    return (skBone*)skVector_Get(animation->bones, index);
}

// Check if animation is valid
int skAnimation_IsValid(skAnimation* animation)
{
    return animation && animation->bones &&
           animation->bones->size > 0 && animation->duration > 0.0f;
}

skAnimator skAnimator_Create(skModel* model)
{
    skAnimator anim = {0};

    anim.currentTime = 0.0f;

    anim.finalBoneMatrices = skVector_Create(sizeof(mat4), 100);
    anim.animations = skVector_Create(sizeof(skAnimation), 1);

    for (int i = 0; i < 100; i++)
    {
        mat4 ident = GLM_MAT4_IDENTITY_INIT;
        skVector_PushBack(anim.finalBoneMatrices, &ident);
    }

    const struct aiScene* scene =
        aiImportFile(model->path, aiProcess_Triangulate);
    
    if (!scene || !scene->mRootNode || !scene->mNumAnimations)
    {
        printf("SK ERROR: Failed to load animation file: %s\n",
               model->path);
        return anim;
    }

    for (int i = 0; i < scene->mNumAnimations; i++)
    {
        const struct aiAnimation* aiAnim = scene->mAnimations[i];

        skAnimation animation = skAnimation_Create(aiAnim, scene->mRootNode, model);

        skVector_PushBack(anim.animations, &animation);
    }
    
    skAnimation* firstAnim = (skAnimation*)skVector_Get(anim.animations, 0);
    anim.currentAnimation = firstAnim;
    
    aiReleaseImport(scene);

    return anim;
}

void skAnimator_UpdateAnimation(skAnimator* animator, float dt)
{
    animator->deltaTime = dt;
    if (animator->currentAnimation)
    {
        animator->currentTime +=
            animator->currentAnimation->ticksPerSecond * dt;

        animator->currentTime =
            fmod(animator->currentTime,
                 animator->currentAnimation->duration);

        skAnimator_CalculateBoneTransform(
            animator, &animator->currentAnimation->rootNode,
            GLM_MAT4_IDENTITY);
    }
}

void skAnimator_PlayAnimation(skAnimator* animator, skAnimation* anim)
{
    animator->currentAnimation = anim;
    animator->currentTime = 0.0f;
}

void skAnimator_CalculateBoneTransform(skAnimator*       animator,
                                       skAssimpNodeData* node,
                                       mat4 parentTransform)
{
    skBone* bone =
        skAnimation_FindBone(animator->currentAnimation, node->name);

    mat4 nodeTransform;
    glm_mat4_copy(node->transformation, nodeTransform);

    if (bone)
    {
        skBone_Update(bone, animator->currentTime);
        glm_mat4_copy(bone->localTransform, nodeTransform);
    }

    mat4 globalTransformation;
    glm_mat4_mul(parentTransform, nodeTransform,
                 globalTransformation);

    const char* nodeName = &node->name;

    if (skMap_Contains(animator->currentAnimation->boneInfoMap,
                       &nodeName))
    {
        skBoneInfo* info = (skBoneInfo*)skMap_Get(
            animator->currentAnimation->boneInfoMap, &nodeName);

        int index = info->id;

        mat4 bruhMat;
        glm_mat4_mul(globalTransformation, info->offset, bruhMat);

        mat4* boneMat =
            (mat4*)skVector_Get(animator->finalBoneMatrices, index);
        glm_mat4_copy(bruhMat, *boneMat);
    }

    for (int i = 0; i < node->childrenCount; i++)
    {
        skAssimpNodeData* nodeData =
            (skAssimpNodeData*)skVector_Get(node->children, i);

        skAnimator_CalculateBoneTransform(animator, nodeData,
                                          globalTransformation);
    }
}
