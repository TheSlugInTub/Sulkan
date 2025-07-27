#include <sulkan/bone.h>
#include <assert.h>

void skAssimpMat4ToGLM(const struct aiMatrix4x4* from, mat4 to);

void skAssimpVec3ToGLM(const struct aiVector3D* from, vec3 to);

skBone skBone_Create(const char* name, int ID,
                     const struct aiNodeAnim* channel)
{
    skBone bone = {0};

    strcpy(bone.name, name);
    bone.ID = ID;
    glm_mat4_identity(bone.localTransform);

    // Initialize all keyframes by acessing them through assimp

    bone.positions = skVector_Create(sizeof(skKeyPosition), 5);
    bone.rotations = skVector_Create(sizeof(skKeyRotation), 5);
    bone.scales = skVector_Create(sizeof(skKeyScale), 5);

    bone.numPositions = channel->mNumPositionKeys;
    for (int positionIndex = 0; positionIndex < bone.numPositions;
         ++positionIndex)
    {
        const struct aiVector3D aiPosition =
            channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        skKeyPosition data;
        skAssimpVec3ToGLM(&aiPosition, data.position);
        data.timeStamp = timeStamp;

        skVector_PushBack(bone.positions, &data);
    }

    bone.numRotations = channel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < bone.numRotations;
         ++rotationIndex)
    {
        const struct aiQuaternion aiOrientation =
            channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        skKeyRotation data;
        data.rotation[0] = aiOrientation.x;
        data.rotation[1] = aiOrientation.y;
        data.rotation[2] = aiOrientation.z;
        data.rotation[3] = aiOrientation.w;
        data.timeStamp = timeStamp;

        skVector_PushBack(bone.rotations, &data);
    }

    bone.numScales = channel->mNumScalingKeys;
    for (int keyIndex = 0; keyIndex < bone.numScales; ++keyIndex)
    {
        const struct aiVector3D scale =
            channel->mScalingKeys[keyIndex].mValue;
        float      timeStamp = channel->mScalingKeys[keyIndex].mTime;
        skKeyScale data;
        skAssimpVec3ToGLM(&scale, data.scale);
        data.timeStamp = timeStamp;

        skVector_PushBack(bone.scales, &data);
    }

    return bone;
}

int skBone_GetPositionIndex(skBone* bone, float animationTime)
{
    for (int index = 0; index < bone->numPositions - 1; ++index)
    {
        skKeyPosition* pos =
            (skKeyPosition*)skVector_Get(bone->positions, index + 1);

        if (animationTime < pos->timeStamp)
            return index;
    }
    assert(0);
}

int skBone_GetRotationIndex(skBone* bone, float animationTime)
{
    for (int index = 0; index < bone->numRotations - 1; ++index)
    {
        skKeyRotation* rot =
            (skKeyRotation*)skVector_Get(bone->rotations, index + 1);

        if (animationTime < rot->timeStamp)
            return index;
    }
    assert(0);
}

int skBone_GetScaleIndex(skBone* bone, float animationTime)
{
    for (int index = 0; index < bone->numScales - 1; ++index)
    {
        skKeyScale* scale =
            (skKeyScale*)skVector_Get(bone->scales, index + 1);

        if (animationTime < scale->timeStamp)
            return index;
    }
    assert(0);
}

float skGetScaleFactor(float lastTimeStamp, float nextTimeStamp,
                       float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

void skBone_InterpolatePosition(skBone* bone, float animationTime,
                                mat4 dest)
{
    if (bone->numPositions == 1)
    {
        skKeyPosition* pos =
            (skKeyPosition*)skVector_Get(bone->positions, 0);
        glm_translate(dest, pos->position);
        return;
    }

    int p0Index = skBone_GetPositionIndex(bone, animationTime);
    int p1Index = p0Index + 1;

    skKeyPosition* key1 =
        (skKeyPosition*)skVector_Get(bone->positions, p0Index);
    skKeyPosition* key2 =
        (skKeyPosition*)skVector_Get(bone->positions, p1Index);

    float scaleFactor = skGetScaleFactor(
        key1->timeStamp, key2->timeStamp, animationTime);

    vec3 finalPosition;
    glm_vec3_mix(key1->position, key2->position, scaleFactor,
                 finalPosition);

    glm_translate(dest, finalPosition);
}

void skBone_InterpolateRotation(skBone* bone, float animationTime,
                                mat4 dest)
{
    if (bone->numRotations == 1)
    {
        skKeyRotation* rot =
            (skKeyRotation*)skVector_Get(bone->rotations, 0);
        glm_quat_mat4(rot->rotation, dest);
        return;
    }

    int p0Index = skBone_GetRotationIndex(bone, animationTime);
    int p1Index = p0Index + 1;

    skKeyRotation* key1 =
        (skKeyRotation*)skVector_Get(bone->rotations, p0Index);
    skKeyRotation* key2 =
        (skKeyRotation*)skVector_Get(bone->rotations, p1Index);

    float scaleFactor = skGetScaleFactor(
        key1->timeStamp, key2->timeStamp, animationTime);

    vec4 finalRotation;
    glm_quat_slerp(key1->rotation, key2->rotation, scaleFactor,
                   finalRotation);

    glm_quat_mat4(finalRotation, dest);
}

void skBone_InterpolateScale(skBone* bone, float animationTime,
                             mat4 dest)
{
    if (bone->numScales == 1)
    {
        skKeyScale* scale =
            (skKeyScale*)skVector_Get(bone->scales, 0);
        glm_scale(dest, scale->scale);
        return;
    }

    int   p0Index = skBone_GetScaleIndex(bone, animationTime);
    int   p1Index = p0Index + 1;
    float scaleFactor = skGetScaleFactor(
        ((skKeyScale*)skVector_Get(bone->scales, p0Index))->timeStamp,
        ((skKeyScale*)skVector_Get(bone->scales, p1Index))->timeStamp,
        animationTime);

    vec3 finalScale;
    glm_vec3_mix(
        ((skKeyScale*)skVector_Get(bone->scales, p0Index))->scale,
        ((skKeyScale*)skVector_Get(bone->scales, p1Index))->scale,
        scaleFactor, finalScale);

    glm_scale(dest, finalScale);
}

void skBone_Update(skBone* bone, float animationTime)
{
    mat4 trans = GLM_MAT4_IDENTITY_INIT,
         rotation = GLM_MAT4_IDENTITY_INIT,
         scale = GLM_MAT4_IDENTITY_INIT;
    skBone_InterpolatePosition(bone, animationTime, trans);
    skBone_InterpolateRotation(bone, animationTime, rotation);
    skBone_InterpolateScale(bone, animationTime, scale);
    glm_mat4_mul(trans, rotation, bone->localTransform);
    glm_mat4_mul(bone->localTransform, scale, bone->localTransform);
}
