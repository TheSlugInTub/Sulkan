#pragma once

#include <sulkan/vector.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/matrix4x4.h>
#include <assimp/vector3.h>
#include <cglm/cglm.h>

typedef struct
{
    vec3 position;
    vec3 normal;
    vec2 textureCoordinates;
    vec3 tangent;
    vec3 bitangent;
} skVertex;

typedef struct
{
    char            type[64];
    char            path[128];
    struct aiTexel* embeddedData; // If the texture is embedded,
                                  // its data will be stored here
} skTexture;

typedef struct
{
    skVector* vertices; // skVertex
    skVector* indices;  // unsigned int
    skVector* textures; // skTexture
} skMesh;

skMesh skMesh_Create(skVector* meshVertices, skVector* meshIndices,
                     skVector* meshTextures);

typedef struct
{
    char      path[128];
    char      directory[128];
    skVector* loadedTextures; // skTexture
    skVector* meshes;         // skMesh
} skModel;

skModel skModel_Create();
void    skModel_Load(skModel* model, const char* path);
void    skModel_ProcessNode(skModel* model, struct aiNode* node,
                            const struct aiScene* scene);
skMesh  skModel_ProcessMesh(skModel* model, struct aiMesh* mesh,
                            const struct aiScene* scene);
void    skModel_LoadMaterialTextures(skModel*              model,
                                     struct aiMaterial*    mat,
                                     enum aiTextureType    type,
                                     const char*           typeName,
                                     const struct aiScene* scene,
                                     skVector*             textures);

unsigned int skTextureFromFile(const char* path,
                               const char* directory);

unsigned int
skTextureFromEmbeddedData(const struct aiTexture* texture);

void skAssimpVec3ToGLM(const struct aiVector3D* from, vec3 to);
void skAssimpMat4ToGLM(const struct aiMatrix4x4* from, mat4 to);
