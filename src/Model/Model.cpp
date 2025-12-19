#include <Model/Model.h>

#include <stdexcept>
#include <Model/Mesh.h>
#include <Texture/Texture.h>

Model::Model()
{
}

Model::~Model()
{
}

void Model::LoadModel(const char* filename)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(filename,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
    
    if (!scene)
    {
        //throw std::runtime_error("Model " + std::string(filename) + " failed to load " + importer.GetErrorString());
        return;
    }

    LoadModel(scene->mRootNode, scene);
    LoadMaterials(scene);
}

void Model::Render()
{
   for (size_t i = 0; i < _meshList.size(); ++i)
   {
       unsigned int materialIndex = _meshToTexture[i];
       if (materialIndex < _textureList.size() && _textureList[materialIndex])
       {
           _textureList[materialIndex]->Use();
       }

       _meshList[i]->Draw();
   }
}

void Model::Release()
{
    for (size_t i = 0; i < _meshList.size(); ++i)
    {
        if (_meshList[i])
        {
            _meshList[i] = nullptr;
        }
    }
    for (size_t i = 0; i < _textureList.size(); ++i)
    {
        if (_textureList[i])
        {
            _textureList[i] = nullptr;
        }
    }
}

void Model::LoadModel(aiNode* node, const aiScene* scene)
{
    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
        LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
    }

    for (size_t i = 0; i < node->mNumChildren; ++i)
    {
        LoadModel(node->mChildren[i], scene);
    }
}

void Model::LoadMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<GLfloat> vertices;
    std::vector<unsigned int> indices;

    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        vertices.insert(vertices.end(), {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z});
        if (mesh->mTextureCoords[0])
        {
            vertices.insert(vertices.end(), {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y});
        }
        else
        {
            vertices.insert(vertices.end(), {0.0f, 0.0f});
        }
        vertices.insert(vertices.end(), {-mesh->mNormals[i].x, -mesh->mNormals[i].y, -mesh->mNormals[i].z});
    }

    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    std::shared_ptr<Mesh> newMesh = std::make_shared<Mesh>();
    newMesh->Create(vertices, indices);
    _meshList.push_back(newMesh);
    _meshToTexture.push_back(mesh->mMaterialIndex);
}

void Model::LoadMaterials(const aiScene* scene)
{
    _textureList.resize(scene->mNumMaterials);

    for (size_t i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* material = scene->mMaterials[i];
        _textureList[i] = nullptr;

        if (material->GetTextureCount(aiTextureType_DIFFUSE))
        {
            aiString texturePath;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
            {
                int idx = std::string(texturePath.data).rfind("\\");
                std::string filename = std::string(texturePath.data).substr(idx + 1);
                
                _textureList[i] = std::make_shared<Texture>("assets/textures/" + filename);
                if (!_textureList[i]->LoadA())
                {
                    _textureList[i] = nullptr;
                }
            }
        }

        if (!_textureList[i])
        {
            _textureList[i] = std::make_shared<Texture>("assets/textures/plain.png");
            _textureList[i]->LoadA();
        }
    }
}