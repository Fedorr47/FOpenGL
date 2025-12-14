#pragma once
#include <memory>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Mesh;
class Texture;

class Model
{
public:
    Model();
    ~Model();
    Model(const Model& other){}
    Model& operator=(const Model& other){}
    Model(Model&& other){}
    Model& operator=(Model&& other){}

    void LoadModel(const char* filename);
    void Render();
    void Release();
  
private:
    
    void LoadModel(aiNode* node, const aiScene* scene);
    void LoadMesh(aiMesh* mesh, const aiScene* scene);
    void LoadMaterials(const aiScene* scene);
  
    // TODO: change to id map against this 3 vectors
    std::vector<std::shared_ptr<Mesh>> _meshList;
    std::vector<std::shared_ptr<Texture>> _textureList;
    std::vector<unsigned int> _meshToTexture;
};