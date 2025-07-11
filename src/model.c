#include <sulkan/model.h>
#include <stb/stb_image.h>
#include <assert.h>

skMesh skMesh_Create(skVector* meshVertices, skVector* meshIndices,
                     skVector* meshTextures)
{
    skMesh mesh = {};

    mesh.vertices = meshVertices;
    mesh.indices = meshIndices;
    mesh.textures = meshTextures;

    return mesh;
}

void skMesh_Destroy(skMesh* mesh)
{
    skVector_Free(mesh->vertices);
    skVector_Free(mesh->indices);
    skVector_Free(mesh->textures);
}

void skAssimpMat4ToGLM(const struct aiMatrix4x4* from, mat4 to)
{
    to[0][0] = from->a1;
    to[1][0] = from->a2;
    to[2][0] = from->a3;
    to[3][0] = from->a4;
    to[0][1] = from->b1;
    to[1][1] = from->b2;
    to[2][1] = from->b3;
    to[3][1] = from->b4;
    to[0][2] = from->c1;
    to[1][2] = from->c2;
    to[2][2] = from->c3;
    to[3][2] = from->c4;
    to[0][3] = from->d1;
    to[1][3] = from->d2;
    to[2][3] = from->d3;
    to[3][3] = from->d4;
}

void skAssimpVec3ToGLM(const struct aiVector3D* from, vec3 to)
{
    to[0] = from->x;
    to[1] = from->y;
    to[2] = from->z;
}

skModel skModel_Create()
{
    skModel model = {0};

    model.loadedTextures = skVector_Create(sizeof(skTexture), 2);
    model.meshes = skVector_Create(sizeof(skMesh), 2);

    return model;
}

void skModel_Load(skModel* model, const char* path)
{
    // Extract directory from path
    strncpy(model->directory, path, sizeof(model->directory) - 1);
    model->directory[sizeof(model->directory) - 1] = '\0';

    // Find last slash to extract directory
    char* lastSlash = strrchr(model->directory, '/');
    if (lastSlash != NULL)
    {
        *(lastSlash + 1) = '\0';
    }
    else
    {
        // If no slash found, set directory to empty string
        model->directory[0] = '\0';
    }

    // Read file via ASSIMP
    const struct aiScene* scene = aiImportFile(
        path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                  aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
                  aiProcess_GenUVCoords |
                  aiProcess_JoinIdenticalVertices);

    // Check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode)
    {
        fprintf(stderr, "ERROR::ASSIMP:: %s\n", aiGetErrorString());
        return;
    }

    // Process ASSIMP's root node recursively
    skModel_ProcessNode(model, scene->mRootNode, scene);
}

// Process node recursively
void skModel_ProcessNode(skModel* model, struct aiNode* node,
                         const struct aiScene* scene)
{
    // Process each mesh at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        struct aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        skMesh         processedMesh =
            skModel_ProcessMesh(model, mesh, scene);

        skVector_PushBack(model->meshes, &processedMesh);
    }

    // Process children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        skModel_ProcessNode(model, node->mChildren[i], scene);
    }
}

// Process mesh
skMesh skModel_ProcessMesh(skModel* model, struct aiMesh* mesh,
                           const struct aiScene* scene)
{
    skVector* vertices; // Vector of Vertex
    skVector* indices;  // Vector of unsigned int
    skVector* textures; // Vector of Texture

    // Initialize vectors
    vertices = skVector_Create(sizeof(skVertex), 10);
    indices = skVector_Create(sizeof(uint32_t), 10);
    textures = skVector_Create(sizeof(skTexture), 2);

    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        skVertex vertex;
        // Position
        skAssimpVec3ToGLM(&mesh->mVertices[i], vertex.position);

        // Normal
        if (mesh->mNormals)
        {
            skAssimpVec3ToGLM(&mesh->mNormals[i], vertex.normal);
        }

        if (mesh->mTextureCoords[0]) // Check if texture coordinates
                                     // exist
        {
            vertex.textureCoordinates[0] =
                mesh->mTextureCoords[0][i].x;
            vertex.textureCoordinates[1] =
                mesh->mTextureCoords[0][i].y;
        }
        else
        {
            vertex.textureCoordinates[0] = 0.0f;
            vertex.textureCoordinates[1] = 0.0f;
        }

        skVector_PushBack(vertices, (void*)&vertex);
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        struct aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            skVector_PushBack(indices, (void*)&face.mIndices[j]);
        }
    }

    // Process material/textures
    if (mesh->mMaterialIndex >= 0)
    {
        struct aiMaterial* material =
            scene->mMaterials[mesh->mMaterialIndex];

        // Load diffuse maps
        skModel_LoadMaterialTextures(
            model, material, aiTextureType_DIFFUSE, "texture_diffuse",
            scene, textures);

        // Load specular maps
        skModel_LoadMaterialTextures(
            model, material, aiTextureType_SPECULAR,
            "texture_specular", scene, textures);

        // Load normal maps
        skModel_LoadMaterialTextures(
            model, material, aiTextureType_HEIGHT, "texture_normal",
            scene, textures);

        // Load height maps
        skModel_LoadMaterialTextures(
            model, material, aiTextureType_AMBIENT, "texture_height",
            scene, textures);
    }

    skMesh result = skMesh_Create(vertices, indices, textures);

    return result;
}

// Load material textures
void skModel_LoadMaterialTextures(skModel*              model,
                                  struct aiMaterial*    mat,
                                  enum aiTextureType    type,
                                  const char*           typeName,
                                  const struct aiScene* scene,
                                  skVector*             textures)
{
    // Get texture count for this type
    unsigned int textureCount = aiGetMaterialTextureCount(mat, type);

    for (unsigned int i = 0; i < textureCount; i++)
    {
        struct aiString path;
        aiGetMaterialTexture(mat, type, i, &path, NULL, NULL, NULL,
                             NULL, NULL, NULL);

        // Check if texture was loaded before
        bool skip = false;

        for (size_t j = 0; j < model->loadedTextures->size; j++)
        {
            skTexture* loadedTexture =
                (skTexture*)skVector_Get(model->loadedTextures, j);
            if (strcmp(loadedTexture->path, path.data) == 0)
            {
                skVector_PushBack(textures, (void*)loadedTexture);
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            skTexture texture;
            strncpy(texture.path, path.data,
                    sizeof(texture.path) - 1);
            texture.path[sizeof(texture.path) - 1] = '\0';

            // Check if this is an embedded texture
            if (path.data[0] == '*')
            {
                // Extract embedded texture index
                unsigned int embeddedIndex = atoi(path.data + 1);
                if (embeddedIndex < scene->mNumTextures)
                {
                    const struct aiTexture* embeddedTexture =
                        scene->mTextures[embeddedIndex];
                    texture.embeddedData = embeddedTexture->pcData;
                }
            }

            strncpy(texture.type, typeName, sizeof(texture.type) - 1);
            texture.type[sizeof(texture.type) - 1] = '\0';

            skVector_PushBack(textures, &texture);
            skVector_PushBack(model->loadedTextures, &texture);
        }
    }
}

// Free model resources
void skModel_Destroy(skModel* model)
{
    for (size_t i = 0; i < model->meshes->size; i++)
    {
        skMesh* mesh = (skMesh*)skVector_Get(model->meshes, i);
        skMesh_Destroy(mesh);
    }

    skVector_Free(model->meshes);
    skVector_Free(model->loadedTextures);
}
